#pragma once
#include <interfaces/icomponent.h>
#include <systems/entity.h>
#include <platform/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace lambda
{
	namespace components
	{
		struct ParticleComponent {};

		namespace ParticleSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				Vector<glm::vec3> positions;

				entity::Entity entity;
				bool valid = true;

			private:
				entity::Entity parent = entity::InvalidEntity;
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

			ParticleComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			ParticleComponent getComponent(const entity::Entity& entity, scene::Scene& scene);
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);

			void collectGarbage(scene::Scene& scene);
			void deinitialize(scene::Scene& scene);
		}
	}
}