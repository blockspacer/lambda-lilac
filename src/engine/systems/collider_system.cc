#include "collider_system.h"
#include "rigid_body_system.h"
#include "transform_system.h"
#include "interfaces/iworld.h"
#include "utils/mesh_decimator.h"

namespace lambda
{
  namespace components
  {
    ColliderComponent ColliderSystem::addComponent(const entity::Entity& entity)
    {
      require(transform_system_.get(), entity);

			if (!unused_data_entries_.empty())
			{
				uint32_t idx = unused_data_entries_.front();
				unused_data_entries_.pop();

				data_[idx] = ColliderData(entity);
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}
			else
			{
				data_.push_back(ColliderData(entity));
				uint32_t idx = (uint32_t)data_.size() - 1u;
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}

      /** Init start */
      ColliderData& data = lookUpData(entity);
      
	    data.collision_body = rigid_body_system_->getPhysicsWorld()->createCollisionBody(entity);

      return ColliderComponent(entity, this);
    }
    ColliderComponent ColliderSystem::getComponent(const entity::Entity& entity)
    {
      return ColliderComponent(entity, this);
    }
    bool ColliderSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void ColliderSystem::removeComponent(const entity::Entity& entity)
    {
      if (true == rigid_body_system_->hasComponent(entity))
        rigid_body_system_->removeComponent(entity);
			marked_for_delete_.insert(entity);
    }
		void ColliderSystem::collectGarbage()
		{
			if (!marked_for_delete_.empty())
			{
				for (entity::Entity entity : marked_for_delete_)
				{
					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						{
							ColliderData& data = data_.at(it->second);
							rigid_body_system_->getPhysicsWorld()->destroyCollisionBody(data.collision_body);
							data.collision_body = nullptr;
						}

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
    void ColliderSystem::initialize(world::IWorld& world)
    {
      transform_system_  = world.getScene().getSystem<TransformSystem>();
      rigid_body_system_ = world.getScene().getSystem<RigidBodySystem>();
    }
    void ColliderSystem::deinitialize()
    {
			Vector<entity::Entity> entities;
			for (const auto& it : entity_to_data_)
				entities.push_back(it.first);

			for (const auto& entity : entities)
				removeComponent(entity);
			collectGarbage();

      rigid_body_system_.reset();
      transform_system_.reset();
    }
    void ColliderSystem::makeBox(const entity::Entity& entity)
    {
		lookUpData(entity).collision_body->makeBoxCollider();
	}
    void ColliderSystem::makeSphere(const entity::Entity& entity)
    {
		lookUpData(entity).collision_body->makeSphereCollider();
	}
    void ColliderSystem::makeCapsule(const entity::Entity& entity)
    {
		lookUpData(entity).collision_body->makeCapsuleCollider();
    }

    void ColliderSystem::makeMeshCollider(const entity::Entity& entity, asset::VioletMeshHandle mesh, const uint32_t& sub_mesh_id)
    {
		lookUpData(entity).collision_body->makeMeshCollider(mesh, sub_mesh_id);
	}

	uint16_t ColliderSystem::getLayers(const entity::Entity& entity) const
	{
		return lookUpData(entity).collision_body->getLayers();
	}

	void ColliderSystem::setLayers(const entity::Entity& entity, const uint16_t& layers)
	{
		lookUpData(entity).collision_body->setLayers(layers);
	}

	ColliderData& ColliderSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    const ColliderData& ColliderSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    ColliderComponent::ColliderComponent(const entity::Entity& entity, ColliderSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    ColliderComponent::ColliderComponent(const ColliderComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    ColliderComponent::ColliderComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    void ColliderComponent::makeBoxCollider()
    {
		system_->makeBox(entity_);
	}
    void ColliderComponent::makeSphereCollider()
    {
		system_->makeSphere(entity_);
	}
    void ColliderComponent::makeCapsuleCollider()
    {
		system_->makeCapsule(entity_);
	}
    void ColliderComponent::makeMeshCollider(asset::VioletMeshHandle mesh, const uint32_t& sub_mesh_id)
    {
		system_->makeMeshCollider(entity_, mesh, sub_mesh_id);
    }

	uint16_t ColliderComponent::getLayers() const
	{
		return system_->getLayers(entity_);
	}

	void ColliderComponent::setLayers(const uint16_t& layers)
	{
		system_->setLayers(entity_, layers);
	}

	ColliderData::ColliderData(const ColliderData& other)
    {
      type           = other.type;
	  collision_body = other.collision_body;
      is_trigger     = other.is_trigger;
      entity         = other.entity;
	  valid          = other.valid;
    }
    ColliderData & ColliderData::operator=(const ColliderData & other)
    {
      type           = other.type;
	  collision_body = other.collision_body;
	  is_trigger     = other.is_trigger;
	  entity         = other.entity;
	  valid          = other.valid;
	  return *this;
    }
}
}