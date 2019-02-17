#include "d3d11_texture.h"
#include <utils/console.h>
#include <utils/file_system.h>
#include "d3d11_context.h" // TODO (Hilze): Remove this!

namespace lambda
{
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    DXGI_FORMAT formatToD3DFormat(const TextureFormat& format)
    {
      switch (format)
      {
			case TextureFormat::kA8:
				return DXGI_FORMAT::DXGI_FORMAT_A8_UNORM;
      case TextureFormat::kR8G8B8A8:
        return DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
      case TextureFormat::kB8G8R8A8:
        return DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
      case TextureFormat::kR16G16B16A16:
        return DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
      case TextureFormat::kR32G32B32A32:
        return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
      case TextureFormat::kR16G16:
        return DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT;
      case TextureFormat::kR32G32:
        return DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
      case TextureFormat::kR16:
        return DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT;
      case TextureFormat::kR32:
        return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
      case TextureFormat::kR24G8:
        return DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
      case TextureFormat::kBC1:
        return DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;
      case TextureFormat::kBC2:
        return DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM;
      case TextureFormat::kBC3:
        return DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM;
      case TextureFormat::kBC4:
        return DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM;
      case TextureFormat::kBC5:
        return DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM;
      case TextureFormat::kBC6:
        return DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16;
      case TextureFormat::kBC7:
        return DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM;
      }

      return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11Texture::D3D11Texture(
      asset::VioletTextureHandle texture, 
      ID3D11Device* device, 
      ID3D11DeviceContext* context) 
      : format_(formatToD3DFormat(texture->getLayer(0u).getFormat()))
      , texture_index_(0u)
    {
      LMB_ASSERT(texture->getLayerCount() > 0,
        "D3D11 TEXTURE: No layers were added to this texture.");
      
      texture_[0u] = texture_[1u] = nullptr;
      srv_[0u] = srv_[1u] = nullptr;
      dsv_[0u] = dsv_[1u] = nullptr;

      if (texture->getLayer(0u).getWidth() == 0u || 
        texture->getLayer(0u).getHeight() == 0u)
      {
        //LMB_ASSERT(false, "D3D11 TEXTURE: Provided Width and/or Height was 0.");
        return;
      }

      D3D11_TEXTURE2D_DESC desc{};
      Vector<D3D11_SUBRESOURCE_DATA> datas;
      
      is_render_target = (texture->getLayer(0u).getFlags() & 
        kTextureFlagIsRenderTarget) ? true : false;
			is_dynamic = (texture->getLayer(0u).getFlags() & 
				kTextureFlagDynamicData) ? true : false;
      UINT bind_flags    = is_render_target ? 
        (format_ == DXGI_FORMAT_R24G8_TYPELESS ? D3D11_BIND_DEPTH_STENCIL : 
          D3D11_BIND_RENDER_TARGET) : 0u;
      bool contains_data = !texture->getLayer(0u).getData().empty();

      desc.Width              = texture->getLayer(0u).getWidth();
      desc.Height             = texture->getLayer(0u).getHeight();
      desc.ArraySize          = texture->getLayerCount();
      desc.MipLevels          = texture->getLayer(0u).getMipCount();
      desc.Format             = format_;
      desc.SampleDesc.Count   = 1u;
      desc.SampleDesc.Quality = 0u;
      desc.Usage              = is_dynamic ? D3D11_USAGE_DYNAMIC :
				(is_render_target || !contains_data) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
      desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | bind_flags;
      desc.CPUAccessFlags     = is_dynamic ? D3D11_CPU_ACCESS_WRITE : 0u;
      desc.MiscFlags          = 0u | 
        (desc.ArraySize == 6u ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0u);

      datas.resize(contains_data ? desc.ArraySize * desc.MipLevels : 0u);

      for (uint32_t layer = 0u; layer < desc.ArraySize && 
        contains_data; ++layer)
      {
        uint32_t w = texture->getLayer(layer).getWidth();
        uint32_t h = texture->getLayer(layer).getHeight();

        uint32_t offset = 0u;
        if (texture->getLayer(layer).getData()[0u] == 0x20534444)
        {
          // TODO (Hilze): Read the DDS header and find out
          // if it uses the extended or normal header.
          
          if (format_ == DXGI_FORMAT_BC6H_UF16)
            offset = 148u;
          else
            offset = 128u;
        }

        const char* data = 
          (const char*)texture->getLayer(layer).getData().data();

        for (uint32_t mip = 0u; mip < desc.MipLevels; ++mip)
        {
          uint32_t bpp, bpr, bpl;
          calculateImageMemory(
            texture->getLayer(layer).getFormat(), 
            w, 
            h, 
            bpp, 
            bpr, 
            bpl
          );
          D3D11_SUBRESOURCE_DATA& subresource = 
            datas[layer * desc.MipLevels + mip];
          subresource.pSysMem          = data + offset;
          subresource.SysMemPitch      = bpr;
          subresource.SysMemSlicePitch = bpl;
          offset += subresource.SysMemSlicePitch;

          w /= 2u;
          h /= 2u;
        }
      }

      // Create the textures.
      for (unsigned char i = 0u; i < (is_render_target ? 2u : 1u); ++i)
      {
        HRESULT result = device->CreateTexture2D(
          &desc, 
          datas.data(), 
          &texture_[i]
        );

        LMB_ASSERT(SUCCEEDED(result), 
          "D3D11 TEXTURE: Failed to create texture!");
      }

      // SRV.
      DXGI_FORMAT srv_format = format_;
      if (format_ == DXGI_FORMAT_R24G8_TYPELESS)
        srv_format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
      if (texture->getLayerCount() == 1)
        srv_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
      else if ((desc.MiscFlags | D3D11_RESOURCE_MISC_TEXTURECUBE) != 0)
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
      else
        srv_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;

      srv_desc.Format                   = srv_format;
      srv_desc.Texture2D.MipLevels      = desc.MipLevels;
      srv_desc.Texture2DArray.MipLevels = desc.MipLevels;
      srv_desc.Texture2DArray.ArraySize = texture->getLayerCount();

      // Create shader resource view.
      for (unsigned char i = 0u; i < (is_render_target ? 2u : 1u); ++i)
      {
        HRESULT result = device->CreateShaderResourceView(
          texture_[i], 
          &srv_desc, 
          &srv_[i]
        );
        LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Failed to create srv!");
      }

      if (is_render_target)
      {
        if (format_ == DXGI_FORMAT_R24G8_TYPELESS)
          createDepthStencilView(device);
        else
          createRenderTargetView(device, desc.MipLevels);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11Texture::~D3D11Texture()
    {
      for (uint32_t i = 0u; i < 2u; ++i)
      {
        if (srv_[i] != nullptr)
          srv_[i]->Release();
        for (auto& rtv : rtv_[i])
          if (rtv != nullptr)
            rtv->Release();
        if (dsv_[i] != nullptr)
          dsv_[i]->Release();
        if (texture_[i] != nullptr)
          texture_[i]->Release();
      }
    }

		///////////////////////////////////////////////////////////////////////////
		void D3D11Texture::update(
			asset::VioletTextureHandle texture, 
			ID3D11Device* device, 
			ID3D11DeviceContext* context)
		{
			for (uint32_t i = 0; i < texture->getLayerCount(); ++i)
			{
				auto& layer = texture->getLayer(i);

				if (layer.isDirty())
				{
					D3D11_MAPPED_SUBRESOURCE mapped_resource;
					HRESULT result = context->Map(texture_[0], i, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
					LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Could not bind texture!");
					memcpy(mapped_resource.pData, layer.getData().data(), mapped_resource.DepthPitch);
					context->Unmap(texture_[0], i);

					layer.clean();
				}
			}
		}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::bind(ID3D11DeviceContext* context, uint8_t slot)
    {
      context->PSSetShaderResources(slot, 1u, &srv_[texture_index_]);
    }

    ///////////////////////////////////////////////////////////////////////////
    DXGI_FORMAT D3D11Texture::getFormat() const
    {
      return format_;
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11ShaderResourceView* D3D11Texture::getSRV() const
    {
      return srv_[texture_index_];
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::generateMips(ID3D11DeviceContext* context) const
    {
      context->GenerateMips(srv_[texture_index_]);
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11DepthStencilView* D3D11Texture::getDSV(unsigned char idx) const
    {
      LMB_ASSERT(dsv_[idx] != nullptr, "TODO (Hilze): Fill in");
      return dsv_[idx];
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11RenderTargetView* D3D11Texture::getRTV(
      unsigned char idx, 
      unsigned char mip_map) const
    {
      LMB_ASSERT(rtv_[idx][mip_map] != nullptr, "TODO (Hilze): Fill in");
      return rtv_[idx][mip_map];
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::pingPong()
    {
      LMB_ASSERT(srv_[1u] != nullptr, "TODO (Hilze): Fill in");
      texture_index_ = (texture_index_ == 0u) ? 1u : 0u;
    }

    ///////////////////////////////////////////////////////////////////////////
    unsigned char D3D11Texture::pingPongIdx() const
    {
      return texture_index_;
    }

    ///////////////////////////////////////////////////////////////////////////
    const ID3D11Texture2D* D3D11Texture::getTexture(unsigned char idx) const
    {
      LMB_ASSERT(srv_[idx] != nullptr, "TODO (Hilze): Fill in");
      return texture_[idx];
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11Texture2D* D3D11Texture::getTexture(unsigned char idx)
    {
      LMB_ASSERT(srv_[idx] != nullptr, "TODO (Hilze): Fill in");
      return texture_[idx];
    }


    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::createRenderTargetView(
      ID3D11Device* device, 
      unsigned char mip_count)
    {
      D3D11_RENDER_TARGET_VIEW_DESC desc{};
      desc.Format        = format_;
      desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      
      for (unsigned char i = 0u; i < 2u; ++i)
      {
        rtv_[i].resize(mip_count);
        for (unsigned char m = 0u; m < mip_count; ++m)
        {
          desc.Texture2D.MipSlice = m;
          HRESULT result = device->CreateRenderTargetView(
            texture_[i], 
            &desc, 
            &rtv_[i][m]
          );
          LMB_ASSERT(SUCCEEDED(result), 
            "D3D11 TEXTURE: Failed to create rtv!");
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::createDepthStencilView(ID3D11Device* device)
    {
      D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
      desc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT;
      desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

      for (unsigned char i = 0u; i < 2u; ++i)
      {
        HRESULT result = 
          device->CreateDepthStencilView(texture_[i], &desc, &dsv_[i]);
        LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Failed to create dsv!");
      }
    }
  }
}