#pragma once
#include "interfaces/isystem.h"
#include <containers/containers.h>
#include <memory/memory.h>
#include <systems/entity_system.h>
#include <systems/name_system.h>
#include <systems/lod_system.h>
#include <systems/rigid_body_system.h>
#include <systems/collider_system.h>
#include <systems/mono_behaviour_system.h>
#include <systems/wave_source_system.h>
#include <systems/camera_system.h>
#include <systems/transform_system.h>
#include <systems/mesh_render_system.h>

namespace lambda
{
	namespace world
	{
		struct SceneData
		{
			components::EntitySystem::SystemData        entity;
			components::NameSystem::SystemData          name;
			components::LODSystem::SystemData           lod;
			components::CameraSystem::SystemData        camera;
			components::RigidBodySystem::SystemData     rigid_body;
			components::ColliderSystem::SystemData      collider;
			components::MonoBehaviourSystem::SystemData mono_behaviour;
			components::WaveSourceSystem::SystemData    wave_source;
			components::TransformSystem::SystemData     transform;
			components::MeshRenderSystem::SystemData    mesh_render;
			platform::IRenderer* renderer;
			platform::IWindow*   window;
			world::IWorld* world;
		};

		///////////////////////////////////////////////////////////////////////////
		class Scene
		{
		public:
			Scene();
			~Scene();

			void initialize(IWorld* world);

			template<typename T>
			foundation::SharedPointer<T> getSystem()
			{
				return eastl::static_shared_pointer_cast<T>(
					systems_.at(T::systemId())
					);
			}

			const Vector<foundation::SharedPointer<components::ISystem>>&
				getAllSystems() const;

			Vector<foundation::SharedPointer<components::ISystem>>& getAllSystems();

			SceneData& getSceneData();


		private:
			SceneData* scene_data_;
			Vector<foundation::SharedPointer<components::ISystem>> systems_;
		};
	}

	namespace scene
	{
		void initialize(world::SceneData& scene);
		void update(const float& delta_time, world::SceneData& scene);
		void fixedUpdate(const float& delta_time, world::SceneData& scene);
		void onRender(world::SceneData& scene);
		void collectGarbage(world::SceneData& scene);
		void deinitialize(world::SceneData& scene);
	}
}