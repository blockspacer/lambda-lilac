#pragma once
#include "interfaces/irenderer.h"

namespace lambda
{
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    class NoRenderer : public platform::IRenderer
    {
    public:
      virtual ~NoRenderer();
      virtual void setWindow(
        foundation::SharedPointer<platform::IWindow> window
      ) override;
      virtual void initialize(world::IWorld* world) override;
      virtual void deinitialize() override;
      virtual void resize() override;
      virtual void update(const double& delta_time) override;
      virtual void startFrame() override;
      virtual void endFrame(bool display) override;

      /////////////////////////////////////////////////////////////////////////
      ///// Deferred Calls ////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////
      virtual void draw() override;
      virtual void drawInstanced(const Vector<glm::mat4>& matrices) override;

      virtual void setRasterizerState(
        const platform::RasterizerState& rasterizer_state
      ) override;
      virtual void setBlendState(
        const platform::BlendState& blend_state
      ) override;
      virtual void setDepthStencilState(
        const platform::DepthStencilState& depth_stencil_state
      ) override;
      virtual void setSamplerState(
        const platform::SamplerState& sampler_state, 
        unsigned char slot
      ) override;

      virtual void generateMipMaps(
        const asset::VioletTextureHandle& texture
      ) override;
      virtual void copyToScreen(
        const asset::VioletTextureHandle& texture
      ) override;
      virtual void bindShaderPass(
        const platform::ShaderPass& shader_pass
      ) override;
      virtual void clearRenderTarget(
        asset::VioletTextureHandle texture, 
        const glm::vec4& colour
      ) override;

      virtual void setScissorRect(const glm::vec4& rect) override;
      virtual void setViewports(const Vector<glm::vec4>& rects);

      virtual void setMesh(asset::MeshHandle mesh) override;
      virtual void setSubMesh(const uint32_t& sub_mesh_idx) override;
      virtual void setShader(asset::ShaderHandle shader) override;
      virtual void setTexture(
        asset::VioletTextureHandle texture, 
        uint8_t slot
      ) override;

      virtual void pushMarker(const String& name) override;
      virtual void setMarker(const String& name) override;
      virtual void popMarker() override;

      virtual void  beginTimer(const String& name) override;
      virtual void  endTimer(const String& name) override;
      virtual uint64_t getTimerMicroSeconds(const String& name) override;
      /////////////////////////////////////////////////////////////////////////
      ///// Deferred Calls End ////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      virtual void setRenderScale(const float& render_scale) override;
      virtual float getRenderScale() override;

      virtual void setVSync(bool vsync) override;
      virtual bool getVSync() const override;

      void setShaderVariable(
        const platform::ShaderVariable& variable
      ) override;

    protected:
      virtual void destroyAsset(
        foundation::SharedPointer<asset::IAsset> asset
      ) override;
    };
  }
}