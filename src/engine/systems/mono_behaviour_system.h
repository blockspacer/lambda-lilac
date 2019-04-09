#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include <containers/containers.h>
#include <memory/memory.h>
#include <glm/glm.hpp>

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

	namespace components
	{
		class MonoBehaviourComponent : public IComponent
		{
		public:
			MonoBehaviourComponent(const entity::Entity& entity, scene::Scene& scene);
			MonoBehaviourComponent(const MonoBehaviourComponent& other);
			MonoBehaviourComponent();

		private:
			scene::Scene* scene_;
		};

		namespace MonoBehaviourSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				void* object = nullptr;

				void* initialize = nullptr;
				void* deinitialize = nullptr;

				void* update = nullptr;
				void* fixed_update = nullptr;

				void* on_collision_enter = nullptr;
				void* on_collision_exit = nullptr;

				void* on_trigger_enter = nullptr;
				void* on_trigger_exit = nullptr;
				bool valid = true;

				entity::Entity entity;
			};

			struct SystemData
			{
				Vector<Data>                  data;
				Map<entity::Entity, uint32_t> entity_to_data;
				Map<uint32_t, entity::Entity> data_to_entity;
				Set<entity::Entity>           marked_for_delete;
				Queue<uint32_t>               unused_data_entries;

				Data& add(const entity::Entity& entity);
				Data& get(const entity::Entity& entity);
				void  remove(const entity::Entity& entity);
				bool  has(const entity::Entity& entity);
			};

			MonoBehaviourComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			MonoBehaviourComponent getComponent(const entity::Entity& entity, scene::Scene& scene);
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);

			void deinitialize(scene::Scene& scene);
			void collectGarbage(scene::Scene& scene);
			void update(const float& delta_time, scene::Scene& scene);
			void fixedUpdate(const float& delta_time, scene::Scene& scene);

			void setObject(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setInitialize(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setDeinitialize(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setUpdate(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setFixedUpdate(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setOnCollisionEnter(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setOnCollisionExit(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setOnTriggerEnter(const entity::Entity& entity, void* ptr, scene::Scene& scene);
			void setOnTriggerExit(const entity::Entity& entity, void* ptr, scene::Scene& scene);

			void* getObject(const entity::Entity& entity, scene::Scene& scene);
			void* getInitialize(const entity::Entity& entity, scene::Scene& scene);
			void* getDeinitialize(const entity::Entity& entity, scene::Scene& scene);
			void* getUpdate(const entity::Entity& entity, scene::Scene& scene);
			void* getFixedUpdate(const entity::Entity& entity, scene::Scene& scene);
			void* getOnCollisionEnter(const entity::Entity& entity, scene::Scene& scene);
			void* getOnCollisionExit(const entity::Entity& entity, scene::Scene& scene);
			void* getOnTriggerEnter(const entity::Entity& entity, scene::Scene& scene);
			void* getOnTriggerExit(const entity::Entity& entity, scene::Scene& scene);

			void onCollisionEnter(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene);
			void onCollisionExit(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene);
			void onTriggerEnter(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene);
			void onTriggerExit(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal, scene::Scene& scene);

			const MonoBehaviourSystem::Data* getClosest(entity::Entity entity, scene::Scene& scene);
		}
	}
}