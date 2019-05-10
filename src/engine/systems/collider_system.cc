#include "collider_system.h"
#include "rigid_body_system.h"
#include "transform_system.h"
#include "utils/mesh_decimator.h"
#include <platform/scene.h>

namespace lambda
{
	namespace components
	{
		namespace ColliderSystem
		{
			ColliderComponent addComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				if (!TransformSystem::hasComponent(entity, scene))
					TransformSystem::addComponent(entity, scene);

				scene.collider.add(entity);

				/** Init start */
				auto& data = scene.collider.get(entity);

				data.collision_body = RigidBodySystem::getPhysicsWorld(scene)->createCollisionBody(entity);

				return ColliderComponent(entity, scene);
			}
			ColliderComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return ColliderComponent(entity, scene);
			}
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.collider.has(entity);
			}
			void removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.collider.remove(entity);
			}

			void collectGarbage(scene::Scene& scene)
			{
				if (!scene.collider.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.collider.marked_for_delete)
					{
						const auto& it = scene.collider.entity_to_data.find(entity);
						if (it != scene.collider.entity_to_data.end())
						{
							auto& data = scene.collider.data.at(it->second);
							auto collision_body = data.collision_body;
							
							data.collision_body = nullptr;

							uint32_t idx = it->second;
							scene.collider.unused_data_entries.push(idx);
							scene.collider.data_to_entity.erase(idx);
							scene.collider.entity_to_data.erase(entity);
							scene.collider.data[idx].valid = false;

							if (RigidBodySystem::hasComponent(entity, scene))
								RigidBodySystem::removeComponent(entity, scene);
							RigidBodySystem::getPhysicsWorld(scene)->destroyCollisionBody(collision_body);
						}
					}
					scene.collider.marked_for_delete.clear();
				}
			}
			void deinitialize(scene::Scene& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.collider.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					removeComponent(entity, scene);
				collectGarbage(scene);
			}

			void makeBox(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.collider.get(entity).collision_body->makeBoxCollider();
			}
			void makeSphere(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.collider.get(entity).collision_body->makeSphereCollider();
			}
			void makeCapsule(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.collider.get(entity).collision_body->makeCapsuleCollider();
			}

			void makeMeshCollider(const entity::Entity& entity, asset::VioletMeshHandle mesh, const uint32_t& sub_mesh_id, scene::Scene& scene)
			{
				scene.collider.get(entity).collision_body->makeMeshCollider(mesh, sub_mesh_id);
			}

			uint16_t getLayers(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.collider.get(entity).collision_body->getLayers();
			}

			void setLayers(const entity::Entity& entity, const uint16_t& layers, scene::Scene& scene)
			{
				scene.collider.get(entity).collision_body->setLayers(layers);
			}
		}



		// The system data.
		namespace ColliderSystem
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
				LMB_ASSERT(it != entity_to_data.end(), "COLLIDER: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "COLLIDER: %llu's data was not valid", entity);
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




		namespace ColliderSystem
		{
			Data::Data(const Data& other)
			{
				type = other.type;
				collision_body = other.collision_body;
				is_trigger = other.is_trigger;
				entity = other.entity;
				valid = other.valid;
			}
			Data& Data::operator=(const Data& other)
			{
				type = other.type;
				collision_body = other.collision_body;
				is_trigger = other.is_trigger;
				entity = other.entity;
				valid = other.valid;
				return *this;
			}
		}






		ColliderComponent::ColliderComponent(const entity::Entity& entity, scene::Scene& scene) :
			IComponent(entity), scene_(&scene)
		{
		}
		ColliderComponent::ColliderComponent(const ColliderComponent& other) :
			IComponent(other.entity_), scene_(other.scene_)
		{
		}
		ColliderComponent::ColliderComponent() :
			IComponent(entity::Entity()), scene_(nullptr)
		{
		}
		void ColliderComponent::makeBoxCollider()
		{
			ColliderSystem::makeBox(entity_, *scene_);
		}
		void ColliderComponent::makeSphereCollider()
		{
			ColliderSystem::makeSphere(entity_, *scene_);
		}
		void ColliderComponent::makeCapsuleCollider()
		{
			ColliderSystem::makeCapsule(entity_, *scene_);
		}
		void ColliderComponent::makeMeshCollider(asset::VioletMeshHandle mesh, const uint32_t& sub_mesh_id)
		{
			ColliderSystem::makeMeshCollider(entity_, mesh, sub_mesh_id, *scene_);
		}

		uint16_t ColliderComponent::getLayers() const
		{
			return ColliderSystem::getLayers(entity_, *scene_);
		}

		void ColliderComponent::setLayers(const uint16_t& layers)
		{
			ColliderSystem::setLayers(entity_, layers, *scene_);
		}

	}
}