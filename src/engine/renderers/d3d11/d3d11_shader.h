#pragma once
#include "assets/shader.h"
#include "d3d11_mesh.h"

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
    class D3D11Shader
    {
    public:
      D3D11Shader(asset::VioletShaderHandle shader, D3D11Context* context);
      ~D3D11Shader();
      void bind();
	  Vector<uint32_t> getStages() const;

	  const Vector<VioletShaderResource>& getBuffers();
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
	  Vector<VioletShaderResource> buffers_;
	  Vector<uint32_t> stages_;

      D3D11Context* context_;
    };
  }
}