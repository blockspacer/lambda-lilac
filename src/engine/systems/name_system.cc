#include "name_system.h"
#include <utils/console.h>

namespace lambda
{
  namespace components
  {
		/////////////////////////////////////////////////////////////////////////////
    NameComponent::NameComponent(
			const entity::Entity& entity, 
			NameSystem* system) 
			: IComponent(entity)
			, system_(system)
    {
    }

		/////////////////////////////////////////////////////////////////////////////
    NameComponent::NameComponent(const NameComponent& other)
			: IComponent(other.entity_)
			, system_(other.system_)
    {
    }

		/////////////////////////////////////////////////////////////////////////////
    NameComponent::NameComponent()
			: IComponent(entity::Entity())
			, system_(nullptr)
    {
    }

		/////////////////////////////////////////////////////////////////////////////
    void NameComponent::setName(const String& name)
    {
      system_->setName(entity_, name);
    }

		/////////////////////////////////////////////////////////////////////////////
    String NameComponent::getName() const
    {
      return system_->getName(entity_);
    }

		/////////////////////////////////////////////////////////////////////////////
		void NameComponent::setTags(const Vector<String>& tags)
		{
			system_->setTags(entity_, tags);
		}

		/////////////////////////////////////////////////////////////////////////////
		Vector<String> NameComponent::getTags() const
		{
			return system_->getTags(entity_);
		}

		/////////////////////////////////////////////////////////////////////////////
		NameComponent NameSystem::addComponent(const entity::Entity& entity)
    {
			if (!unused_data_entries_.empty())
			{
				uint32_t idx = unused_data_entries_.front();
				unused_data_entries_.pop();

				data_[idx] = NameData(entity);
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}
			else
			{
				data_.push_back(NameData(entity));
				uint32_t idx = (uint32_t)data_.size() - 1u;
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}

      return NameComponent(entity, this);
    }

		/////////////////////////////////////////////////////////////////////////////
    NameComponent NameSystem::getComponent(const entity::Entity& entity)
    {
      return NameComponent(entity, this);
    }

		/////////////////////////////////////////////////////////////////////////////
    bool NameSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }

		/////////////////////////////////////////////////////////////////////////////
    void NameSystem::removeComponent(const entity::Entity& entity)
    {
			marked_for_delete_.insert(entity);
    }

		/////////////////////////////////////////////////////////////////////////////
		void NameSystem::collectGarbage()
		{
			if (!marked_for_delete_.empty())
			{
				for (entity::Entity entity : marked_for_delete_)
				{
					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						uint32_t idx = it->second;
						unused_data_entries_.push(idx);
						data_to_entity_.erase(idx);
						entity_to_data_.erase(entity);
						data_[idx].valid = false;
					}
				}
				marked_for_delete_.clear();
			}
		}

		/////////////////////////////////////////////////////////////////////////////
    void NameSystem::initialize(world::IWorld& world)
    {
    }

		/////////////////////////////////////////////////////////////////////////////
    void NameSystem::deinitialize()
    {
    }

		/////////////////////////////////////////////////////////////////////////////
    void NameSystem::setName(const entity::Entity& entity, const String& name)
    {
      lookUpData(entity).name = name;
    }

		/////////////////////////////////////////////////////////////////////////////
		String NameSystem::getName(const entity::Entity& entity) const
    {
      return lookUpData(entity).name;
    }

		/////////////////////////////////////////////////////////////////////////////
		void NameSystem::setTags(
			const entity::Entity& entity, 
			const Vector<String>& tags)
		{
			lookUpData(entity).tags = tags;
		}

		/////////////////////////////////////////////////////////////////////////////
		Vector<String> NameSystem::getTags(const entity::Entity& entity) const
		{
			return lookUpData(entity).tags;
		}

		/////////////////////////////////////////////////////////////////////////////
    NameData& NameSystem::lookUpData(const entity::Entity& entity)
    {
      LMB_ASSERT(
				entity_to_data_.find(entity) != entity_to_data_.end(), 
				"Name: could not find component"
			);
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }

		/////////////////////////////////////////////////////////////////////////////
    const NameData& NameSystem::lookUpData(const entity::Entity& entity) const
    {
			LMB_ASSERT(
				entity_to_data_.find(entity) != entity_to_data_.end(),
				"Name: could not find component"
			);
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }

		/////////////////////////////////////////////////////////////////////////////
    NameData::NameData(const NameData& other)
    {
			name     = other.name;
			tags     = other.tags;
      entity   = other.entity;
			valid    = other.valid;
    }

		/////////////////////////////////////////////////////////////////////////////
    NameData& NameData::operator=(const NameData& other)
    {
			name     = other.name;
			tags     = other.tags;
      entity   = other.entity;
			valid    = other.valid;
      
      return *this;
    }
}
}