//#include "skeleton_system.h"
//#include <utils/console.h>
//
//namespace lambda
//{
//  namespace components
//  {
//    SkeletonComponent SkeletonSystem::addComponent(const entity::Entity& entity)
//    {
//      data_.push_back(SkeletonData(entity));
//      data_to_entity_[(uint32_t)data_.size() - 1u] = entity;
//      entity_to_data_[entity] = (uint32_t)data_.size() - 1u;
//
//      return SkeletonComponent(entity, this);
//    }
//    SkeletonComponent SkeletonSystem::getComponent(const entity::Entity& entity)
//    {
//      return SkeletonComponent(entity, this);
//    }
//    bool SkeletonSystem::hasComponent(const entity::Entity& entity)
//    {
//      return entity_to_data_.find(entity) != entity_to_data_.end();
//    }
//    void SkeletonSystem::removeComponent(const entity::Entity& entity)
//    {
//			marked_for_delete_.insert(entity);
//    }
//    void SkeletonSystem::initialize(world::IWorld& world)
//    {
//    }
//    void SkeletonSystem::deinitialize()
//    {
//			Vector<entity::Entity> entities;
//			for (const auto& it : entity_to_data_)
//				entities.push_back(it.first);
//
//			for (const auto& entity : entities)
//				removeComponent(entity);
//			collectGarbage();
//    }
//    void SkeletonSystem::update(const double& delta_time)
//    {
//    }
//    void SkeletonSystem::fixedUpdate(const double& time_step)
//    {
//    }
//		void SkeletonSystem::collectGarbage()
//		{
//			if (!marked_for_delete_.empty())
//			{
//				for (entity::Entity entity : marked_for_delete_)
//				{
//					const auto& it = entity_to_data_.find(entity);
//					if (it != entity_to_data_.end())
//					{
//						uint32_t id = it->second;
//
//						for (auto i = data_to_entity_.find(id); i != data_to_entity_.end(); i++)
//						{
//							entity_to_data_.at(i->second)--;
//						}
//
//						data_.erase(data_.begin() + id);
//						entity_to_data_.erase(it);
//						data_to_entity_.erase(id);
//					}
//				}
//				marked_for_delete_.clear();
//			}
//		}
//    SkeletonData& SkeletonSystem::lookUpData(const entity::Entity& entity)
//    {
//      LMB_ASSERT(entity_to_data_.find(entity) != entity_to_data_.end(), "Skeleton: could not find component");
//      return data_.at(entity_to_data_.at(entity));
//    }
//    const SkeletonData& SkeletonSystem::lookUpData(const entity::Entity& entity) const
//    {
//      LMB_ASSERT(entity_to_data_.find(entity) != entity_to_data_.end(), "Skeleton: could not find component");
//      return data_.at(entity_to_data_.at(entity));
//    }
//    SkeletonComponent::SkeletonComponent(const entity::Entity& entity, SkeletonSystem* system) :
//      IComponent(entity), system_(system)
//    {
//    }
//    SkeletonComponent::SkeletonComponent(const SkeletonComponent& other) :
//      IComponent(other.entity_), system_(other.system_)
//    {
//    }
//    SkeletonComponent::SkeletonComponent() :
//      IComponent(entity::Entity()), system_(nullptr)
//    {
//    }
//    SkeletonData::SkeletonData(const SkeletonData & other)
//    {
//      mesh   = other.mesh;
//      entity = other.entity;
//    }
//    SkeletonData & SkeletonData::operator=(const SkeletonData & other)
//    {
//      mesh   = other.mesh;
//      entity = other.entity;
//
//      return *this;
//    }
//  }
//}