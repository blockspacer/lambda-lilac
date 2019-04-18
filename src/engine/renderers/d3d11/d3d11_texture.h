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
			void update(
				asset::VioletTextureHandle texture,
				ID3D11Device* device,
				ID3D11DeviceContext* context);
      void bind(ID3D11DeviceContext* context, uint8_t slot);
      DXGI_FORMAT getFormat() const;
      ID3D11ShaderResourceView* getSRV() const;
      void generateMips(ID3D11DeviceContext* context) const;
      ID3D11DepthStencilView* getDSV(
        unsigned char layer,
        unsigned char mip_map
	  ) const;
      ID3D11RenderTargetView* getRTV(
        unsigned char layer,
        unsigned char mip_map
      ) const;

    protected:
      const ID3D11Texture2D* getTexture() const;
      ID3D11Texture2D* getTexture();
    
    private:
	  void createSRVs(
        ID3D11Device* device,
        unsigned char layer_count,
        unsigned char mip_count
      );
      void createRTVs(
        ID3D11Device* device,
        unsigned char layer_count,
        unsigned char mip_count
      );
	  void createDSVs(
        ID3D11Device* device,
        unsigned char layer_count,
        unsigned char mip_count
      );

    private:
      struct Layer
	  {
		Vector<ID3D11DepthStencilView*> dsvs;
		Vector<ID3D11RenderTargetView*> rtvs;
	  };
      DXGI_FORMAT format_;
	  ID3D11ShaderResourceView* srv_;
	  ID3D11Texture2D* texture_;
	  Vector<Layer> layers_;
	  
	  bool is_render_target_;
	  bool is_dynamic_;
    };
  }
}