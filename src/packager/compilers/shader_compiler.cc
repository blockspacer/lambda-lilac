#include "shader_compiler.h"
#include <utils/file_system.h>
#include <utils/console.h>
#include <utils/utilities.h>

#include <ShaderConductor/ShaderConductor.hpp>

namespace lambda
{
	///////////////////////////////////////////////////////////////////////////
	VioletShaderCompiler::VioletShaderCompiler()
		: VioletShaderManager()
	{
	}

	///////////////////////////////////////////////////////////////////////////
	ShaderConductor::Blob* includeCallback(const char* include_name)
	{
		Vector<char> bytes = FileSystem::FileToVector(include_name);
		return ShaderConductor::CreateBlob(bytes.data(), (uint32_t)bytes.size());
	}

	///////////////////////////////////////////////////////////////////////////
	bool CompileX(
		String file, 
		ShaderConductor::ShaderStage stage,
		Array<Vector<char>, VIOLET_LANG_COUNT>& output)
	{
		String source = FileSystem::FileToString(file);

		String entry;
		switch (stage)
		{
		case ShaderConductor::ShaderStage::ComputeShader:
			entry = "CS";
			break;
		case ShaderConductor::ShaderStage::DomainShader:
			entry = "DS";
			break;
		case ShaderConductor::ShaderStage::GeometryShader:
			entry = "GS";
			break;
		case ShaderConductor::ShaderStage::HullShader:
			entry = "HS";
			break;
		case ShaderConductor::ShaderStage::PixelShader:
			entry = "PS";
			break;
		case ShaderConductor::ShaderStage::VertexShader:
			entry = "VS";
			break;
		}

		ShaderConductor::Compiler::SourceDesc source_desc;
		source_desc.defines = nullptr;
		source_desc.entryPoint = entry.c_str();
		source_desc.fileName = file.c_str();
		source_desc.loadIncludeCallback = includeCallback;
		source_desc.numDefines = 0;
		source_desc.source = source.c_str();
		source_desc.stage = stage;

		ShaderConductor::Compiler::Options options;
		options.disableOptimizations = false;
		options.enable16bitTypes = false;
		options.enableDebugInfo = false;
		options.optimizationLevel = 3;
		options.packMatricesInRowMajor = true;
		options.shaderModel.major_ver = 6;
		options.shaderModel.minor_ver = 0;

		ShaderConductor::Compiler::TargetDesc target_descs[VIOLET_LANG_COUNT];
		target_descs[VIOLET_DXIL].language  = ShaderConductor::ShadingLanguage::Dxil;
		target_descs[VIOLET_DXIL].version   = nullptr;
		target_descs[VIOLET_SPIRV].language = ShaderConductor::ShadingLanguage::SpirV;
		target_descs[VIOLET_SPIRV].version  = nullptr;
		target_descs[VIOLET_METAL].language = ShaderConductor::ShadingLanguage::Msl;
		target_descs[VIOLET_METAL].version  = nullptr;
		target_descs[VIOLET_HLSL].language  = ShaderConductor::ShadingLanguage::Hlsl;
		target_descs[VIOLET_HLSL].version   = "50";

		ShaderConductor::Compiler::ResultDesc results[VIOLET_LANG_COUNT];
		ShaderConductor::Compiler::Compile(source_desc, options, target_descs, VIOLET_LANG_COUNT, results);

		for (int i = 0; i < VIOLET_LANG_COUNT; ++i)
		{
			if (results[i].hasError)
			{
				options.enableDebugInfo = true;
				ShaderConductor::Compiler::Compile(source_desc, options, &target_descs[i], 1, &results[i]);

				String error = String(
					(char*)results[i].errorWarningMsg->Data(),
					results[i].errorWarningMsg->Size()
				);

				if (error == "error: missing entry point definition\n")
					return true;

				String err = "ShaderConductor: Failed to compile shader \"" + file +
					"\" with message:\n" + error;

				foundation::Info(err.c_str());
				return false;
			}
		}

		for (int i = 0; i < VIOLET_LANG_COUNT; ++i)
		{
			if (i != VIOLET_HLSL)
			{
				output[i].resize(results[i].target->Size());
				memcpy(output[i].data(), results[i].target->Data(), output[i].size());
			}
			else
			{
				output[i].resize(source.size());
				memcpy(output[i].data(), source.c_str(), source.size());
			}
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	bool VioletShaderCompiler::Compile(ShaderCompileInfo compile_info)
	{
		VioletShader shader_program;
		shader_program.file_path = compile_info.file;
		shader_program.hash = GetHash(shader_program.file_path);
		if (!CompileX(compile_info.file, ShaderConductor::ShaderStage::VertexShader, shader_program.blobs[(int)ShaderStages::kVertex]))
			return false;
		if (!CompileX(compile_info.file, ShaderConductor::ShaderStage::PixelShader, shader_program.blobs[(int)ShaderStages::kPixel]))
			return false;
		if (!CompileX(compile_info.file, ShaderConductor::ShaderStage::GeometryShader, shader_program.blobs[(int)ShaderStages::kGeometry]))
			return false;
		if (!CompileX(compile_info.file, ShaderConductor::ShaderStage::HullShader, shader_program.blobs[(int)ShaderStages::kHull]))
			return false;
		if (!CompileX(compile_info.file, ShaderConductor::ShaderStage::DomainShader, shader_program.blobs[(int)ShaderStages::kDomain]))
			return false;
		if (!CompileX(compile_info.file, ShaderConductor::ShaderStage::ComputeShader, shader_program.blobs[(int)ShaderStages::kCompute]))
			return false;

		AddShader(shader_program);
		return true;
	}
}