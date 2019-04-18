#include "vulkan_texture.h"
#include "vulkan_renderer.h"

#include <utils/console.h>
#include <utils/file_system.h>

namespace lambda
{
  namespace linux
  {
    ///////////////////////////////////////////////////////////////////////////
    VkFormat formatToVulkanFormat(const TextureFormat& format)
    {
      switch (format)
      {
	  case TextureFormat::kA8:
		return VkFormat::VK_FORMAT_R8_UNORM;
      case TextureFormat::kR8G8B8A8:
        return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
      case TextureFormat::kB8G8R8A8:
        return VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
      case TextureFormat::kR16G16B16A16:
        return VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
      case TextureFormat::kR32G32B32A32:
        return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
      case TextureFormat::kR16G16:
        return VkFormat::VK_FORMAT_R16G16_SFLOAT;
      case TextureFormat::kR32G32:
        return VkFormat::VK_FORMAT_R32G32_SFLOAT;
      case TextureFormat::kR16:
        return VkFormat::VK_FORMAT_R16_SFLOAT;
	  case TextureFormat::kR32:
        return VkFormat::VK_FORMAT_R32_SFLOAT;
	  case TextureFormat::kD32:
		return VkFormat::VK_FORMAT_D32_SFLOAT;
      case TextureFormat::kR24G8:
        return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;
      case TextureFormat::kBC1:
        return VkFormat::VK_FORMAT_BC1_RGB_UNORM_BLOCK;
      case TextureFormat::kBC2:
        return VkFormat::VK_FORMAT_BC2_UNORM_BLOCK;
      case TextureFormat::kBC3:
        return VkFormat::VK_FORMAT_BC3_UNORM_BLOCK;
      case TextureFormat::kBC4:
        return VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;
      case TextureFormat::kBC5:
        return VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;
      case TextureFormat::kBC6:
        return VkFormat::VK_FORMAT_BC6H_SFLOAT_BLOCK;
      case TextureFormat::kBC7:
        return VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;
      }

      return VkFormat::VK_FORMAT_UNDEFINED;
    }

#pragma optimize ("", off)
    ///////////////////////////////////////////////////////////////////////////
	VulkanTexture::VulkanTexture(
      asset::VioletTextureHandle texture, 
      VulkanRenderer* renderer) 
      : format_(formatToVulkanFormat(texture->getLayer(0u).getFormat()))
      , texture_(nullptr)
      , srv_(nullptr)
      , size_(0u)
	  , renderer_(renderer)
    {
	  VkResult result;

	  LMB_ASSERT(texture->getLayerCount() > 0,
        "Vulkan TEXTURE: No layers were added to this texture.");

      if (texture->getLayer(0u).getWidth() == 0u || 
        texture->getLayer(0u).getHeight() == 0u)
      {
        LMB_ASSERT(false, "Vulkan TEXTURE: Provided Width and/or Height was 0.");
        return;
      }

	  is_render_target_ = (texture->getLayer(0u).getFlags() &
		  kTextureFlagIsRenderTarget) ? true : false;
	  is_dynamic_ = ((texture->getLayer(0u).getFlags() &
		  kTextureFlagDynamicData) && !is_render_target_) ? true : false;
	  bool from_dds = (texture->getLayer(0u).getFlags() & kTextureFlagFromDDS)
		  ? true : false;
	  bool is_dsv = (format_ == VK_FORMAT_D24_UNORM_S8_UINT || format_ == VK_FORMAT_D32_SFLOAT);

		// TODO (Hilze): Remove the flag VK_IMAGE_USAGE_TRANSFER_SRC_BIT ASAP!
	  VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	  if (is_render_target_)
	  {
		  if (is_dsv)
			  usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		  else
			  usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	  }

		if (!(is_dsv && is_render_target_))
		{
			usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}

	  VezImageCreateInfo image_create_info{};
	  image_create_info.pNext         = nullptr;
	  image_create_info.imageType     = VK_IMAGE_TYPE_2D;
	  image_create_info.format        = format_;
	  image_create_info.extent.width  = texture->getLayer(0u).getWidth();
	  image_create_info.extent.height = texture->getLayer(0u).getHeight();
	  image_create_info.extent.depth  = 1;
	  image_create_info.mipLevels     = texture->getLayer(0u).getMipCount();
	  image_create_info.arrayLayers   = texture->getLayerCount();
	  image_create_info.flags         = (image_create_info.arrayLayers == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : (VkImageCreateFlagBits)0;
	  image_create_info.samples       = VK_SAMPLE_COUNT_1_BIT;
	  image_create_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
	  image_create_info.usage         = usage;

	  // Create the textures.
	  {
			result = vezCreateImage(renderer_->getDevice(), VEZ_MEMORY_GPU_ONLY, &image_create_info, &texture_);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create texture | %s", vkErrorCode(result));

			VkMemoryRequirements memory_requirements{};
			vkGetImageMemoryRequirements(renderer_->getDevice(), texture_, &memory_requirements);
			size_ += (size_t)memory_requirements.size;
	  }

	  // Upload the data.
	  bool contains_data = !texture->getLayer(0u).getData().empty() || from_dds;
	  
		for (uint32_t layer = 0u; layer < image_create_info.arrayLayers && contains_data; ++layer)
		{
			uint32_t w = texture->getLayer(layer).getWidth();
			uint32_t h = texture->getLayer(layer).getHeight();

			Vector<char> dds_data;
			const char* data = texture->getLayer(layer).getData().data();
			if (!data && from_dds)
			{
				dds_data = asset::TextureManager::getInstance()->getData(texture);
				data = dds_data.data();
			}


			uint32_t offset = 0u;
			if (*((uint32_t*)data) == 0x20534444)
			{
				// TODO (Hilze): Read the DDS header and find out
				// if it uses the extended or normal header.

				if (format_ == VK_FORMAT_BC6H_SFLOAT_BLOCK)
					offset = 148u;
				else
					offset = 128u;
			}

			for (uint32_t mip = 0u; mip < image_create_info.mipLevels; ++mip)
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
				VezImageSubDataInfo sub_data_info{};
				sub_data_info.imageSubresource.mipLevel = mip;
				sub_data_info.imageSubresource.baseArrayLayer = layer;
				sub_data_info.imageSubresource.layerCount = 1;
				sub_data_info.imageOffset = { 0, 0, 0 };
				sub_data_info.imageExtent = { w, h, 1 };
				result = vezImageSubData(renderer_->getDevice(), texture_, &sub_data_info, data);
				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not update iamge sub data | %s", vkErrorCode(result));

				offset += bpl;
				w /= 2u;
				h /= 2u;
			}
		}

      // Create shader resource view.
	  createMainViews(image_create_info.arrayLayers, image_create_info.mipLevels);

	  layers_.resize(image_create_info.arrayLayers);
      if (is_render_target_)
          createSubViews(image_create_info.arrayLayers, image_create_info.mipLevels);
    }

    ///////////////////////////////////////////////////////////////////////////
    VulkanTexture::~VulkanTexture()
    {
    if (srv_ != VK_NULL_HANDLE)
        vezDestroyImageView(renderer_->getDevice(), srv_);
		
	for (auto& layer : layers_)
		for (auto view : layer)
			vezDestroyImageView(renderer_->getDevice(), view);
	layers_.clear();

    if (texture_ != nullptr)
        vezDestroyImage(renderer_->getDevice(), texture_);
    }

		///////////////////////////////////////////////////////////////////////////
		void VulkanTexture::update(asset::VioletTextureHandle texture)
		{
			VkResult result;
			for (uint32_t i = 0; i < texture->getLayerCount(); ++i)
			{
				auto& layer = texture->getLayer(i);

				// TODO (Hilze): Find out why DDSses are dynamic.
				if (layer.isDirty() && !(layer.getFlags() & kTextureFlagFromDDS))
				{
					VezImageSubDataInfo sub_data_info{};
					sub_data_info.imageSubresource.mipLevel = 0;
					sub_data_info.imageSubresource.baseArrayLayer = i;
					sub_data_info.imageSubresource.layerCount = 1;
					sub_data_info.imageOffset = { 0, 0, 0 };
					sub_data_info.imageExtent = { layer.getWidth(), layer.getHeight(), 1 };
					result = vezImageSubData(renderer_->getDevice(), texture_, &sub_data_info, layer.getData().data());
					LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not update image data | %s", vkErrorCode(result));

					layer.clean();
				}
			}
		}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanTexture::bind(uint8_t slot)
    {
		LMB_ASSERT(false, "VULKAN: Not implemented");
    }

    ///////////////////////////////////////////////////////////////////////////
    VkFormat VulkanTexture::getFormat() const
    {
      return format_;
    }

    ///////////////////////////////////////////////////////////////////////////
	VkImageView VulkanTexture::getMainView() const
    {
      return srv_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanTexture::generateMips() const
    {
		LMB_ASSERT(false, "VULKAN: Not implemented");
    }

    ///////////////////////////////////////////////////////////////////////////
	VkImageView VulkanTexture::getSubView(
	  unsigned char layer,
	  unsigned char mip_map) const
    {
		LMB_ASSERT(layers_.size() >= layer && layers_[layer].size() >= mip_map, "getSubView(Layer: %i, MipMap: %i)", layer, mip_map);
      return layers_[layer][mip_map];
    }

    ///////////////////////////////////////////////////////////////////////////
    VkImage VulkanTexture::getTexture() const
    {
      LMB_ASSERT(texture_ != nullptr, "getTexture() : Texture was nullptr");
      return texture_;
    }

	///////////////////////////////////////////////////////////////////////////
	size_t VulkanTexture::getGPUSize() const
	{
		return size_;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanTexture::createMainViews(
      unsigned char layer_count,
      unsigned char mip_count)
	{
		VezImageViewCreateInfo image_view_create_info{};
		image_view_create_info.image = texture_;
		image_view_create_info.viewType = layer_count == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		image_view_create_info.format = format_;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.layerCount = layer_count;
		image_view_create_info.subresourceRange.levelCount = mip_count;
			
		VkResult result = vezCreateImageView(renderer_->getDevice(), &image_view_create_info, &srv_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create dsv | %s", vkErrorCode(result));
	}

    ///////////////////////////////////////////////////////////////////////////
	void VulkanTexture::createSubViews(
		unsigned char layer_count,
		unsigned char mip_count)
	{
		VkResult result;

		for (unsigned char l = 0u; l < layer_count; ++l)
		{
			layers_[l].resize(mip_count);
			for (unsigned char m = 0u; m < mip_count; ++m)
			{
				VezImageViewCreateInfo image_view_create_info{};
				image_view_create_info.image = texture_;
				image_view_create_info.viewType = layer_count == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				image_view_create_info.format = format_;
				image_view_create_info.subresourceRange.baseArrayLayer = l;
				image_view_create_info.subresourceRange.baseMipLevel = m;
				image_view_create_info.subresourceRange.layerCount = 1;
				image_view_create_info.subresourceRange.levelCount = 1;
				result = vezCreateImageView(renderer_->getDevice(), &image_view_create_info, &layers_[l][m]);

				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create dsv | %s", vkErrorCode(result));
			}
		}
	}
  }
}