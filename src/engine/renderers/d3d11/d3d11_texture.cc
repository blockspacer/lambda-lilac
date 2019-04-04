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
	  case TextureFormat::kD32:
		return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
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
      
	  textures_[0u] = textures_[1u] = nullptr;
	  srvs_[0u] = srvs_[1u] = nullptr;

      if (texture->getLayer(0u).getWidth() == 0u || 
        texture->getLayer(0u).getHeight() == 0u)
      {
        LMB_ASSERT(false, "D3D11 TEXTURE: Provided Width and/or Height was 0.");
        return;
      }

      D3D11_TEXTURE2D_DESC desc{};
      Vector<D3D11_SUBRESOURCE_DATA> datas;
      
      is_render_target_ = (texture->getLayer(0u).getFlags() & 
        kTextureFlagIsRenderTarget) ? true : false;
	  is_dynamic_ = ((texture->getLayer(0u).getFlags() &
				kTextureFlagDynamicData) && !is_render_target_) ? true : false;
	  bool from_dds = (texture->getLayer(0u).getFlags() & kTextureFlagFromDDS)
				? true : false;
	  bool is_dsv = (format_ == DXGI_FORMAT_R24G8_TYPELESS || format_ == DXGI_FORMAT_R32_TYPELESS);
	  UINT bind_flags    = is_render_target_ ? 
        (is_dsv ? D3D11_BIND_DEPTH_STENCIL : 
          D3D11_BIND_RENDER_TARGET) : 0u;
      bool contains_data = !texture->getLayer(0u).getData().empty() || from_dds;

      desc.Width              = texture->getLayer(0u).getWidth();
      desc.Height             = texture->getLayer(0u).getHeight();
      desc.ArraySize          = texture->getLayerCount();
      desc.MipLevels          = texture->getLayer(0u).getMipCount();
      desc.Format             = format_;
      desc.SampleDesc.Count   = 1u;
      desc.SampleDesc.Quality = 0u;
      desc.Usage              = is_dynamic_ ? D3D11_USAGE_DYNAMIC :
        (is_render_target_ || !contains_data) ?
        D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
      desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | bind_flags;
      desc.CPUAccessFlags     = is_dynamic_ ? D3D11_CPU_ACCESS_WRITE : 0u;
      desc.MiscFlags          = 0u | 
        (desc.ArraySize == 6u ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0u);

      datas.resize(contains_data ? desc.ArraySize * desc.MipLevels : 0u);

	  Vector<Vector<char>> dds_data(desc.ArraySize);

      for (uint32_t layer = 0u; layer < desc.ArraySize && 
        contains_data; ++layer)
      {
        uint32_t w = texture->getLayer(layer).getWidth();
        uint32_t h = texture->getLayer(layer).getHeight();


		const char* data = texture->getLayer(layer).getData().data();
		if (!data && from_dds)
		{
          dds_data[layer] = asset::TextureManager::getInstance()->getData(texture);
		  data = dds_data[layer].data();
		}
					

        uint32_t offset = 0u;
        if (*((uint32_t*)data) == 0x20534444)
        {
          // TODO (Hilze): Read the DDS header and find out
          // if it uses the extended or normal header.
          
          if (format_ == DXGI_FORMAT_BC6H_UF16)
            offset = 148u;
          else
            offset = 128u;
        }

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
      for (unsigned char i = 0u; i < (is_render_target_ ? 2u : 1u); ++i)
      {
        HRESULT result = device->CreateTexture2D(
          &desc, 
          datas.data(), 
          &textures_[i]
        );

        LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Failed to create texture | %i", result);
      }

      // Create shader resource view.
	  createSRVs(device, desc.ArraySize, desc.MipLevels);

	  layers_[0].resize(desc.ArraySize);
	  layers_[1].resize(desc.ArraySize);
      if (is_render_target_)
      {
        if (is_dsv)
          createDSVs(device, desc.ArraySize, desc.MipLevels);
        else
          createRTVs(device, desc.ArraySize, desc.MipLevels);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11Texture::~D3D11Texture()
    {
      for (uint32_t i = 0u; i < 2u; ++i)
      {
        if (srvs_[i] != nullptr)
          srvs_[i]->Release();
		for (auto& layer : layers_[i])
		{
			for (auto dsv : layer.dsvs)
				dsv->Release();
			for (auto rtv : layer.rtvs)
				rtv->Release();
		}
		layers_[i].clear();
        if (textures_[i] != nullptr)
          textures_[i]->Release();
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

				// TODO (Hilze): Find out why DDSses are dynamic.
				if (layer.isDirty() && !(layer.getFlags() & kTextureFlagFromDDS))
				{
					D3D11_MAPPED_SUBRESOURCE mapped_resource;
					HRESULT result = context->Map(textures_[0], i, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
					LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Could not bind texture | %i", result);
					memcpy(mapped_resource.pData, layer.getData().data(), mapped_resource.DepthPitch);
					context->Unmap(textures_[0], i);

					layer.clean();
				}
			}
		}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::bind(ID3D11DeviceContext* context, uint8_t slot)
    {
      context->PSSetShaderResources(slot, 1u, &srvs_[texture_index_]);
    }

    ///////////////////////////////////////////////////////////////////////////
    DXGI_FORMAT D3D11Texture::getFormat() const
    {
      return format_;
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11ShaderResourceView* D3D11Texture::getSRV() const
    {
      return srvs_[texture_index_];
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::generateMips(ID3D11DeviceContext* context) const
    {
      context->GenerateMips(srvs_[texture_index_]);
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11DepthStencilView* D3D11Texture::getDSV(
	  unsigned char idx,
	  unsigned char layer,
	  unsigned char mip_map) const
    {
      LMB_ASSERT(layers_[idx].size() >= layer && layers_[idx][layer].dsvs.size() >= mip_map, "D3D11 TEXTURE: getDSV(Idx: %i, Layer: %i, MipMap: %i)", idx, layer, mip_map);
      return layers_[idx][layer].dsvs[mip_map];
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11RenderTargetView* D3D11Texture::getRTV(
	  unsigned char idx,
      unsigned char layer, 
      unsigned char mip_map) const
    {
      LMB_ASSERT(layers_[idx].size() >= layer && layers_[idx][layer].rtvs.size() >= mip_map, "D3D11 TEXTURE: getRTV(Idx: %i, Layer: %i, MipMap: %i)", idx, layer, mip_map);
      return layers_[idx][layer].rtvs[mip_map];
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::pingPong()
    {
      LMB_ASSERT(srvs_[1u] != nullptr, "D3D11 TEXTURE: Only one texture was created");
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
      LMB_ASSERT(srvs_[idx] != nullptr, "D3D11 TEXTURE: getTexture(%i) : Texture at index was nullptr", idx);
      return textures_[idx];
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11Texture2D* D3D11Texture::getTexture(unsigned char idx)
    {
      LMB_ASSERT(srvs_[idx] != nullptr, "D3D11 TEXTURE: getTexture(%i) : Texture at index was nullptr", idx);
      return textures_[idx];
    }

	///////////////////////////////////////////////////////////////////////////
	void D3D11Texture::createSRVs(
      ID3D11Device* device,
      unsigned char layer_count,
      unsigned char mip_count)
	{
		
      // SRV.
      DXGI_FORMAT format = format_;
      if (format_ == DXGI_FORMAT_R24G8_TYPELESS)
        format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      if (format_ == DXGI_FORMAT_R32_TYPELESS)
        format = DXGI_FORMAT_R32_FLOAT;

      D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
      if (layer_count == 1)
        desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
      else if (layer_count == 6)
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
      else
        desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;

	  desc.Format                   = format;
	  desc.Texture2D.MipLevels      = mip_count;
	  desc.Texture2DArray.MipLevels = mip_count;
	  desc.Texture2DArray.ArraySize = layer_count;

	  for (unsigned char i = 0u; i < (is_render_target_ ? 2u : 1u); ++i)
	  {
		  HRESULT result = device->CreateShaderResourceView(
			  textures_[i],
			  &desc,
			  &srvs_[i]
		  );
		  LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Failed to create srv | %i", result);
	  }
	}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::createRTVs(
      ID3D11Device* device,
      unsigned char layer_count,
      unsigned char mip_count)
    {
      D3D11_RENDER_TARGET_VIEW_DESC desc{};
      desc.Format        = format_;
      desc.ViewDimension = layer_count == 1 ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DARRAY;

	  for (unsigned char i = 0u; i < (is_render_target_ ? 2u : 1u); ++i)
	  {
		  for (unsigned char l = 0u; l < layer_count; ++l)
		  {
			  layers_[i][l].rtvs.resize(mip_count);
			  for (unsigned char m = 0u; m < mip_count; ++m)
			  {
				  desc.Texture2DArray.MipSlice = desc.Texture2D.MipSlice = m;
				  desc.Texture2DArray.ArraySize = 1;
				  desc.Texture2DArray.FirstArraySlice = l;
				  HRESULT result = device->CreateRenderTargetView(
					  textures_[i],
					  &desc,
					  &layers_[i][l].rtvs[m]
				  );
				  LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Failed to create rtv | %i", result);
			  }
		  }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Texture::createDSVs(
      ID3D11Device* device,
      unsigned char layer_count,
      unsigned char mip_count)
    {
      D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
      desc.Format        = format_ == DXGI_FORMAT_R24G8_TYPELESS ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
      desc.ViewDimension = layer_count == 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	  
	  for (unsigned char i = 0u; i < (is_render_target_ ? 2u : 1u); ++i)
	  {
		  for (unsigned char l = 0u; l < layer_count; ++l)
		  {
			  layers_[i][l].dsvs.resize(mip_count);
			  for (unsigned char m = 0u; m < mip_count; ++m)
			  {
				  desc.Texture2DArray.MipSlice = desc.Texture2D.MipSlice = m;
				  desc.Texture2DArray.ArraySize = 1;
				  desc.Texture2DArray.FirstArraySlice = l;
				  HRESULT result = device->CreateDepthStencilView(
					  textures_[i],
					  &desc,
					  &layers_[i][l].dsvs[m]
				  );
				  LMB_ASSERT(SUCCEEDED(result), "D3D11 TEXTURE: Failed to create dsv | %i", result);
			  }
		  }
      }
    }
  }
}