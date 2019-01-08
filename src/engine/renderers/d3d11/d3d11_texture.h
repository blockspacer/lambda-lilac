#pragma once
#include "assets/texture.h"
#include <d3d11.h>

namespace lambda
{
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    class D3D11Texture
    {
      friend class D3D11RenderTargetView;
      friend class D3D11DepthStencilView;
      friend class D3D11RenderTexture;
      friend class D3D11Context;

    public:
      D3D11Texture(
        asset::VioletTextureHandle texture, 
        ID3D11Device* device, 
        ID3D11DeviceContext* context
      );
      ~D3D11Texture();
      void bind(ID3D11DeviceContext* context, uint8_t slot);
      DXGI_FORMAT getFormat() const;
      ID3D11ShaderResourceView* getSRV() const;
      void generateMips(ID3D11DeviceContext* context) const;
      ID3D11DepthStencilView* getDSV(unsigned char idx) const;
      ID3D11RenderTargetView* getRTV(
        unsigned char idx, 
        unsigned char mip_map
      ) const;

      void pingPong();
      unsigned char pingPongIdx() const;

    protected:
      const ID3D11Texture2D* getTexture(unsigned char idx) const;
      ID3D11Texture2D* getTexture(unsigned char idx);
    
    private:
      void createRenderTargetView(
        ID3D11Device* device, 
        unsigned char mip_count
      );
      void createDepthStencilView(ID3D11Device* device);

    private:
      DXGI_FORMAT format_;
      ID3D11Texture2D* texture_[2];
      ID3D11ShaderResourceView* srv_[2];
      ID3D11DepthStencilView* dsv_[2u];
      Vector<ID3D11RenderTargetView*> rtv_[2u];
      unsigned char texture_index_;
    };
  }
}