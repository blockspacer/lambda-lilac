#include "camera_system.h"
#include "transform_system.h"
#include "mesh_render_system.h"
#include "platform/depth_stencil_state.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <utils/utilities.h>
#include "platform/debug_renderer.h"
#include <platform/scene.h>

namespace lambda
{
	namespace components
	{
		namespace CameraSystem
		{
			void collectGarbage(scene::Scene& scene)
			{
				if (!scene.camera.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.camera.marked_for_delete)
					{
						const auto& it = scene.camera.entity_to_data.find(entity);
						if (it != scene.camera.entity_to_data.end())
						{
							uint32_t idx = it->second;
							scene.camera.unused_data_entries.push(idx);
							scene.camera.data_to_entity.erase(idx);
							scene.camera.entity_to_data.erase(entity);
							scene.camera.data[idx].valid = false;
						}
					}
					scene.camera.marked_for_delete.clear();
				}
			}

			void initialize(scene::Scene& scene)
			{
				scene.camera.main_camera_culler.setCullFrequency(10u);
				scene.camera.main_camera_culler.setShouldCull(true);
				scene.camera.main_camera_culler.setCullShadowCasters(false);
			}

			void deinitialize(scene::Scene& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.camera.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					scene.camera.remove(entity);
				collectGarbage(scene);
			}

			void onRender(scene::Scene& scene)
			{
				// Check if there is no camera. If so, the program should exit.
				if (scene.camera.main_camera == 0u)
				{
					LMB_ASSERT(false, "There was no camera set");
					return;
				}

				bindCamera(scene.camera.main_camera, scene);
				const auto& data = scene.camera.get(scene.camera.main_camera);

				// Create render list.
				Vector<utilities::Renderable*> opaque;
				Vector<utilities::Renderable*> alpha;
				components::MeshRenderSystem::createRenderList(scene.camera.main_camera_culler, scene.camera.main_camera_frustum, scene);
				auto statics = scene.camera.main_camera_culler.getStatics();
				auto dynamics = scene.camera.main_camera_culler.getDynamics();
				components::MeshRenderSystem::createSortedRenderList(&statics, opaque, alpha, scene);
				components::MeshRenderSystem::createSortedRenderList(&dynamics, opaque, alpha, scene);

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
			}

			void updateCameraTransforms(scene::Scene& scene)
			{
				for (Data& data : scene.camera.data)
				{
					TransformSystem::cleanIfDirty(scene.transform.get(data.entity), scene);
					data.world_matrix = scene.transform.get(data.entity).world;
				}
			}

			CameraComponent CameraSystem::addComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				if (!TransformSystem::hasComponent(entity, scene))
					TransformSystem::addComponent(entity, scene);

				scene.camera.add(entity);

				if (scene.camera.main_camera == 0u)
					setMainCamera(entity, scene);

				return CameraComponent(entity, scene);
			}
			CameraComponent CameraSystem::getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return CameraComponent(entity, scene);
			}
			bool CameraSystem::hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.camera.has(entity);
			}
			void CameraSystem::removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.camera.remove(entity);
			}
			void CameraSystem::setFov(const entity::Entity& entity, const utilities::Angle& fov, scene::Scene& scene)
			{
				scene.camera.get(entity).fov = fov;
			}
			utilities::Angle CameraSystem::getFov(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.camera.get(entity).fov;
			}
			void CameraSystem::setNearPlane(const entity::Entity& entity, const utilities::Distance& near_plane, scene::Scene& scene)
			{
				scene.camera.get(entity).near_plane = near_plane;
			}
			utilities::Distance CameraSystem::getNearPlane(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.camera.get(entity).near_plane;
			}
			void CameraSystem::setFarPlane(const entity::Entity& entity, const utilities::Distance& far_plane, scene::Scene& scene)
			{
				scene.camera.get(entity).far_plane = far_plane;
			}
			utilities::Distance CameraSystem::getFarPlane(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.camera.get(entity).far_plane;
			}
			void CameraSystem::addShaderPass(const entity::Entity & entity, const platform::ShaderPass & shader_pass, scene::Scene& scene)
			{
				scene.camera.get(entity).shader_passes.push_back(shader_pass);
			}
			void CameraSystem::setShaderPasses(const entity::Entity & entity, const Vector<platform::ShaderPass>& shader_pass, scene::Scene& scene)
			{
				scene.camera.get(entity).shader_passes = shader_pass;
			}
			platform::ShaderPass CameraSystem::getShaderPass(const entity::Entity & entity, uint32_t id, scene::Scene& scene)
			{
				return scene.camera.get(entity).shader_passes[id];
			}
			Vector<platform::ShaderPass> CameraSystem::getShaderPasses(const entity::Entity & entity, scene::Scene& scene)
			{
				return scene.camera.get(entity).shader_passes;
			}
			void CameraSystem::bindCamera(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.camera.get(entity);

				TransformComponent transform = TransformSystem::getComponent(data.entity, scene);
				const glm::mat4x4 view = glm::inverse(transform.getWorld());
				const glm::mat4x4 projection = glm::perspective(
					data.fov.asRad(),
					(float)scene.window->getSize().x / (float)scene.window->getSize().y,
					data.near_plane.asMeter(),
					data.far_plane.asMeter()
				);

				// Update the frustum.
				scene.camera.main_camera_frustum.construct(
					projection,
					view
				);

				/*scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("view_matrix"), view));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("inverse_view_matrix"), glm::inverse(view)));

				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("projection_matrix"), projection));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("inverse_projection_matrix"), glm::inverse(projection)));

				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("view_projection_matrix"), projection * view));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("inverse_view_projection_matrix"), glm::inverse(projection * view)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("camera_position"), transform.getWorldTranslation()));

				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("camera_near"), data.near_plane.asMeter()));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("camera_far"), data.far_plane.asMeter()));*/
			}
			entity::Entity CameraSystem::getMainCamera(scene::Scene& scene)
			{
				return scene.camera.main_camera;
			}
			void CameraSystem::setMainCamera(const entity::Entity& main_camera, scene::Scene& scene)
			{
				scene.camera.main_camera = main_camera;
			}
		}

		// The system data.
		namespace CameraSystem
		{
			Data& SystemData::add(const entity::Entity& entity)
			{
				uint32_t idx = 0ul;
				if (!unused_data_entries.empty())
				{
					idx = unused_data_entries.front();
					unused_data_entries.pop();
					data[idx] = Data(entity);
				}
				else
				{
					idx = (uint32_t)data.size();
					data.push_back(Data(entity));
					data_to_entity[idx] = entity;
				}

				data_to_entity[idx] = entity;
				entity_to_data[entity] = idx;

				return data[idx];
			}

			Data& SystemData::get(const entity::Entity& entity)
			{
				auto it = entity_to_data.find(entity);
				LMB_ASSERT(it != entity_to_data.end(), "CAMERA: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "CAMERA: %llu's data was not valid", entity);
				return data[it->second];
			}

			void SystemData::remove(const entity::Entity& entity)
			{
				marked_for_delete.insert(entity);
			}

			bool SystemData::has(const entity::Entity& entity)
			{
				return entity_to_data.find(entity) != entity_to_data.end();
			}
		}

		namespace CameraSystem
		{
			Data::Data(const Data & other)
			{
				fov = other.fov;
				near_plane = other.near_plane;
				far_plane = other.far_plane;
				shader_passes = other.shader_passes;
				entity = other.entity;
				valid = other.valid;
				world_matrix = other.world_matrix;
			}
			Data& Data::operator=(const Data & other)
			{
				fov = other.fov;
				near_plane = other.near_plane;
				far_plane = other.far_plane;
				shader_passes = other.shader_passes;
				entity = other.entity;
				valid = other.valid;
				world_matrix = other.world_matrix;
				return *this;
			}
		}




		CameraComponent::CameraComponent(const entity::Entity& entity, scene::Scene& scene) :
			IComponent(entity), scene_(&scene)
		{
		}
		CameraComponent::CameraComponent(const CameraComponent& other) :
			IComponent(other.entity()), scene_(other.scene_)
		{
		}
		CameraComponent::CameraComponent() :
			IComponent(entity::Entity()), scene_(nullptr)
		{
		}

		void CameraComponent::setFov(const utilities::Angle& fov)
		{
			CameraSystem::setFov(entity_, fov, *scene_);
		}

		utilities::Angle CameraComponent::getFov() const
		{
			return CameraSystem::getFov(entity_, *scene_);
		}

		void CameraComponent::setNearPlane(const utilities::Distance& near_plane)
		{
			CameraSystem::setNearPlane(entity_, near_plane, *scene_);
		}

		utilities::Distance CameraComponent::getNearPlane() const
		{
			return CameraSystem::getNearPlane(entity_, *scene_);
		}

		void CameraComponent::setFarPlane(const utilities::Distance& far_plane)
		{
			CameraSystem::setFarPlane(entity_, far_plane, *scene_);
		}

		utilities::Distance CameraComponent::getFarPlane() const
		{
			return CameraSystem::getFarPlane(entity_, *scene_);
		}

		void CameraComponent::addShaderPass(const platform::ShaderPass & shader_pass)
		{
			CameraSystem::addShaderPass(entity_, shader_pass, *scene_);
		}

		void CameraComponent::setShaderPasses(const Vector<platform::ShaderPass>& shader_pass)
		{
			CameraSystem::setShaderPasses(entity_, shader_pass, *scene_);
		}

		platform::ShaderPass CameraComponent::getShaderPass(uint32_t id) const
		{
			return CameraSystem::getShaderPass(entity_, id, *scene_);
		}

		Vector<platform::ShaderPass> CameraComponent::getShaderPasses() const
		{
			return CameraSystem::getShaderPasses(entity_, *scene_);
		}
	}
}
