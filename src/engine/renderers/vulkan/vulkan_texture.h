#pragma once
#include "assets/texture.h"
#include <vulkan/vulkan.hpp>

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
        unsigned char idx,
        unsigned char layer,
        unsigned char mip_map
      ) const;

      void pingPong();
      unsigned char pingPongIdx() const;

    protected:
      VkImage getTexture(unsigned char idx) const;
    
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
	  VkImageView srvs_[2];
	  VkImage textures_[2];
	  Vector<Vector<VkImageView>> layers_[2];
	  
	  unsigned char texture_index_;
	  bool is_render_target_;
	  bool is_dynamic_;
    };
  }
}