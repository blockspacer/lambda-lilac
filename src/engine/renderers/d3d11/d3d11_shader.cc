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
		class IncludeHandler : public ID3DInclude
		{
		public:
			IncludeHandler(String file_path)
				: file_path_(file_path)
			{
				file_path_ = FileSystem::RemoveName(file_path_);
			}


			// Inherited via ID3DInclude
			STDMETHOD(Open)(
				THIS_ D3D_INCLUDE_TYPE IncludeType, 
				LPCSTR pFileName, 
				LPCVOID pParentData, 
				LPCVOID *ppData, 
				UINT *pBytes) override
			{
				Vector<char> bytes = 
					FileSystem::FileToVector(file_path_ + '/' + pFileName);
				UINT size = (UINT)bytes.size();

				*ppData = foundation::Memory::allocate(size);
				memcpy((void*)*ppData, bytes.data(), size);
				*pBytes = size;
				return S_OK;
			}

			STDMETHOD(Close)(THIS_ LPCVOID pData) override
			{
				foundation::Memory::deallocate((void*)pData);
				return S_OK;
			}

			String file_path_;
		};

		///////////////////////////////////////////////////////////////////////////
		ID3D10Blob* compile(
			const String& file,
			const Vector<char>& source,
			const String& entry,
			const String& target)
		{
			if (source.empty())
				return nullptr;

			ID3D10Blob* blob;

			if (source.size() > 3 && (source[0] == 'D' && source[1] == 'X' && source[2] == 'B' && source[3] == 'C'))
			{
				HRESULT res = D3DCreateBlob(source.size(), &blob);
				LMB_ASSERT(res == S_OK, "[SHADER] Failed to create blob!");
				memcpy(blob->GetBufferPointer(), source.data(), source.size());
			}
			else
			{
				ID3D10Blob* error;

				IncludeHandler include_handler(file);

				HRESULT result = D3DCompile(
					(void*)source.data(),
					source.size(),
					file.c_str(), 0,
					&include_handler,
					entry.c_str(),
					target.c_str(),
					0,
					0,
					&blob,
					&error
				);

				if (FAILED(result))
				{
					if (error)
					{
						String err =
							"[SHADER] Failed to compile shader with message:\n" +
							String((char*)error->GetBufferPointer());
						LMB_ASSERT(false, err.c_str());
						error->Release();
					}

					if (blob)
						blob->Release();
				}
			}

			return blob;
		}

    ///////////////////////////////////////////////////////////////////////////
    D3D11Shader::D3D11Shader(
			asset::VioletShaderHandle shader,
      D3D11Context* context)
      : context_(context)
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

				reflect(vs_blob, vs_buffers_, vs_d3d11_buffers_);
				reflectInputLayout(vs_blob, context_->getD3D11Device());
				vs_blob->Release();
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

				reflect(ps_blob, ps_buffers_, ps_d3d11_buffers_);
				ps_blob->Release();
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

				reflect(gs_blob, gs_buffers_, gs_d3d11_buffers_);
				gs_blob->Release();
			}
    }
    
    ///////////////////////////////////////////////////////////////////////////
    D3D11Shader::~D3D11Shader()
    {
      ps_.Reset();
      vs_.Reset();
      il_.Reset();

      ps_buffers_.resize(0u);
      vs_buffers_.resize(0u);
      gs_buffers_.resize(0u);

      for (auto& buffer : ps_d3d11_buffers_) 
        context_->freeRenderBuffer((platform::IRenderBuffer*&)buffer.buffer);
      for (auto& buffer : vs_d3d11_buffers_) 
        context_->freeRenderBuffer((platform::IRenderBuffer*&)buffer.buffer);
      for (auto& buffer : gs_d3d11_buffers_) 
        context_->freeRenderBuffer((platform::IRenderBuffer*&)buffer.buffer);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Shader::bind()
    {
      context_->getD3D11Context()->VSSetShader(vs_.Get(), nullptr, 0);
      context_->getD3D11Context()->PSSetShader(ps_.Get(), nullptr, 0);
      context_->getD3D11Context()->GSSetShader(gs_.Get(), nullptr, 0);
      context_->getD3D11Context()->IASetInputLayout(il_.Get());
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Shader::unbind()
    {
      for (auto& buffer : vs_d3d11_buffers_)
        buffer.bound = false;
      for (auto& buffer : ps_d3d11_buffers_)
        buffer.bound = false;
      for (auto& buffer : gs_d3d11_buffers_)
        buffer.bound = false;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Shader::bindBuffers()
    {
      for (size_t i = 0u; i < vs_buffers_.size(); ++i)
      {
        auto& buffer = vs_buffers_.at(i);
        auto& d3d11_buffer = vs_d3d11_buffers_.at(i);

        if (buffer.getChanged() == true)
        {
          buffer.setChanged(false);
          context_->getD3D11Context()->UpdateSubresource(
            d3d11_buffer.buffer->getBuffer(), 
            0u, 
            NULL, 
            buffer.getData(), 
            0u, 
            0u
          );
          d3d11_buffer.bound = false;
        }
        if (!d3d11_buffer.bound)
        {
          d3d11_buffer.bound = true;
          ID3D11Buffer* buffer =
            (ID3D11Buffer*)d3d11_buffer.buffer->getBuffer();
          
          context_->getD3D11Context()->VSSetConstantBuffers(
            (UINT)d3d11_buffer.slot, 
            1u, 
            &buffer
          );
        }
      }
      for (size_t i = 0u; i < ps_buffers_.size(); ++i)
      {
        auto& buffer = ps_buffers_.at(i);
        auto& d3d11_buffer = ps_d3d11_buffers_.at(i);

        if (buffer.getChanged() == true)
        {
          buffer.setChanged(false);
          
          context_->getD3D11Context()->UpdateSubresource(
            d3d11_buffer.buffer->getBuffer(), 
            0u, 
            NULL, 
            buffer.getData(), 
            0u, 
            0u
          );
          
          d3d11_buffer.bound = false;
        }
        if (!d3d11_buffer.bound)
        {
          d3d11_buffer.bound = true;
          ID3D11Buffer* buffer = 
            (ID3D11Buffer*)d3d11_buffer.buffer->getBuffer();
          
          context_->getD3D11Context()->PSSetConstantBuffers(
            (UINT)d3d11_buffer.slot, 
            1u, 
            &buffer
          );
        }
      }
      for (size_t i = 0u; i < gs_buffers_.size(); ++i)
      {
        auto& buffer = gs_buffers_.at(i);
        auto& d3d11_buffer = gs_d3d11_buffers_.at(i);

        if (buffer.getChanged() == true)
        {
          buffer.setChanged(false);
          
          context_->getD3D11Context()->UpdateSubresource(
            d3d11_buffer.buffer->getBuffer(), 
            0u, 
            NULL, 
            buffer.getData(), 
            0u, 
            0u
          );
          
          d3d11_buffer.bound = false;
        }
        if (!d3d11_buffer.bound)
        {
          d3d11_buffer.bound = true;
          ID3D11Buffer* buffer = 
            (ID3D11Buffer*)d3d11_buffer.buffer->getBuffer();
          
          context_->getD3D11Context()->GSSetConstantBuffers(
            (UINT)d3d11_buffer.slot, 
            1u, 
            &buffer
          );
        }
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
		Vector<uint32_t> D3D11Shader::getStages() const
    {
      return stages_;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Shader::updateShaderVariable(
      const platform::ShaderVariable& variable)
    {
      for (size_t i = 0u; i < vs_buffers_.size(); ++i)
      {
        auto& buffer = vs_buffers_.at(i);

        for (auto& v : buffer.getVariables())
        {
          if (variable.name == v.name)
          {
            memcpy(v.data, variable.data.data(), v.size);
            buffer.setChanged(true);
            continue;
          }
        }
      }
      
      for (size_t i = 0u; i < ps_buffers_.size(); ++i)
      {
        auto& buffer = ps_buffers_.at(i);

        for (auto& v : buffer.getVariables())
        {
          if (variable.name == v.name)
          {
            memcpy(v.data, variable.data.data(), v.size);
            buffer.setChanged(true);
            continue;
          }
        }
      }

      for (size_t i = 0u; i < gs_buffers_.size(); ++i)
      {
        auto& buffer = gs_buffers_.at(i);

        for (auto& v : buffer.getVariables())
        {
          if (variable.name == v.name)
          {
            memcpy(v.data, variable.data.data(), v.size);
            buffer.setChanged(true);
            continue;
          }
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<platform::ShaderBuffer>& D3D11Shader::getVsBuffers()
    {
      return vs_buffers_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<platform::ShaderBuffer>& D3D11Shader::getPsBuffers()
    {
      return ps_buffers_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<platform::ShaderBuffer>& D3D11Shader::getGsBuffers()
    {
      return gs_buffers_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Shader::reflect(
      ID3D10Blob* blob, 
      Vector<platform::ShaderBuffer>& buffers, 
      Vector<D3D11Buffer>& d3d11_buffers)
    {
      ID3D11ShaderReflection* reflection = nullptr;
      if (FAILED(D3DReflect(
        blob->GetBufferPointer(), 
        blob->GetBufferSize(), 
        IID_PPV_ARGS(&reflection))))
      {
        MessageBox(NULL, "Failed to do shader reflection!", "Shader", 0);
        return;
      }

      D3D11_SHADER_DESC shader_description{};
      reflection->GetDesc(&shader_description);

      for (uint32_t i = 0; i < shader_description.ConstantBuffers; ++i)
      {
        ID3D11ShaderReflectionConstantBuffer* constant_buffer = 
          reflection->GetConstantBufferByIndex(i);
        D3D11_SHADER_BUFFER_DESC buffer_desc;
        constant_buffer->GetDesc(&buffer_desc);

        Vector<platform::BufferVariable> variables;
        foundation::SharedPointer<void> buffer_data = 
          foundation::Memory::makeShared(
            foundation::Memory::allocate(buffer_desc.Size)
          );
        D3D11Buffer d3d11_buffer;

        for (unsigned int j = 0; j < shader_description.BoundResources; ++j)
        {
          D3D11_SHADER_INPUT_BIND_DESC input_desc;
          reflection->GetResourceBindingDesc(j,&input_desc);

          if (strcmp(buffer_desc.Name, input_desc.Name) == 0)
          {
            d3d11_buffer.slot = input_desc.BindPoint;
          }
        }

        for (uint32_t j = 0; j < buffer_desc.Variables; ++j)
        {
          ID3D11ShaderReflectionVariable* variable = 
            constant_buffer->GetVariableByIndex(j);
          D3D11_SHADER_VARIABLE_DESC variable_desc;
          variable->GetDesc(&variable_desc);

          platform::ShaderVariable::Type type;

          D3D11_SHADER_TYPE_DESC type_desc;
          variable->GetType()->GetDesc(&type_desc);

          switch (type_desc.Type)
          {
          case D3D_SHADER_VARIABLE_TYPE::D3D10_SVT_BOOL:
            LMB_ASSERT(false, 
              "SHADER REFLECTION: Does not currently support booleans");
            break;
          case D3D_SHADER_VARIABLE_TYPE::D3D10_SVT_INT:
            LMB_ASSERT(false, 
              "SHADER REFLECTION: Does not currently support integers");
            break;
          case D3D_SHADER_VARIABLE_TYPE::D3D10_SVT_FLOAT:
          {
            int rows = type_desc.Rows;
            int cols = type_desc.Columns;

            if (rows == 1)
            {
              if      (cols == 1) 
                type = platform::ShaderVariable::Type::kFloat1;
              else if (cols == 2) 
                type = platform::ShaderVariable::Type::kFloat2;
              else if (cols == 3)
                type = platform::ShaderVariable::Type::kFloat3;
              else if (cols == 4)
                type = platform::ShaderVariable::Type::kFloat4;
            }
            else if (rows == 2)
            {
              type = platform::ShaderVariable::Type::kFloat2x2;
            }
            else if (rows == 3)
            {
              type = platform::ShaderVariable::Type::kFloat3x3;
            }
            else if (rows == 4)
            {
              type = platform::ShaderVariable::Type::kFloat4x4;
            }

            break;
          }
          case D3D_SHADER_VARIABLE_TYPE::D3D10_SVT_STRING:
            LMB_ASSERT(false, 
              "SHADER REFLECTION: Does not currently support strings");
            break;
          default:
            LMB_ASSERT(false, 
              "SHADER REFLECTION: Found a variable of unknown type");
            break;
          }

          void* data = (char*)buffer_data.get() + variable_desc.StartOffset;

          if (variable_desc.DefaultValue != nullptr)
          {
            memcpy(data, variable_desc.DefaultValue, variable_desc.Size);
          }
          
          variables.push_back(
            platform::BufferVariable{ 
              0u, 
              variable_desc.Size, 
              Name(variable_desc.Name),
              data 
            }
          );
        }

        d3d11_buffer.buffer = (D3D11RenderBuffer*)context_->allocRenderBuffer(
          buffer_desc.Size, 
          platform::IRenderBuffer::kFlagConstant, 
          nullptr
        );

        buffers.push_back(
          platform::ShaderBuffer(
            Name(buffer_desc.Name), 
            variables, 
            buffer_data
          )
        );
        
        d3d11_buffers.push_back(d3d11_buffer);
      }

      reflection->Release();
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

        // Instancing
        if (String(element.SemanticName).find("instance_") != String::npos)
        {
					semantic = semantic.substr(strlen("instance_"));
          element.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
          element.InstanceDataStepRate = 1;
        }

        element.SemanticIndex        = parameter.SemanticIndex;
        element.AlignedByteOffset    = offset;
        element.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        element.InstanceDataStepRate = 0;

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