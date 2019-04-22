#include "d3d11_shader.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <utils/file_system.h>
#include <renderers/d3d11/d3d11_context.h>

#include <D3Dcompiler.h>
#pragma comment(lib,"D3dcompiler.lib")

namespace lambda
{
  namespace windows
  {
		///////////////////////////////////////////////////////////////////////////
		ID3D10Blob* compile(
			const String& file,
			const Vector<char>& source,
			const String& entry,
			const String& target)
		{
			if (source.empty())
				return nullptr;

			LMB_ASSERT((source.size() > 3 && (source[0] == 'D' && source[1] == 'X' && source[2] == 'B' && source[3] == 'C')), "D3D11SHADER: Shader was invalid");

			ID3D10Blob* blob;
			HRESULT res = D3DCreateBlob(source.size(), &blob);
			LMB_ASSERT(res == S_OK, "[SHADER] Failed to create blob!");
			memcpy(blob->GetBufferPointer(), source.data(), source.size());

			return blob;
		}

    ///////////////////////////////////////////////////////////////////////////
    D3D11Shader::D3D11Shader(
      asset::VioletShaderHandle shader,
      D3D11Context* context)
      : context_(context)
      , vs_(nullptr)
      , ps_(nullptr)
      , gs_(nullptr)
      , il_(nullptr)
    {
			auto data = asset::ShaderManager::getInstance()->getData(shader);

			ID3D10Blob* vs_blob = compile(
				shader->getFilePath(),
				data[(int)ShaderStages::kVertex][VIOLET_HLSL],
				"VS",
				"vs_5_0"
			);
			ID3D10Blob* ps_blob = compile(
				shader->getFilePath(),
				data[(int)ShaderStages::kPixel][VIOLET_HLSL],
				"PS",
				"ps_5_0"
			);
			ID3D10Blob* gs_blob = compile(
				shader->getFilePath(),
				data[(int)ShaderStages::kGeometry][VIOLET_HLSL],
				"GS",
				"gs_5_0"
			);

			if (vs_blob)
			{
				if (FAILED(context_->getD3D11Device()->CreateVertexShader(
					vs_blob->GetBufferPointer(),
					vs_blob->GetBufferSize(),
					NULL,
					&vs_
				)))
					vs_ = nullptr;

				if (vs_)
					reflectInputLayout(vs_blob, context_->getD3D11Device());
				if (vs_blob) vs_blob->Release();
			}

			if (ps_blob)
			{
				if (FAILED(context_->getD3D11Device()->CreatePixelShader(
					ps_blob->GetBufferPointer(),
					ps_blob->GetBufferSize(),
					NULL,
					&ps_
				)))
					ps_ = nullptr;

				if (ps_blob) ps_blob->Release();
			}

			if (gs_blob)
			{
				if (FAILED(context_->getD3D11Device()->CreateGeometryShader(
					gs_blob->GetBufferPointer(),
					gs_blob->GetBufferSize(),
					NULL,
					&gs_
				)))
					gs_ = nullptr;

				if (gs_blob) gs_blob->Release();
			}

			for (uint32_t i = 0; i < (uint32_t)ShaderStages::kCount; ++i)
			{
				for (const auto& resource : shader->getResources((ShaderStages)i))
				{
					switch (resource.type)
					{
					case VioletShaderResourceType::kConstantBuffer:
						buffers_.push_back(resource);
						break;
					case VioletShaderResourceType::kSampler:
						samplers_.push_back(resource);
						break; 
					case VioletShaderResourceType::kTexture:
						textures_.push_back(resource);
						break;
					}
				}
			}
    }
    
    ///////////////////////////////////////////////////////////////////////////
    D3D11Shader::~D3D11Shader()
    {
	  if (ps_) ps_->Release(), ps_ = nullptr;
	  if (vs_) vs_->Release(), vs_ = nullptr;
	  if (gs_) gs_->Release(), gs_ = nullptr;
	  if (il_) il_->Release(), il_ = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Shader::bind()
    {
      context_->getD3D11Context()->VSSetShader(vs_, nullptr, 0);
      context_->getD3D11Context()->PSSetShader(ps_, nullptr, 0);
      context_->getD3D11Context()->GSSetShader(gs_, nullptr, 0);
      context_->getD3D11Context()->IASetInputLayout(il_);
    }
	
    ///////////////////////////////////////////////////////////////////////////
		Vector<uint32_t> D3D11Shader::getStages() const
    {
      return stages_;
    }
    
    ///////////////////////////////////////////////////////////////////////////
	const Vector<VioletShaderResource>& D3D11Shader::getBuffers()
    {
      return buffers_;
    }

	///////////////////////////////////////////////////////////////////////////
	const Vector<VioletShaderResource>& D3D11Shader::getTextures()
	{
		return textures_;
	}

	///////////////////////////////////////////////////////////////////////////
	const Vector<VioletShaderResource>& D3D11Shader::getSamplers()
	{
		return samplers_;
	}

    ///////////////////////////////////////////////////////////////////////////
    int sizeFromFormat(DXGI_FORMAT format)
    {
      switch (format)
      {
      case DXGI_FORMAT_R32_UINT: return 4; break;
      case DXGI_FORMAT_R32_SINT: return 4; break;
      case DXGI_FORMAT_R32_FLOAT: return 4; break;
      case DXGI_FORMAT_R32G32_UINT: return 8; break;
      case DXGI_FORMAT_R32G32_SINT: return 8; break;
      case DXGI_FORMAT_R32G32_FLOAT: return 8; break;
      case DXGI_FORMAT_R32G32B32_UINT: return 12; break;
      case DXGI_FORMAT_R32G32B32_SINT: return 12; break;
      case DXGI_FORMAT_R32G32B32_FLOAT: return 12; break;
      case DXGI_FORMAT_R32G32B32A32_UINT: return 16; break;
      case DXGI_FORMAT_R32G32B32A32_SINT: return 16; break;
      case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16; break;
      }
      return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Shader::reflectInputLayout(
      ID3D10Blob* blob, 
      ID3D11Device* device)
    {
      ID3D11ShaderReflection* reflection = nullptr;
      if (FAILED(D3DReflect(
        blob->GetBufferPointer(), 
        blob->GetBufferSize(), 
        IID_PPV_ARGS(&reflection)
      )))
      {
        MessageBox(NULL, "Failed to do shader reflection!", "Shader", 0);
        return;
      }

      D3D11_SHADER_DESC shader_description{};
      reflection->GetDesc(&shader_description);

      //! Input layout.
      int idx    = 0;
      int offset = 0;
      int inl    = 0;

      Vector<D3D11_INPUT_ELEMENT_DESC> input_layout;
      for (uint32_t i = 0; i < shader_description.InputParameters; ++i)
      {
        D3D11_SIGNATURE_PARAMETER_DESC parameter;
        reflection->GetInputParameterDesc(i,&parameter);

        D3D11_INPUT_ELEMENT_DESC element;
        element.SemanticName = parameter.SemanticName;

				String semantic = element.SemanticName;

        // Inlining
        if (semantic.find("inl_") != String::npos)
        {
          semantic = semantic.substr(strlen("inl_"));
          element.InputSlot = idx;
          inl++;
        }
        else
        {
          if (inl == 0)
          {
            element.InputSlot = idx++;
          }
          else
          {
            element.InputSlot = ++idx;
          }
          offset = 0;
        }

        if (semantic.find("use_me_") != String::npos)
          semantic = semantic.substr(strlen("use_me_"));

		element.SemanticIndex = parameter.SemanticIndex;
		element.AlignedByteOffset = offset;
		element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;

        // Instancing
        if (String(element.SemanticName).find("instance_") != String::npos)
        {
					semantic = semantic.substr(strlen("instance_"));
          element.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
          element.InstanceDataStepRate = 1;
        }

				if (inl <= 1)
					stages_.push_back(constexprHash(semantic));
			
				if (parameter.Mask == 1)
        {
          if (parameter.ComponentType == D3D_REGISTER_COMPONENT_UINT32)  
            element.Format = DXGI_FORMAT_R32_UINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  
            element.Format = DXGI_FORMAT_R32_SINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) 
            element.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (parameter.Mask <= 3)
        {
          if (parameter.ComponentType == D3D_REGISTER_COMPONENT_UINT32)  
            element.Format = DXGI_FORMAT_R32G32_UINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  
            element.Format = DXGI_FORMAT_R32G32_SINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) 
            element.Format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if (parameter.Mask <= 7)
        {
          if (parameter.ComponentType == D3D_REGISTER_COMPONENT_UINT32)  
            element.Format = DXGI_FORMAT_R32G32B32_UINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  
            element.Format = DXGI_FORMAT_R32G32B32_SINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) 
            element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        }
        else if (parameter.Mask <= 15)
        {
          if (parameter.ComponentType == D3D_REGISTER_COMPONENT_UINT32)  
            element.Format = DXGI_FORMAT_R32G32B32A32_UINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  
            element.Format = DXGI_FORMAT_R32G32B32A32_SINT;
          else if (parameter.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) 
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
        
        offset += sizeFromFormat(element.Format);

        input_layout.push_back(element);
      }

      ID3D11InputLayout* input_layout_ptr;

      if (FAILED(device->CreateInputLayout(
        input_layout.data(),
        (UINT)input_layout.size(),
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
      &input_layout_ptr
      )))
      {
        LMB_ASSERT(false, "D3D11Shader: Failed to create input layout!");
        return;
      }

      il_ = input_layout_ptr;

      reflection->Release();
    }
  }
}