#pragma once
#include "systems/entity.h"
#include "icomponent.h"

namespace lambda
{
  namespace world
  {
    class IWorld;
  }

  namespace components
  {
    ///////////////////////////////////////////////////////////////////////////
    enum class SystemIds : size_t
    {
      kEntitySystem,
      kTransformSystem,
      kCameraSystem,
      kLightSystem,
      kMeshRenderSystem,
      kColliderSystem,
      kMonoBehaviourSystem,
      kRigidBodySystem,
      kSkeletonSystem,
      kLODSystem,
      kWaveSourceSystem,
      kCount
    };

    ///////////////////////////////////////////////////////////////////////////
    class ISystem
    {
    public:
      virtual ~ISystem() { };
      virtual void initialize(world::IWorld& world) = 0;
      virtual void deinitialize() = 0;
      virtual void update(const double& /*delta_time*/) {};
      virtual void fixedUpdate(const double& /*time_step*/) {};
      virtual void onRender() {};
      static size_t systemId() { return 0; };
			virtual String profilerInfo() const { return ""; }

    protected:
      template<typename T>
      void require(T* t, const entity::Entity& e)
      {
        if (t->hasComponent(e) == false)
        {
          t->addComponent(e);
        }
      }
    };
  }
}
