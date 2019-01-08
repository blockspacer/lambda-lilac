#pragma once
#include "shader_pass.h"

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    class RenderPassManager
    {
      void initialize(world::IWorld* world);
      void executePre();
      void executePost();

    private:
      /////////////////////////////////////////////////////////////////////////
      struct PriorityShader
      {
        uint16_t priority : 15u;
        bool     enabled  : 1u;
        asset::ShaderHandle shader;
      };
    
      /////////////////////////////////////////////////////////////////////////
      struct PriorityShaderPass
      {
        uint16_t   priority : 15u;
        bool       enabled  : 1u;
        ShaderPass pass;
      };
      Vector<PriorityShader> pre_passes_;
      Vector<PriorityShaderPass> post_passes_;
      world::IWorld* world_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class PostProcessManager
    {
    public:
      void addTarget(const RenderTarget& target);
      void removeTarget(const RenderTarget& target);
      const RenderTarget& getTarget(const Name& name) const;
      RenderTarget& getTarget(const Name& name);
      void resize(const glm::uvec2& size);
      
      void addPass(const ShaderPass& pass);
      
      void setFinalTarget(const Name& final_target);
      Name getFinalTarget() const;
      
      Vector<ShaderPass>& getPasses();
      const Vector<ShaderPass>& getPasses() const;
      const UnorderedMap<Name, RenderTarget>& getAllTargets() const;

    private:
      glm::uvec2 last_size_ = glm::uvec2(1u);
      Name final_target_;
      UnorderedMap<Name, RenderTarget> targets_;
      Vector<ShaderPass> passes_;
    };
  }
}
