#pragma once
#include "interfaces/icomponent.h"
#include "interfaces/isystem.h"

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

	namespace components
	{
		class NameComponent : public IComponent
		{
		public:
			NameComponent(const entity::Entity& entity, scene::Scene& scene);
			NameComponent(const NameComponent& other);
			NameComponent();

			void setName(const String& name);
			String getName() const;
			void setTags(const Vector<String>& tags);
			Vector<String> getTags() const;

		private:
			scene::Scene* scene_;
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

			NameComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			NameComponent getComponent(const entity::Entity& entity, scene::Scene& scene);
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);

			void collectGarbage(scene::Scene& scene);
			void deinitialize(scene::Scene& scene);

			void setName(const entity::Entity& entity, const String& name, scene::Scene& scene);
			String getName(const entity::Entity& entity, scene::Scene& scene);
			void setTags(const entity::Entity& entity, const Vector<String>& tags, scene::Scene& scene);
			Vector<String> getTags(const entity::Entity& entity, scene::Scene& scene);
		}
	}
}