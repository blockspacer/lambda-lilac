#include "scene.h"
#include "systems/entity_system.h"
#include "systems/transform_system.h"
#include "systems/camera_system.h"
#include "systems/mesh_render_system.h"
#include "systems/light_system.h"
#include "systems/wave_source_system.h"
#include "systems/rigid_body_system.h"
#include "systems/mono_behaviour_system.h"
#include "systems/collider_system.h"
#include "systems/skeleton_system.h"
#include "systems/lod_system.h"

namespace lambda
{
  namespace world
  {
		///////////////////////////////////////////////////////////////////////////
		Scene::Scene()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Scene::~Scene()
		{
			for (foundation::SharedPointer<components::ISystem>& system : systems_)
				system->deinitialize();

			systems_.resize(0u);
		}

		///////////////////////////////////////////////////////////////////////////
    void Scene::initialize(IWorld* world)
    {
      systems_.resize((size_t)components::SystemIds::kCount);
      systems_.at((size_t)components::SystemIds::kEntitySystem)        = 
        foundation::Memory::constructShared<entity::EntitySystem>();
      systems_.at((size_t)components::SystemIds::kTransformSystem)     = 
        foundation::Memory::constructShared<components::TransformSystem>();
      systems_.at((size_t)components::SystemIds::kCameraSystem)        = 
        foundation::Memory::constructShared<components::CameraSystem>();
      systems_.at((size_t)components::SystemIds::kLightSystem)         =
        foundation::Memory::constructShared<components::LightSystem>();
      systems_.at((size_t)components::SystemIds::kMeshRenderSystem)    = 
        foundation::Memory::constructShared<components::MeshRenderSystem>();
      systems_.at((size_t)components::SystemIds::kColliderSystem)      = 
        foundation::Memory::constructShared<components::ColliderSystem>();
      systems_.at((size_t)components::SystemIds::kRigidBodySystem)     = 
        foundation::Memory::constructShared<components::RigidBodySystem>();
      systems_.at((size_t)components::SystemIds::kMonoBehaviourSystem) = 
        foundation::Memory::constructShared<components::MonoBehaviourSystem>();
      systems_.at((size_t)components::SystemIds::kSkeletonSystem)      = 
        foundation::Memory::constructShared<components::SkeletonSystem>();
      systems_.at((size_t)components::SystemIds::kWaveSourceSystem)    = 
        foundation::Memory::constructShared<components::WaveSourceSystem>();
      systems_.at((size_t)components::SystemIds::kLODSystem)           =
        foundation::Memory::constructShared<components::LODSystem>();

      for (foundation::SharedPointer<components::ISystem> system : systems_)
        system->initialize(*world);
    }

    ///////////////////////////////////////////////////////////////////////////
    const Vector<foundation::SharedPointer<components::ISystem>>&
      Scene::getAllSystems() const
    {
      return systems_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<foundation::SharedPointer<components::ISystem>>&
      Scene::getAllSystems()
    {
      return systems_;
    }
  }
}
