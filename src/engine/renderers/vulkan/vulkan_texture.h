#pragma once
#include "assets/texture.h"
#include "vulkan.h"

namespace lambda
{
  namespace linux
  {
    class VulkanRenderer;

    ///////////////////////////////////////////////////////////////////////////
    class VulkanTexture
    {
      friend class VulkanRenderTargetView;
      friend class VulkanDepthStencilView;
      friend class VulkanRenderTexture;
      friend class VulkanContext;

    public:
      VulkanTexture(
        asset::VioletTextureHandle texture, 
        VulkanRenderer* renderer
      );
      ~VulkanTexture();
			void update(asset::VioletTextureHandle texture);
      void bind(uint8_t slot);
      VkFormat getFormat() const;
      VkImageView getMainView() const;
      void generateMips() const;
      VkImageView getSubView(
        unsigned char layer,
        unsigned char mip_map
      ) const;

      VkImage getTexture() const;
	  size_t getGPUSize() const;

    private:
	  void createMainViews(
        unsigned char layer_count,
        unsigned char mip_count
      );
      void createSubViews(
        unsigned char layer_count,
        unsigned char mip_count
      );

    private:
	  VulkanRenderer* renderer_;
	  VkFormat format_;
	  VkImageView srv_;
	  VkImage texture_;
	  Vector<Vector<VkImageView>> layers_;
	  
	  bool is_render_target_;
	  bool is_dynamic_;
	  size_t size_;
    };
  }
}