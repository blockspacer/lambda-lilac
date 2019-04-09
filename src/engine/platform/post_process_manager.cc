#include "post_process_manager.h"
#include <interfaces/iworld.h>

namespace lambda
{
  namespace platform
  {
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void PostProcessManager::operator=(const PostProcessManager& other)
		{
			last_size_    = other.last_size_;
			final_target_ = other.final_target_;
			targets_      = other.targets_;
			passes_       = other.passes_;
		}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void PostProcessManager::addTarget(const RenderTarget& target)
    {
      target.getTexture()->getLayer(0u).setFlags(target.getTexture()->getLayer(0u).getFlags() | kTextureFlagIsRenderTarget);
      targets_.insert(eastl::make_pair(target.getName(), target));
      targets_[target.getName()].resize(last_size_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void PostProcessManager::removeTarget(const RenderTarget& target)
    {
      targets_.erase(target.getName());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void PostProcessManager::resize(const glm::uvec2& size)
    {
			last_size_ = size;
			for (auto& target : targets_)
        target.second.resize(size);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const RenderTarget& PostProcessManager::getTarget(const Name& name) const
    {
     LMB_ASSERT(targets_.find(name) != targets_.end(), "RENDER TARGET: Could not find target with name: %s", name.getName().c_str());
     return targets_.at(name);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RenderTarget& PostProcessManager::getTarget(const Name& name)
    {
      LMB_ASSERT(targets_.find(name) != targets_.end(), "RENDER TARGET: Could not find target with name: %s", name.getName().c_str());
      return targets_.at(name);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void PostProcessManager::addPass(const ShaderPass& pass)
    {
      passes_.push_back(pass);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void PostProcessManager::setFinalTarget(const Name& final_target)
    {
      final_target_ = final_target;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Name PostProcessManager::getFinalTarget() const
    {
      return final_target_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Vector<ShaderPass>& PostProcessManager::getPasses()
    {
      return passes_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const Vector<ShaderPass>& PostProcessManager::getPasses() const
    {
      return passes_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const UnorderedMap<Name, RenderTarget>& PostProcessManager::getAllTargets() const
    {
      return targets_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ShaderPass::ShaderPass() :
      enabled_(false)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ShaderPass::ShaderPass(Name name,
      asset::VioletShaderHandle shader,
      Vector<RenderTarget> inputs,
      Vector<RenderTarget> outputs)
      : enabled_(true)
      , name_(name)
      , shader_(shader)
      , inputs_(inputs)
      , outputs_(outputs)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    asset::VioletShaderHandle ShaderPass::getShader() const
    {
      return shader_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const Vector<RenderTarget>& ShaderPass::getInputs() const
    {
      return inputs_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const Vector<RenderTarget>& ShaderPass::getOutputs() const
    {
      return outputs_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Name ShaderPass::getName() const
    {
      return name_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ShaderPass::setEnabled(const bool& enabled)
    {
      enabled_ = enabled;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool ShaderPass::getEnabled() const
    {
      return enabled_;
    }










    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RenderTarget::RenderTarget()
      : render_scale_(1.0f)
      , name_("back_buffer")
      , mip_map_(0)
      , layer_(0)
      , is_back_buffer_(true)
      , from_texture_(false)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RenderTarget::RenderTarget(const RenderTarget& other)
      : render_scale_(other.render_scale_)
      , texture_(other.texture_)
      , name_(other.name_)
      , mip_map_(other.mip_map_)
      , layer_(other.layer_)
      , is_back_buffer_(other.is_back_buffer_)
      , from_texture_(other.from_texture_)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RenderTarget::RenderTarget(const Name& name, const float& render_scale, const TextureFormat& format)
      : render_scale_(render_scale)
      , texture_(asset::TextureManager::getInstance()->create(name, 1u, 1u, 1u, format, kTextureFlagIsRenderTarget | kTextureFlagResize | kTextureFlagClear | kTextureFlagDynamicScale /*// TODO (Hilze): Remove!*/))
      , name_(name)
      , mip_map_(0)
      , layer_(0)
      , is_back_buffer_(false)
      , from_texture_(false)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RenderTarget::RenderTarget(const Name& name, asset::VioletTextureHandle texture)
      : render_scale_(1.0f)
      , texture_(texture)
      , name_(name)
      , mip_map_(0)
      , layer_(0)
      , is_back_buffer_(false)
      , from_texture_(true)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RenderTarget::RenderTarget(const Name& name, const float& render_scale, asset::VioletTextureHandle texture, const bool& from_texture)
      : render_scale_(render_scale)
      , texture_(texture)
      , name_(name)
      , mip_map_(0)
      , layer_(0)
      , is_back_buffer_(false)
      , from_texture_(from_texture)
    {
      if (!from_texture_)
      {
        if (!(texture->getLayer(0u).getFlags() & kTextureFlagIsRenderTarget))
          texture->getLayer(0u).setFlags(texture->getLayer(0u).getFlags() | kTextureFlagIsRenderTarget);
        if (!(texture->getLayer(0u).getFlags() & kTextureFlagResize))
          texture->getLayer(0u).setFlags(texture->getLayer(0u).getFlags() | kTextureFlagResize);
        if (!(texture->getLayer(0u).getFlags() & kTextureFlagClear))
          texture->getLayer(0u).setFlags(texture->getLayer(0u).getFlags() | kTextureFlagClear);
        if (!(texture->getLayer(0u).getFlags() & kTextureFlagDynamicScale))
          texture->getLayer(0u).setFlags(texture->getLayer(0u).getFlags() | kTextureFlagDynamicScale);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void RenderTarget::resize(const glm::uvec2& size)
    {
			if ((texture_->getLayer(0u).getFlags() & kTextureFlagResize) != 0)
				texture_->resize((uint32_t)((float)size.x * render_scale_), (uint32_t)((float)size.y * render_scale_));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool RenderTarget::isBackBuffer() const
    {
      return is_back_buffer_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    asset::VioletTextureHandle RenderTarget::getTexture() const
    {
      return texture_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Name RenderTarget::getName() const
    {
      return name_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float RenderTarget::getRenderScale() const
    {
      return render_scale_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool RenderTarget::fromTexture() const
    {
      return from_texture_;
    }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int RenderTarget::getMipMap() const
    {
      return mip_map_;
    }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void RenderTarget::setMipMap(int mip_map)
    {
      mip_map_ = mip_map;
    }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int RenderTarget::getLayer() const
	{
		return layer_;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void RenderTarget::setLayer(int layer)
	{
		layer_ = layer;
	}
  }
}
