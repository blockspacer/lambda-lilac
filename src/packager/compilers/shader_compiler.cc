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
	Vector<Vector<uint32_t>> CompileX(
		String file, 
		ShaderConductor::ShaderStage stage)
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
		target_descs[VIOLET_METAL].version = nullptr;
		target_descs[VIOLET_HLSL].language = ShaderConductor::ShadingLanguage::Hlsl;
		target_descs[VIOLET_HLSL].version = "50";

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
					return Vector<Vector<uint32_t>>(VIOLET_LANG_COUNT);

				String err = "ShaderConductor: Failed to compile shader \"" + file +
					"\" with message:\n" + error;

				LMB_ASSERT(false, err.c_str());
			}
		}

		Vector<Vector<uint32_t>> res(VIOLET_LANG_COUNT);
		for (int i = 0; i < VIOLET_LANG_COUNT; ++i)
		{
			Vector<char> temp;
			if (i != VIOLET_HLSL)
			{
				temp.resize(results[i].target->Size());
				memcpy(temp.data(), results[i].target->Data(), temp.size());
			}
			else

			{
				temp.resize(source.size());
				memcpy(temp.data(), source.c_str(), source.size());
			}
			res[i] = utilities::convertVec<char, uint32_t>(temp);
		}
		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	void VioletShaderCompiler::Compile(ShaderCompileInfo compile_info)
	{
		VioletShader shader_program;
		shader_program.file_path = compile_info.file;
		shader_program.hash = GetHash(shader_program.file_path);
		shader_program.blobs.resize((int)ShaderStages::kCount);
		shader_program.blobs[(int)ShaderStages::kVertex]   = CompileX(compile_info.file, ShaderConductor::ShaderStage::VertexShader);
		shader_program.blobs[(int)ShaderStages::kPixel]    = CompileX(compile_info.file, ShaderConductor::ShaderStage::PixelShader);
		shader_program.blobs[(int)ShaderStages::kGeometry] = CompileX(compile_info.file, ShaderConductor::ShaderStage::GeometryShader);
		shader_program.blobs[(int)ShaderStages::kHull]     = CompileX(compile_info.file, ShaderConductor::ShaderStage::HullShader);
		shader_program.blobs[(int)ShaderStages::kDomain]   = CompileX(compile_info.file, ShaderConductor::ShaderStage::DomainShader);
		shader_program.blobs[(int)ShaderStages::kCompute]  = CompileX(compile_info.file, ShaderConductor::ShaderStage::ComputeShader);
		AddShader(shader_program);
	}
}