#include "d3d11_renderer.h"
#include "d3d11_context.h"
#include <memory/frame_heap.h>

namespace lambda
{
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    D3D11Renderer::~D3D11Renderer()
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::initialize(world::IWorld* world)
    {
      context_ = foundation::Memory::construct<D3D11Context>();
      context_->initialize(world);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::deinitialize()
    {
      context_->deinitialize();
      foundation::Memory::destruct(context_);
      context_ = nullptr;
    }






    






#if FLUSH_METHOD
    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetImGUI : public IRenderAction
    {
      RenderActionSetImGUI() :
        imgui(nullptr) {};
      ~RenderActionSetImGUI() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setImGUI(imgui);
      }
      foundation::SharedPointer<platform::IImGUI> imgui;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetWindow : public IRenderAction
    {
      RenderActionSetWindow() :
        window(nullptr) {};
      ~RenderActionSetWindow() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setWindow(window);
      }
      foundation::SharedPointer<platform::IWindow> window;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetShaderVariableManager : public IRenderAction
    {
      RenderActionSetShaderVariableManager() :
        shader_variable_manager(nullptr) {};
      ~RenderActionSetShaderVariableManager() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setShaderVariableManager(*shader_variable_manager);
      }
      platform::ShaderVariableManager* shader_variable_manager;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionResize : public IRenderAction
    {
      RenderActionResize() {};
      ~RenderActionResize() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->resize();
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionUpdate : public IRenderAction
    {
      RenderActionUpdate() :
        delta_time(0.0) {};
      ~RenderActionUpdate() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->update(delta_time);
      }
      double delta_time;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionStartFrame : public IRenderAction
    {
      RenderActionStartFrame() {};
      ~RenderActionStartFrame() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->startFrame();
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionEndFrame : public IRenderAction
    {
      RenderActionEndFrame() :
        display(false) {};
      ~RenderActionEndFrame() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->endFrame(display);
      }
      bool display;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionDraw : public IRenderAction
    {
      RenderActionDraw() {};
      ~RenderActionDraw() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->draw();
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionDrawInstanced : public IRenderAction
    {
      RenderActionDrawInstanced() :
        matrices() {};
      ~RenderActionDrawInstanced() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->drawInstanced(matrices);
      }
      Vector<glm::mat4> matrices;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetRasterizerState : public IRenderAction
    {
      RenderActionSetRasterizerState() :
        rasterizer_state() {};
      ~RenderActionSetRasterizerState() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setRasterizerState(rasterizer_state);
      }
      platform::RasterizerState rasterizer_state;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetBlendState : public IRenderAction
    {
      RenderActionSetBlendState() :
        blend_state() {};
      ~RenderActionSetBlendState() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setBlendState(blend_state);
      }
      platform::BlendState blend_state;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetDepthStencilState : public IRenderAction
    {
      RenderActionSetDepthStencilState() :
        depth_stencil_state() {};
      ~RenderActionSetDepthStencilState() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setDepthStencilState(depth_stencil_state);
      }
      platform::DepthStencilState depth_stencil_state;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetSamplerState : public IRenderAction
    {
      RenderActionSetSamplerState() :
        sampler_state(), slot(0u) {};
      ~RenderActionSetSamplerState() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setSamplerState(sampler_state, slot);
      }
      platform::SamplerState sampler_state;
      uint8_t slot;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionGenerateMipMaps : public IRenderAction
    {
      RenderActionGenerateMipMaps() :
        texture() {};
      ~RenderActionGenerateMipMaps() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->generateMipMaps(texture);
      }
      asset::VioletTextureHandle texture;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionCopyToScreen : public IRenderAction
    {
      RenderActionCopyToScreen() :
        texture() {};
      ~RenderActionCopyToScreen() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->copyToScreen(texture);
      }
      asset::VioletTextureHandle texture;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionBindShaderPass : public IRenderAction
    {
      RenderActionBindShaderPass() :
      shader_pass() {};
      ~RenderActionBindShaderPass() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->bindShaderPass(shader_pass);
      }
      platform::ShaderPass shader_pass;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionClearRenderTarget : public IRenderAction
    {
      RenderActionClearRenderTarget() :
        texture(), colour() {};
      ~RenderActionClearRenderTarget() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->clearRenderTarget(texture, colour);
      }
      asset::VioletTextureHandle texture;
      glm::vec4 colour;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetScissorRect : public IRenderAction
    {
      RenderActionSetScissorRect() :
      rect() {};
      ~RenderActionSetScissorRect() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setScissorRect(rect);
      }
      glm::vec4 rect;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetViewPorts : public IRenderAction
    {
      RenderActionSetViewPorts() :
      rects() {};
      ~RenderActionSetViewPorts() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setViewports(rects);
      }
      Vector<glm::vec4> rects;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetMesh : public IRenderAction
    {
      RenderActionSetMesh() :
      mesh() {};
      ~RenderActionSetMesh() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setMesh(mesh);
      }
      asset::MeshHandle mesh;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetSubMesh : public IRenderAction
    {
      RenderActionSetSubMesh() :
      sub_mesh_idx(0u) {};
      ~RenderActionSetSubMesh() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setSubMesh(sub_mesh_idx);
      }
      uint32_t sub_mesh_idx;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetShader : public IRenderAction
    {
      RenderActionSetShader() :
      shader() {};
      ~RenderActionSetShader() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setShader(shader);
      }
      asset::VioletShaderHandle shader;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetTexture : public IRenderAction
    {
      RenderActionSetTexture() :
        texture(), slot(0u) {};
      ~RenderActionSetTexture() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setTexture(texture, slot);
      }
      asset::VioletTextureHandle texture;
      uint8_t slot;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionPushMarker : public IRenderAction
    {
      RenderActionPushMarker() :
        name() {};
      ~RenderActionPushMarker() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->pushMarker(name);
      }
      String name;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetMarker : public IRenderAction
    {
      RenderActionSetMarker() :
        name() {};
      ~RenderActionSetMarker() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setMarker(name);
      }
      String name;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionPopMarker : public IRenderAction
    {
      RenderActionPopMarker() {};
      ~RenderActionPopMarker() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->popMarker();
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    struct RenderActionSetShaderVariable : public IRenderAction
    {
      RenderActionSetShaderVariable() :
      variable() {};
      ~RenderActionSetShaderVariable() {};
      virtual void execute(D3D11Context* context) const override
      {
        context->setShaderVariable(variable);
      }
      platform::ShaderVariable variable;
    };

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setWindow(
      foundation::SharedPointer<platform::IWindow> window)
    {
      RenderActionSetWindow* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetWindow>();
      action->window = window;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::resize()
    {
      RenderActionResize* action = 
        foundation::GetFrameHeap()->construct<RenderActionResize>();

      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::update(const double& delta_time)
    {
      RenderActionUpdate* action = 
        foundation::GetFrameHeap()->construct<RenderActionUpdate>();
      action->delta_time = delta_time;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::startFrame()
    {
      RenderActionStartFrame* action = 
        foundation::GetFrameHeap()->construct<RenderActionStartFrame>();
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::endFrame(bool display)
    {
      RenderActionEndFrame* action = 
        foundation::GetFrameHeap()->construct<RenderActionEndFrame>();
      action->display = display;
      queue_actions_.push_back(action);

      foundation::GetFrameHeap()->update();
      context_->flush(eastl::move(queue_actions_));
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::draw()
    {
      RenderActionDraw* action = 
        foundation::GetFrameHeap()->construct<RenderActionDraw>();
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::drawInstanced(const Vector<glm::mat4>& matrices)
    {
      RenderActionDrawInstanced* action = 
        foundation::GetFrameHeap()->construct<RenderActionDrawInstanced>();
      action->matrices = matrices;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setRasterizerState(
      const platform::RasterizerState & rasterizer_state)
    {
      RenderActionSetRasterizerState* action = 
       foundation::GetFrameHeap()->construct<RenderActionSetRasterizerState>();
      action->rasterizer_state = rasterizer_state;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setBlendState(const platform::BlendState & blend_state)
    {
      RenderActionSetBlendState* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetBlendState>();
      action->blend_state = blend_state;
      queue_actions_.push_back(action);
    }
 
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
      RenderActionSetDepthStencilState* action = 
     foundation::GetFrameHeap()->construct<RenderActionSetDepthStencilState>();
      action->depth_stencil_state = depth_stencil_state;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
      RenderActionSetSamplerState* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetSamplerState>();
      action->sampler_state = sampler_state;
      action->slot = slot;
      queue_actions_.push_back(action);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::generateMipMaps(
      const asset::VioletTextureHandle & texture)
    {
      RenderActionGenerateMipMaps* action = 
        foundation::GetFrameHeap()->construct<RenderActionGenerateMipMaps>();
      action->texture = texture;
      queue_actions_.push_back(action);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::copyToScreen(const asset::VioletTextureHandle& texture)
    {
      RenderActionCopyToScreen* action = 
        foundation::GetFrameHeap()->construct<RenderActionCopyToScreen>();
      action->texture = texture;
      queue_actions_.push_back(action);
    }
    
		///////////////////////////////////////////////////////////////////////////
		void D3D11Renderer::copyToTexture(const asset::VioletTextureHandle& src,
			const asset::VioletTextureHandle& dst)
		{
			LMB_ASSERT(false, "TODO (Hilze): Implement");
		}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
      RenderActionBindShaderPass* action = 
        foundation::GetFrameHeap()->construct<RenderActionBindShaderPass>();
      action->shader_pass = shader_pass;
      queue_actions_.push_back(action);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::clearRenderTarget(
      asset::VioletTextureHandle texture, 
      const glm::vec4& colour)
    {
      RenderActionClearRenderTarget* action = 
        foundation::GetFrameHeap()->construct<RenderActionClearRenderTarget>();
      action->texture = texture;
      action->colour = colour;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setScissorRect(const glm::vec4 & rect)
    {
      RenderActionSetScissorRect* action =
        foundation::GetFrameHeap()->construct<RenderActionSetScissorRect>();
      action->rect = rect;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setViewports(const Vector<glm::vec4>& rects)
    {
      RenderActionSetViewPorts* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetViewPorts>();
      action->rects = rects;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setMesh(asset::MeshHandle mesh)
    {
      RenderActionSetMesh* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetMesh>();
      action->mesh = mesh;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setSubMesh(const uint32_t & sub_mesh_idx)
    {
      RenderActionSetSubMesh* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetSubMesh>();
      action->sub_mesh_idx = sub_mesh_idx;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setShader(asset::VioletShaderHandle shader)
    {
      RenderActionSetShader* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetShader>();
      action->shader = shader;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setTexture(
      asset::VioletTextureHandle texture, 
      uint8_t slot)
    {
      RenderActionSetTexture* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetTexture>();
      action->texture = texture;
      action->slot = slot;
      queue_actions_.push_back(action);
    }

		///////////////////////////////////////////////////////////////////////////
		void D3D11Renderer::setRenderTargets(
			Vector<asset::VioletTextureHandle> render_targets, 
			asset::VioletTextureHandle depth_buffer)
		{
			LMB_ASSERT(false, "TODO (Hilze): Implement");
		}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::pushMarker(const String& name)
    {
      RenderActionPushMarker* action = 
        foundation::GetFrameHeap()->construct<RenderActionPushMarker>();
      action->name = name;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setMarker(const String& name)
    {
      RenderActionSetMarker* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetMarker>();
      action->name = name;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::popMarker()
    {
      RenderActionPopMarker* action = 
        foundation::GetFrameHeap()->construct<RenderActionPopMarker>();
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::beginTimer(const String& name)
    {
      LMB_ASSERT(false, "Timers are not supported in deferred mode!");
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::endTimer(const String& name)
    {
      LMB_ASSERT(false, "Timers are not supported in deferred mode!");
    }

    ///////////////////////////////////////////////////////////////////////////
    uint64_t D3D11Renderer::getTimerMicroSeconds(const String& name)
    {
      LMB_ASSERT(false, "Timers are not supported in deferred mode!");
      return 0u;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setShaderVariable(
      const platform::ShaderVariable& variable)
    {
      RenderActionSetShaderVariable* action = 
        foundation::GetFrameHeap()->construct<RenderActionSetShaderVariable>();
      action->variable = variable;
      queue_actions_.push_back(action);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setRenderScale(const float & render_scale)
    {
      // STUB
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::destroyAsset(
      foundation::SharedPointer<asset::IAsset> asset)
    {
      // STUB
    }
#else

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setWindow(
      foundation::SharedPointer<platform::IWindow> window)
    {
      context_->setWindow(window);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::resize()
    {
      context_->resize();
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::update(const double& delta_time)
    {
      context_->update(delta_time);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::startFrame()
    {
      context_->startFrame();
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::endFrame(bool display)
    {
      foundation::GetFrameHeap()->update();
      context_->endFrame(display);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::draw()
    {
      context_->draw();
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::drawInstanced(const Vector<glm::mat4>& matrices)
    {
      context_->drawInstanced(matrices);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
      context_->setRasterizerState(rasterizer_state);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setBlendState(const platform::BlendState& blend_state)
    {
      context_->setBlendState(blend_state);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
      context_->setDepthStencilState(depth_stencil_state);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
      context_->setSamplerState(sampler_state, slot);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::generateMipMaps(
      const asset::VioletTextureHandle& texture)
    {
      context_->generateMipMaps(texture);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::copyToScreen(const asset::VioletTextureHandle& texture)
    {
      context_->copyToScreen(texture);
    }

		///////////////////////////////////////////////////////////////////////////
		void D3D11Renderer::copyToTexture(const asset::VioletTextureHandle& src,
			const asset::VioletTextureHandle& dst)
		{
			context_->copyToTexture(src, dst);
		}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
      context_->bindShaderPass(shader_pass);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::clearRenderTarget(
      asset::VioletTextureHandle texture, 
      const glm::vec4& colour)
    {
      context_->clearRenderTarget(texture, colour);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setScissorRect(const glm::vec4& rect)
    {
      context_->setScissorRect(rect);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setViewports(const Vector<glm::vec4>& rects)
    {
      context_->setViewports(rects);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setMesh(asset::MeshHandle mesh)
    {
      context_->setMesh(mesh);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setSubMesh(const uint32_t& sub_mesh_idx)
    {
      context_->setSubMesh(sub_mesh_idx);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setShader(asset::VioletShaderHandle shader)
    {
      context_->setShader(shader);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setTexture(
      asset::VioletTextureHandle texture, 
      uint8_t slot)
    {
      context_->setTexture(texture, slot);
    }

		///////////////////////////////////////////////////////////////////////////
		void D3D11Renderer::setRenderTargets(
			Vector<asset::VioletTextureHandle> render_targets,
			asset::VioletTextureHandle depth_buffer)
		{
			context_->setRenderTargets(render_targets, depth_buffer);
		}


    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::pushMarker(const String& name)
    {
      context_->pushMarker(name);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setMarker(const String& name)
    {
      context_->setMarker(name);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::popMarker()
    {
      context_->popMarker();
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::beginTimer(const String& name)
    {
      context_->beginTimer(name);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::endTimer(const String& name)
    {
      context_->endTimer(name);
    }

    ///////////////////////////////////////////////////////////////////////////
    uint64_t D3D11Renderer::getTimerMicroSeconds(const String& name)
    {
      return context_->getTimerMicroSeconds(name);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setShaderVariable(
      const platform::ShaderVariable & variable)
    {
      context_->setShaderVariable(variable);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setRenderScale(const float & render_scale)
    {
      context_->setRenderScale(render_scale);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::destroyAsset(
      foundation::SharedPointer<asset::IAsset> asset)
    {
      context_->destroyAsset(asset);
    }
#endif



















    ///////////////////////////////////////////////////////////////////////////
    void D3D11Renderer::setVSync(bool vsync)
    {
      context_->setVSync(vsync);
    }
   
    ///////////////////////////////////////////////////////////////////////////
    float D3D11Renderer::getRenderScale()
    {
      return context_->getRenderScale();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    bool D3D11Renderer::getVSync() const
    {
      return context_->getVSync();
    }
  }
}