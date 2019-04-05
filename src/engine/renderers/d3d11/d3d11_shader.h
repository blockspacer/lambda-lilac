#pragma once
#include "assets/shader.h"
#include "d3d11_mesh.h"
#include "platform/shader_variable_manager.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D10Blob;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11GeometryShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;

namespace lambda
{
  namespace windows
  {
    class D3D11Context;
    class D3D11RenderBuffer;

    ///////////////////////////////////////////////////////////////////////////
    struct D3D11Buffer
    {
      D3D11RenderBuffer* buffer;
	  platform::ShaderBuffer shader_buffer;
      size_t slot = 0u;
      bool bound = false;
	  ShaderStages stage;
    };

    ///////////////////////////////////////////////////////////////////////////
    class D3D11Shader
    {
    public:
      D3D11Shader(asset::VioletShaderHandle shader, D3D11Context* context);
      ~D3D11Shader();
      void bind();
      void unbind();
      void bindBuffers();
	  Vector<uint32_t> getStages() const;

      void updateShaderVariable(const platform::ShaderVariable& variable);

      Vector<D3D11Buffer>& getBuffers();
	  const Vector<VioletShaderResource>& getTextures();
	  const Vector<VioletShaderResource>& getSamplers();

    private:
      void reflectInputLayout(ID3D10Blob* blob, ID3D11Device* device);

    private:
      ID3D11VertexShader*   vs_;
      ID3D11PixelShader*    ps_;
      ID3D11GeometryShader* gs_;
      ID3D11InputLayout*    il_;
	  
	  Vector<VioletShaderResource> textures_;
	  Vector<VioletShaderResource> samplers_;
      Vector<D3D11Buffer> buffers_;
	  Vector<uint32_t> stages_;

      D3D11Context* context_;
    };
  }
}