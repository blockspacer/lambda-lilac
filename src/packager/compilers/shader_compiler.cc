#include "shader_compiler.h"
#include <utils/file_system.h>
#include <utils/console.h>
#include <utils/utilities.h>
#include <memory/memory.h>

#if VIOLET_SHADER_CONDUCTOR
#include <ShaderConductor/ShaderConductor.hpp>

///////////////////////////////////////////////////////////////////////////////
ShaderConductor::Blob* includeCallback(const char* include)
{
	lambda::Vector<char> bytes = lambda::FileSystem::FileToVector(include);
	return ShaderConductor::CreateBlob(bytes.data(), (uint32_t)bytes.size());
}
#endif

#if VIOLET_WIN32
#include <d3dcompiler.h>
#include <d3d11shader.h>
#pragma comment(lib,"D3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")

///////////////////////////////////////////////////////////////////////////////
class IncludeHandler : public ID3DInclude
{
public:
	IncludeHandler(lambda::String file_path)
		: file_path_(file_path)
	{
		file_path_ = lambda::FileSystem::RemoveName(file_path_);
	}

	// Inherited via ID3DInclude
	STDMETHOD(Open)(
		THIS_ D3D_INCLUDE_TYPE IncludeType,
		LPCSTR pFileName,
		LPCVOID pParentData,
		LPCVOID *ppData,
		UINT *pBytes) override
	{
		lambda::Vector<char> bytes =
			lambda::FileSystem::FileToVector(file_path_ + '/' + pFileName);
		UINT size = (UINT)bytes.size();

		*ppData = lambda::foundation::Memory::allocate(size);
		memcpy((void*)*ppData, bytes.data(), size);
		*pBytes = size;
		return S_OK;
	}

	STDMETHOD(Close)(THIS_ LPCVOID pData) override
	{
		lambda::foundation::Memory::deallocate((void*)pData);
		return S_OK;
	}

	lambda::String file_path_;
};
#endif

#pragma optimize ("", off)
bool compileHLSL(lambda::String file, lambda::String source, lambda::String entry, lambda::String stage, lambda::Vector<char>& output, lambda::Vector<lambda::VioletShaderResource>& resources)
{
#if VIOLET_WIN32
	ID3D10Blob* blob;
	ID3D10Blob* error;

	IncludeHandler include_handler(file);

	HRESULT result = D3DCompile(
		(void*)source.data(),
		source.size(),
		file.c_str(), 0,
		&include_handler,
		entry.c_str(),
		stage.c_str(),
		0,
		0,
		&blob,
		&error
	);

	if (FAILED(result))
	{
		if (blob)
			blob->Release();

		if (error)
		{
			lambda::String err =
				"D3DCompile: Failed to compile shader \"" + file +
				"\" with message:\n" + lambda::String((char*)error->GetBufferPointer());

			if (err.find("X3501") == lambda::String::npos)
			{
				lambda::foundation::Info(err.c_str());

				error->Release();
				return false;
			}
		}
	}
	else
	{
		ID3D11ShaderReflection* reflector = nullptr;
		HRESULT result = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);
		if (FAILED(result))
		{
			lambda::foundation::Info("Could not reflect on shader");

			blob->Release();
			return false;
		}

		resources.clear();
		D3D11_SHADER_DESC desc{};
		reflector->GetDesc(&desc);
		for (uint32_t i = 0; i < desc.BoundResources; ++i)
		{
			D3D11_SHADER_INPUT_BIND_DESC bind_desc{};
			reflector->GetResourceBindingDesc(i, &bind_desc);
			
			lambda::VioletShaderResource resource;
			resource.slot = bind_desc.BindPoint;
			resource.name = bind_desc.Name;
			resource.size = 1;
			
			switch (stage[0])
			{
			case 'c': case 'C': resource.stage = lambda::ShaderStages::kCompute;  break;
			case 'd': case 'D': resource.stage = lambda::ShaderStages::kDomain;   break;
			case 'g': case 'G': resource.stage = lambda::ShaderStages::kGeometry; break;
			case 'h': case 'H': resource.stage = lambda::ShaderStages::kHull;     break;
			case 'p': case 'P': resource.stage = lambda::ShaderStages::kPixel;    break;
			case 'v': case 'V': resource.stage = lambda::ShaderStages::kVertex;   break;
			}

			switch (bind_desc.Type)
			{
				case D3D_SIT_CBUFFER:
				{
					resource.type = lambda::VioletShaderResourceType::kConstantBuffer;
					
					ID3D11ShaderReflectionConstantBuffer* cbuffer = reflector->GetConstantBufferByName(bind_desc.Name);
					D3D11_SHADER_BUFFER_DESC cb_desc;
					cbuffer->GetDesc(&cb_desc);
					resource.size = cb_desc.Size;
					
					for (uint32_t j = 0; j < cb_desc.Variables; ++j)
					{
						lambda::VioletShaderResource::Item item;
						ID3D11ShaderReflectionVariable* var = cbuffer->GetVariableByIndex(j);
						D3D11_SHADER_VARIABLE_DESC var_desc;
						var->GetDesc(&var_desc);
						// TODO (Hilze): Support default value.
						//var_desc.DefaultValue;
						item.name   = var_desc.Name;
						item.size   = var_desc.Size;
						item.offset = var_desc.StartOffset;
						resource.items.push_back(item);
					}
					break;
					}
				case D3D_SIT_SAMPLER:
					resource.type = lambda::VioletShaderResourceType::kSampler;
					break;
				case D3D_SIT_TEXTURE:
					resource.type = lambda::VioletShaderResourceType::kTexture;
					break;
			}
			resources.push_back(resource);
		}

		output.resize(blob->GetBufferSize());
		memcpy(output.data(), blob->GetBufferPointer(), output.size());
		blob->Release();
	}

#else
	output.resize(source.size());
	memcpy(output.data(), source.c_str(), output.size());
#endif
	return true;
}

namespace lambda
{
	///////////////////////////////////////////////////////////////////////////
	VioletShaderCompiler::VioletShaderCompiler()
		: VioletShaderManager()
	{
	}

	///////////////////////////////////////////////////////////////////////////
	bool CompileX(
		String file, 
		String stage,
		Array<Vector<char>, VIOLET_LANG_COUNT>& output,
		Vector<VioletShaderResource>& resources)
	{
		String source = FileSystem::FileToString(file);

		String entry;
		switch (stage[0])
		{
		case 'c': case 'C': entry = "CS"; break;
		case 'd': case 'D': entry = "DS"; break;
		case 'g': case 'G': entry = "GS"; break;
		case 'h': case 'H': entry = "HS"; break;
		case 'p': case 'P': entry = "PS"; break;
		case 'v': case 'V': entry = "VS"; break;
		}

#if VIOLET_SHADER_CONDUCTOR
		ShaderConductor::ShaderStage sc_stage;
		switch (entry[0])
		{
		case 'C': sc_stage = ShaderConductor::ShaderStage::ComputeShader; break;
		case 'D': sc_stage = ShaderConductor::ShaderStage::DomainShader; break;
		case 'G': sc_stage = ShaderConductor::ShaderStage::GeometryShader; break;
		case 'H': sc_stage = ShaderConductor::ShaderStage::HullShader; break;
		case 'P': sc_stage = ShaderConductor::ShaderStage::PixelShader; break;
		case 'V': sc_stage = ShaderConductor::ShaderStage::VertexShader; break;
		}

		ShaderConductor::MacroDefine defines[] = {
			{ "VIOLET_SPIRV", "1" }
		};

		ShaderConductor::Compiler::SourceDesc source_desc;
		source_desc.defines = nullptr;
		source_desc.entryPoint = entry.c_str();
		source_desc.fileName = file.c_str();
		source_desc.loadIncludeCallback = includeCallback;
		source_desc.numDefines = 1;
		source_desc.defines = defines;
		source_desc.source = source.c_str();
		source_desc.stage = sc_stage;

		ShaderConductor::Compiler::Options options;
		options.disableOptimizations = false;
		options.enable16bitTypes = false;
		options.enableDebugInfo = true;
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
#if VIOLET_WIN32
				if (!compileHLSL(file, source, entry, stage, output[i], resources))
					return false;
#else
				output[i].resize(source.size());
				memcpy(output[i].data(), source.c_str(), source.size());
#endif
			}
		}
#else
		for (int i = 0; i < VIOLET_LANG_COUNT; ++i)
		{
#if VIOLET_WIN32
			if (i == VIOLET_HLSL)
			{
				if (!compileHLSL(file, source, entry, stage, output[i], resources))
					return false;
			}
			else
#endif
			{
				output[i].resize(source.size());
				memcpy(output[i].data(), source.c_str(), output[i].size());
			}
		}
#endif
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	bool VioletShaderCompiler::Compile(ShaderCompileInfo compile_info)
	{
		VioletShader shader_program;
		shader_program.file_path = compile_info.file;
		shader_program.hash = GetHash(shader_program.file_path);
		if (!CompileX(compile_info.file, "vs_5_0", shader_program.blobs[(int)ShaderStages::kVertex], shader_program.resources[(int)ShaderStages::kVertex]))
			return false;
		if (!CompileX(compile_info.file, "ps_5_0", shader_program.blobs[(int)ShaderStages::kPixel], shader_program.resources[(int)ShaderStages::kPixel]))
			return false;
		if (!CompileX(compile_info.file, "gs_5_0", shader_program.blobs[(int)ShaderStages::kGeometry], shader_program.resources[(int)ShaderStages::kGeometry]))
			return false;
		if (!CompileX(compile_info.file, "cs_5_0", shader_program.blobs[(int)ShaderStages::kCompute], shader_program.resources[(int)ShaderStages::kCompute]))
			return false;
		if (!CompileX(compile_info.file, "hs_5_0", shader_program.blobs[(int)ShaderStages::kHull], shader_program.resources[(int)ShaderStages::kHull]))
			return false;
		if (!CompileX(compile_info.file, "ds_5_0", shader_program.blobs[(int)ShaderStages::kDomain], shader_program.resources[(int)ShaderStages::kDomain]))
			return false;

		AddShader(shader_program);
		return true;
	}
}