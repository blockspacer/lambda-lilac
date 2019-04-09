#include "systems/mono_behaviour_system.h"
#include "systems/transform_system.h"
#include "interfaces/iscript_context.h"
#include <platform/scene.h>

namespace lambda
{
	namespace components
	{
		namespace MonoBehaviourSystem
		{
			MonoBehaviourComponent addComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.mono_behaviour.add(entity);

				return MonoBehaviourComponent(entity, scene);
			}
			MonoBehaviourComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return MonoBehaviourComponent(entity, scene);
			}
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.has(entity);
			}
			void removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.mono_behaviour.remove(entity);
			}
			void collectGarbage(scene::Scene& scene)
			{
				if (!scene.mono_behaviour.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.mono_behaviour.marked_for_delete)
					{
						{
							Data& data = scene.mono_behaviour.get(entity);
#define FREE(x) if (x) scene.scripting->freeHandle(x), x = nullptr
							FREE(data.object);
							FREE(data.initialize);
							FREE(data.deinitialize);
							FREE(data.update);
							FREE(data.fixed_update);
							FREE(data.on_collision_enter);
							FREE(data.on_collision_exit);
							FREE(data.on_trigger_enter);
							FREE(data.on_trigger_exit);
#undef FREE
						}

						const auto& it = scene.mono_behaviour.entity_to_data.find(entity);
						if (it != scene.mono_behaviour.entity_to_data.end())
						{
							uint32_t idx = it->second;
							scene.mono_behaviour.unused_data_entries.push(idx);
							scene.mono_behaviour.data_to_entity.erase(idx);
							scene.mono_behaviour.entity_to_data.erase(entity);
							scene.mono_behaviour.data[idx].valid = false;
						}
						else
							Warning("Could not find id: " + toString(entity));
					}
					scene.mono_behaviour.marked_for_delete.clear();
				}
			}
			void deinitialize(scene::Scene& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.mono_behaviour.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					removeComponent(entity, scene);
				collectGarbage(scene);
			}
			void update(const float& delta_time, scene::Scene& scene)
			{
				for (uint32_t i = 0u; i < scene.mono_behaviour.data.size(); ++i)
				{
					const auto& data = scene.mono_behaviour.data[i];
					if (data.valid && data.object && data.update)
						scene.scripting->executeFunction(data.object, data.update, {});
				}
			}
			void fixedUpdate(const float& delta_time, scene::Scene& scene)
			{
				for (uint32_t i = 0u; i < scene.mono_behaviour.data.size(); ++i)
				{
					const auto& data = scene.mono_behaviour.data[i];
					if (data.valid && data.object && data.fixed_update)
						scene.scripting->executeFunction(data.object, data.fixed_update, {});
				}
			}
			void setObject(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).object = ptr;
			}
			void setInitialize(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).initialize = ptr;
			}
			void setDeinitialize(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).deinitialize = ptr;
			}
			void setUpdate(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).update = ptr;
			}
			void setFixedUpdate(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).fixed_update = ptr;
			}
			void setOnCollisionEnter(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).on_collision_enter = ptr;
			}
			void setOnCollisionExit(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).on_collision_exit = ptr;
			}
			void setOnTriggerEnter(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).on_trigger_enter = ptr;
			}
			void setOnTriggerExit(const entity::Entity& entity, void* ptr, scene::Scene& scene)
			{
				scene.mono_behaviour.get(entity).on_trigger_exit = ptr;
			}
			void* getObject(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).object;
			}
			void* getInitialize(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).initialize;
			}
			void* getDeinitialize(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).deinitialize;
			}
			void* getUpdate(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).update;
			}
			void* getFixedUpdate(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).fixed_update;
			}
			void* getOnCollisionEnter(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).on_collision_enter;
			}
			void* getOnCollisionExit(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).on_collision_exit;
			}
			void* getOnTriggerEnter(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).on_trigger_enter;
			}
			void* getOnTriggerExit(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mono_behaviour.get(entity).on_trigger_exit;
			}
#define SCR_COL(other, normal) { scripting::ScriptValue(other, true), scripting::ScriptValue(normal) }
			void onCollisionEnter(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene)
			{
				auto data_lhs = getClosest(lhs, scene);
				auto data_rhs = getClosest(rhs, scene);
				if (data_lhs) scene.scripting->executeFunction(data_lhs->object, data_lhs->on_collision_enter, SCR_COL(rhs, normal));
				if (data_rhs) scene.scripting->executeFunction(data_rhs->object, data_rhs->on_collision_enter, SCR_COL(lhs, normal));
			}
			void onCollisionExit(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene)
			{
				auto data_lhs = getClosest(lhs, scene);
				auto data_rhs = getClosest(rhs, scene);
				if (data_lhs) scene.scripting->executeFunction(data_lhs->object, data_lhs->on_collision_exit, SCR_COL(rhs, normal));
				if (data_rhs) scene.scripting->executeFunction(data_rhs->object, data_rhs->on_collision_exit, SCR_COL(lhs, normal));
			}
			void onTriggerEnter(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene)
			{
				auto data_lhs = getClosest(lhs, scene);
				auto data_rhs = getClosest(rhs, scene);
				if (data_lhs) scene.scripting->executeFunction(data_lhs->object, data_lhs->on_trigger_enter, SCR_COL(rhs, normal));
				if (data_rhs) scene.scripting->executeFunction(data_rhs->object, data_rhs->on_trigger_enter, SCR_COL(lhs, normal));
			}
			void onTriggerExit(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene)
			{
				auto data_lhs = getClosest(lhs, scene);
				auto data_rhs = getClosest(rhs, scene);
				if (data_lhs) scene.scripting->executeFunction(data_lhs->object, data_lhs->on_trigger_exit, SCR_COL(rhs, normal));
				if (data_rhs) scene.scripting->executeFunction(data_rhs->object, data_rhs->on_trigger_exit, SCR_COL(lhs, normal));
			}
			const Data* getClosest(entity::Entity entity, scene::Scene& scene)
			{
				if (hasComponent(entity, scene))
					return &scene.mono_behaviour.get(entity);

				auto parent = TransformSystem::getParent(entity, scene);
				if (parent)
					return getClosest(parent, scene);

				return nullptr;
			}
		}

		// The system data.
		namespace MonoBehaviourSystem
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
				LMB_ASSERT(it != entity_to_data.end(), "MONOBEHAVIOUR: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "MONOBEHAVIOUR: %llu's data was not valid", entity);
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

		namespace MonoBehaviourSystem
		{
			Data::Data(const Data & other)
			{
				object = other.object;
				initialize = other.initialize;
				deinitialize = other.deinitialize;
				update = other.update;
				fixed_update = other.fixed_update;
				on_collision_enter = other.on_collision_enter;
				on_collision_exit = other.on_collision_exit;
				on_trigger_enter = other.on_trigger_enter;
				on_trigger_exit = other.on_trigger_exit;
				entity = other.entity;
				valid = other.valid;
			}
			Data & Data::operator=(const Data & other)
			{
				object = other.object;
				initialize = other.initialize;
				deinitialize = other.deinitialize;
				update = other.update;
				fixed_update = other.fixed_update;
				on_collision_enter = other.on_collision_enter;
				on_collision_exit = other.on_collision_exit;
				on_trigger_enter = other.on_trigger_enter;
				on_trigger_exit = other.on_trigger_exit;
				entity = other.entity;
				valid = other.valid;

				return *this;
			}
		}

		MonoBehaviourComponent::MonoBehaviourComponent(const entity::Entity& entity, scene::Scene& scene) :
			IComponent(entity), scene_(&scene)
		{
		}
		MonoBehaviourComponent::MonoBehaviourComponent(const MonoBehaviourComponent& other) :
			IComponent(other.entity_), scene_(other.scene_)
		{
		}
		MonoBehaviourComponent::MonoBehaviourComponent() :
			IComponent(entity::Entity()), scene_(nullptr)
		{
		}
	}
}