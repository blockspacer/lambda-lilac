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
      size_t slot = 0u;
      bool bound = false;
    };

    ///////////////////////////////////////////////////////////////////////////
    class D3D11Shader : public asset::IGPUAsset
    {
    public:
      D3D11Shader(const asset::Shader& shader, D3D11Context* context);
      virtual ~D3D11Shader() override;
      void bind();
      void unbind();
      void bindBuffers();
      uint16_t getStages() const;

      void updateShaderVariable(const platform::ShaderVariable& variable);

      Vector<platform::ShaderBuffer>& getVsBuffers();
      Vector<platform::ShaderBuffer>& getPsBuffers();
      Vector<platform::ShaderBuffer>& getGsBuffers();

    private:
      ID3D10Blob* compile(
        const String& file, 
        const Vector<char>& source, 
        const String& entry, 
        const String& target
      );
      void reflect(
        ID3D10Blob* blob, 
        Vector<platform::ShaderBuffer>& buffers, 
        Vector<D3D11Buffer>& d3d11_buffers
      );
      void reflectInputLayout(ID3D10Blob* blob, ID3D11Device* device);

    private:
      Microsoft::WRL::ComPtr<ID3D11VertexShader>   vs_;
      Microsoft::WRL::ComPtr<ID3D11PixelShader>    ps_;
      Microsoft::WRL::ComPtr<ID3D11GeometryShader> gs_;
      Microsoft::WRL::ComPtr<ID3D11InputLayout>    il_;
      uint16_t stages_ = 0;

      D3D11Context* context_;
      Vector<platform::ShaderBuffer> vs_buffers_;
      Vector<platform::ShaderBuffer> ps_buffers_;
      Vector<platform::ShaderBuffer> gs_buffers_;
      Vector<D3D11Buffer> vs_d3d11_buffers_;
      Vector<D3D11Buffer> ps_d3d11_buffers_;
      Vector<D3D11Buffer> gs_d3d11_buffers_;
    };
  }
}