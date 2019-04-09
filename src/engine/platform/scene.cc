#include "scene.h"
#include "systems/light_system.h"
#include "systems/wave_source_system.h"

#include "platform/depth_stencil_state.h"

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
			foundation::Memory::destruct(scene_data_);

			for (foundation::SharedPointer<components::ISystem>& system : systems_)
				system->deinitialize();

			systems_.resize(0u);
		}

		///////////////////////////////////////////////////////////////////////////
		void Scene::initialize(IWorld* world)
		{
			systems_.resize((size_t)components::SystemIds::kCount);
			systems_.at((size_t)components::SystemIds::kLightSystem) =
				foundation::Memory::constructShared<components::LightSystem>();

			for (foundation::SharedPointer<components::ISystem> system : systems_)
				system->initialize(*world);

			scene_data_ = foundation::Memory::construct<SceneData>();
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
		SceneData& Scene::getSceneData()
		{
			return *scene_data_;
		}
	}

	namespace scene
	{
		void initialize(world::SceneData& scene)
		{
			components::CameraSystem::initialize(scene);
			components::MeshRenderSystem::initialize(scene);
			components::RigidBodySystem::initialize(scene);
			components::WaveSourceSystem::initialize(scene);
		}
		void update(const float& delta_time, world::SceneData& scene)
		{
			// LOD.
			do
			{
				// Do not update the LODs every frame. Just not worth it.
				scene.lod.time += delta_time;
				if (scene.lod.time < scene.lod.update_frequency)
					break;
				scene.lod.time -= scene.lod.update_frequency;


				// Update LODs.
				glm::vec3 camera_position = components::TransformSystem::getWorldTranslation(scene.camera.main_camera, scene);

				for (auto& data : scene.lod.data)
				{
					auto* chosen_lod = &data.base_lod;
					float distance = glm::length(components::TransformSystem::getWorldTranslation(data.entity, scene) - camera_position);

					for (auto& lod : data.lods)
					{
						if (distance > lod.getDistance())
						{
							chosen_lod = &lod;
							break;
						}
					}

					components::MeshRenderSystem::setMesh(data.entity, chosen_lod->getMesh(), scene);
				}
			} while (0);

			// Rigid body system.
			do
			{
				scene.rigid_body.physics_world->render();
			} while (0);

			components::MonoBehaviourSystem::update(delta_time, scene);
			components::WaveSourceSystem::update(delta_time, scene);
		}

		void fixedUpdate(const float& delta_time, world::SceneData& scene)
		{
			// Rigid body system.
			do
			{
				scene.rigid_body.physics_world->update(delta_time);
			} while (0);

			components::MonoBehaviourSystem::fixedUpdate(delta_time, scene);
		}

		void onRender(world::SceneData& scene)
		{
			// Camera system.
			do
			{
				// Check if there is no camera. If so, the program should exit.
				if (scene.camera.main_camera == 0u)
				{
					LMB_ASSERT(false, "There was no camera set");
					return;
				}

				components::CameraSystem::bindCamera(scene.camera.main_camera, scene);
				const auto& data = scene.camera.get(scene.camera.main_camera);

				// Create render list.
				Vector<utilities::Renderable*> opaque;
				Vector<utilities::Renderable*> alpha;
				components::MeshRenderSystem::createRenderList(scene.camera.main_camera_culler, scene.camera.main_camera_frustum, scene);
				auto statics = scene.camera.main_camera_culler.getStatics();
				auto dynamics = scene.camera.main_camera_culler.getDynamics();
				components::MeshRenderSystem::createSortedRenderList(&statics, &dynamics, opaque, alpha, scene);

				// Draw all passes.
				scene.renderer->beginTimer("Main Camera");
				for (uint32_t i = 0u; i < data.shader_passes.size(); ++i)
				{
					//if (i == 0u)
					scene.renderer->setDepthStencilState(platform::DepthStencilState::Default());
					//else
					//  scene.renderer->setDepthStencilState(platform::DepthStencilState::Equal());
					scene.renderer->pushMarker("Main Camera");
					scene.renderer->bindShaderPass(data.shader_passes[i]);
					components::MeshRenderSystem::renderAll(opaque, alpha, scene);
					scene.renderer->popMarker();
				}
				scene.renderer->endTimer("Main Camera");

				// Reset the depth stencil state. // TODO (Hilze): Find out how to handle depth stencil state.
				scene.renderer->setDepthStencilState(platform::DepthStencilState::Default());
			} while (0);
		}
		void collectGarbage(world::SceneData& scene)
		{
			components::NameSystem::collectGarbage(scene);
			components::LODSystem::collectGarbage(scene);
			components::CameraSystem::collectGarbage(scene);
			components::MeshRenderSystem::collectGarbage(scene);
			components::TransformSystem::collectGarbage(scene);
			components::RigidBodySystem::collectGarbage(scene);
			components::ColliderSystem::collectGarbage(scene);
			components::MonoBehaviourSystem::collectGarbage(scene);
			components::WaveSourceSystem::collectGarbage(scene);
		}
		void deinitialize(world::SceneData& scene)
		{
			components::RigidBodySystem::deinitialize(scene);
			components::ColliderSystem::deinitialize(scene);
			components::NameSystem::deinitialize(scene);
			components::LODSystem::deinitialize(scene);
			components::CameraSystem::deinitialize(scene);
			components::MeshRenderSystem::deinitialize(scene);
			components::TransformSystem::deinitialize(scene);
			components::MonoBehaviourSystem::deinitialize(scene);
			components::WaveSourceSystem::deinitialize(scene);
		}
	}
}