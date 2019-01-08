#pragma once
#include <glm/glm.hpp>
#include "assets/texture.h"

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    class RenderTarget
    {
      friend class PostProcessManager;
    public:
      RenderTarget();
      RenderTarget(const RenderTarget& other);
      RenderTarget(
        const Name& name, 
        const float& render_scale, 
        const TextureFormat& format
      );
      RenderTarget(const Name& name, asset::VioletTextureHandle texture);
      RenderTarget(
        const Name& name, 
        const float& render_scale, 
        asset::VioletTextureHandle texture, 
        const bool& from_texture
      );

    private:
      void resize(const glm::uvec2& size);

    public:
      bool isBackBuffer() const;
      asset::VioletTextureHandle getTexture() const;
      Name getName() const;
      float getRenderScale() const;
      bool fromTexture() const;
      int getMipMap() const;
      void setMipMap(int mip_map);

    private:
      float render_scale_;
      asset::VioletTextureHandle texture_;
      Name name_;
      int mip_map_;

      // flags.
      bool is_back_buffer_;
      bool from_texture_;
    };
  }
}
