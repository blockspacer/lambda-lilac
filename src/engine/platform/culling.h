#pragma once
#include "systems/mesh_render_system.h"

namespace lambda
{
  namespace utilities
  {
    class Frustum;
	class ZoneManager;
	class BVH;
    
    ///////////////////////////////////////////////////////////////////////////
    struct LinkedNode
    {
      LinkedNode* previous = nullptr;
      LinkedNode* next = nullptr;
      entity::Entity entity = entity::InvalidEntity;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    class Culler
    {
    public:
      Culler();
      ~Culler();
      void setShouldCull(const bool& should_cull);
      void setCullShadowCasters(const bool& cull_shadow_casters);
      void setCullFrequency(const uint8_t& cull_frequency);
	  void cullDynamics(const BaseBVH& bvh, const Frustum& frustum);
	  void cullStatics(const BaseBVH& bvh, const Frustum& frustum);
      LinkedNode getDynamics() const { return dynamic_; }
      LinkedNode getStatics()  const { return static_; }

    private:
      LinkedNode dynamic_;
      LinkedNode static_;
      //uint8_t frames_since_cull_   = UINT8_MAX;
      uint8_t cull_frequency_      = 1u;
      bool    cull_                = true;
      bool    cull_shadow_casters_ = true;
    };
  }
}
