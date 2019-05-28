#include "name_system.h"
#include <utils/console.h>
#include <platform/scene.h>

namespace lambda
{
	namespace components
	{
		namespace NameSystem
		{
			/////////////////////////////////////////////////////////////////////////////
			NameComponent NameSystem::addComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.name.add(entity);
				return NameComponent(entity, scene);
			}

			/////////////////////////////////////////////////////////////////////////////
			NameComponent NameSystem::getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return NameComponent(entity, scene);
			}

			/////////////////////////////////////////////////////////////////////////////
			bool NameSystem::hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.name.has(entity);
			}

			/////////////////////////////////////////////////////////////////////////////
			void NameSystem::removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.name.remove(entity);
			}

			/////////////////////////////////////////////////////////////////////////////
			void collectGarbage(scene::Scene& scene)
			{
				if (!scene.name.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.name.marked_for_delete)
					{
						const auto& it = scene.name.entity_to_data.find(entity);
						if (it != scene.name.entity_to_data.end())
						{
							uint32_t idx = it->second;
							scene.name.unused_data_entries.push(idx);
							scene.name.data_to_entity.erase(idx);
							scene.name.entity_to_data.erase(entity);
							scene.name.data[idx].valid = false;
						}
					}
					scene.name.marked_for_delete.clear();
				}
			}

			/////////////////////////////////////////////////////////////////////////////
			void deinitialize(scene::Scene& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.name.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					scene.name.remove(entity);
				collectGarbage(scene);
			}

			/////////////////////////////////////////////////////////////////////////////
			void NameSystem::setName(const entity::Entity& entity, const String& name, scene::Scene& scene)
			{
				scene.name.get(entity).name = name;
			}

			/////////////////////////////////////////////////////////////////////////////
			String NameSystem::getName(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.name.get(entity).name;
			}

			/////////////////////////////////////////////////////////////////////////////
			void NameSystem::setTags(const entity::Entity& entity, const Vector<String>& tags, scene::Scene& scene)
			{
				scene.name.get(entity).tags = tags;
			}

			/////////////////////////////////////////////////////////////////////////////
			Vector<String> NameSystem::getTags(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.name.get(entity).tags;
			}
		}

		// The system data.
		namespace NameSystem
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
				LMB_ASSERT(it != entity_to_data.end(), "NAME: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "NAME: %llu's data was not valid", entity);
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

		// The name data.
		namespace NameSystem
		{
			/////////////////////////////////////////////////////////////////////////////
			Data::Data(const Data& other)
			{
				name = other.name;
				tags = other.tags;
				entity = other.entity;
				valid = other.valid;
			}

			/////////////////////////////////////////////////////////////////////////////
			Data& Data::operator=(const Data& other)
			{
				name = other.name;
				tags = other.tags;
				entity = other.entity;
				valid = other.valid;

				return *this;
			}
		}

		/////////////////////////////////////////////////////////////////////////////
		NameComponent::NameComponent(const entity::Entity& entity, scene::Scene& scene)
			: IComponent(entity)
			, scene_(&scene)
		{
		}

		/////////////////////////////////////////////////////////////////////////////
		NameComponent::NameComponent(const NameComponent& other)
			: IComponent(other.entity_)
			, scene_(other.scene_)
		{
		}

		/////////////////////////////////////////////////////////////////////////////
		NameComponent::NameComponent()
			: IComponent(entity::Entity())
			, scene_(nullptr)
		{
		}

		/////////////////////////////////////////////////////////////////////////////
		void NameComponent::setName(const String& name)
		{
			NameSystem::setName(entity_, name, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////
		String NameComponent::getName() const
		{
			return NameSystem::getName(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////
		void NameComponent::setTags(const Vector<String>& tags)
		{
			NameSystem::setTags(entity_, tags, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////
		Vector<String> NameComponent::getTags() const
		{
			return NameSystem::getTags(entity_, *scene_);
		}
	}
}