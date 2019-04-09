#pragma once
#include "interfaces/icomponent.h"
#include "interfaces/isystem.h"

namespace lambda
{
	namespace world
	{
		struct SceneData;
	}

	namespace components
	{
		class NameComponent : public IComponent
		{
		public:
			NameComponent(const entity::Entity& entity, world::SceneData& scene);
			NameComponent(const NameComponent& other);
			NameComponent();

			void setName(const String& name);
			String getName() const;
			void setTags(const Vector<String>& tags);
			Vector<String> getTags() const;

		private:
			world::SceneData* scene_;
		};

		namespace NameSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				String name;
				Vector<String> tags;
				entity::Entity entity;
				bool valid = true;
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

			NameComponent addComponent(const entity::Entity& entity, world::SceneData& scene);
			NameComponent getComponent(const entity::Entity& entity, world::SceneData& scene);
			bool hasComponent(const entity::Entity& entity, world::SceneData& scene);
			void removeComponent(const entity::Entity& entity, world::SceneData& scene);

			void collectGarbage(world::SceneData& scene);
			void deinitialize(world::SceneData& scene);

			void setName(const entity::Entity& entity, const String& name, world::SceneData& scene);
			String getName(const entity::Entity& entity, world::SceneData& scene);
			void setTags(const entity::Entity& entity, const Vector<String>& tags, world::SceneData& scene);
			Vector<String> getTags(const entity::Entity& entity, world::SceneData& scene);
		}
	}
}