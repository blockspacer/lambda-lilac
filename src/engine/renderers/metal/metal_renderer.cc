#include "renderers/metal/metal_renderer.h"
#include "renderers/metal/metal_cpp.h"
#include "interfaces/iworld.h"

namespace lambda
{
  namespace osx
  {
    ///////////////////////////////////////////////////////////////////////////
    MetalRenderer::~MetalRenderer()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::initialize(world::IWorld* world)
    {
      world_ = world;
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::deinitialize()
    {
      world_ = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setWindow(
      foundation::SharedPointer<platform::IWindow> window)
    {
      metal::createDeviceAndSwapchain(
        window->getWindow(),
        device_,
        swapchain_);

      metal::createCommandQueue(
        device_,
        queue_);

      metal::createPipelineState(
        device_,
        pipeline_state_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::resize()
    {
      metal::resizeSwapchain(
        swapchain_,
        world_->getWindow()->getSize());
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::update(const double& delta_time)
    {
      metal::draw(
        device_,
        queue_,
        swapchain_,
        pipeline_state_,
        world_->getWindow()->getSize());
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::startFrame()
    {
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::endFrame(bool display)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::draw()
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::drawInstanced(const Vector<glm::mat4>& matrices)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setBlendState(const platform::BlendState& blend_state)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::generateMipMaps(const asset::VioletTextureHandle& texture)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::copyToScreen(const asset::VioletTextureHandle& texture)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::clearRenderTarget(
      asset::VioletTextureHandle texture, 
      const glm::vec4& colour)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setScissorRect(const glm::vec4& rect)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setViewports(const Vector<glm::vec4>& rects)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setMesh(asset::MeshHandle mesh)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setSubMesh(const uint32_t& sub_mesh_idx)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setShader(asset::ShaderHandle shader)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setTexture(
      asset::VioletTextureHandle texture, 
      uint8_t slot)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::pushMarker(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setMarker(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::popMarker()
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::beginTimer(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::endTimer(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    uint64_t MetalRenderer::getTimerMicroSeconds(const String& name)
    {
      return 0ul;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setShaderVariable(
      const platform::ShaderVariable& variable)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setRenderScale(const float & render_scale)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::destroyAsset(
      foundation::SharedPointer<asset::IAsset> asset)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void MetalRenderer::setVSync(bool vsync)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    float MetalRenderer::getRenderScale()
    {
      return 1.0f;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool MetalRenderer::getVSync() const
    {
      return false;
    }
  }
}
