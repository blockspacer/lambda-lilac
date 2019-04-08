#include "systems/rigid_body_system.h"
#include "systems/transform_system.h"
#include "systems/collider_system.h"
#include "systems/mono_behaviour_system.h"
#include "systems/entity_system.h"
#include "interfaces/iworld.h"

#include <glm/gtx/functions.hpp>

#if VIOLET_PHYSICS_BULLET
#include "physics/bullet/bullet_physics_world.h"
#endif
#if VIOLET_PHYSICS_REACT
#include "physics/react/react_physics_world.h"
#endif
#if VIOLET_PHYSICS_NEWTON
#include "physics/newton/newton_physics_world.h"
#endif


namespace lambda
{
  namespace components
  {
    RigidBodyComponent RigidBodySystem::addComponent(const entity::Entity& entity)
    {
      require(transform_system_.get(), entity);
      require(collider_system_, entity);

			if (!unused_data_entries_.empty())
			{
				uint32_t idx = unused_data_entries_.front();
				unused_data_entries_.pop();

				data_[idx] = RigidBodyData(entity);
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}
			else
			{
				data_.push_back(RigidBodyData(entity));
				uint32_t idx = (uint32_t)data_.size() - 1u;
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}

      RigidBodyData& data = lookUpData(entity);
      ColliderData& collider = collider_system_->lookUpData(entity);
	  data.collision_body = collider.collision_body;
	  data.collision_body->makeRigidBody();

      return RigidBodyComponent(entity, this);
    }
    RigidBodyComponent RigidBodySystem::getComponent(const entity::Entity& entity)
    {
      return RigidBodyComponent(entity, this);
    }
    bool RigidBodySystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void RigidBodySystem::removeComponent(const entity::Entity& entity)
    {
    }
		void RigidBodySystem::collectGarbage()
		{
			if (!marked_for_delete_.empty())
			{
				for (entity::Entity entity : marked_for_delete_)
				{
					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						RigidBodyData& data = data_.at(it->second);
						data.collision_body->makeCollider();
						data.collision_body = nullptr;
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
	
    physics::IPhysicsWorld* RigidBodySystem::getPhysicsWorld() const
    {
      return physics_world_;
    }
    void RigidBodySystem::initialize(world::IWorld& world)
    {
      transform_system_ = world.getScene().getSystem<TransformSystem>();
      collider_system_ = world.getScene().getSystem<ColliderSystem>().get();

#if VIOLET_PHYSICS_BULLET
	  physics_world_ = foundation::Memory::construct<physics::BulletPhysicsWorld>();
#endif
#if VIOLET_PHYSICS_REACT
	  physics_world_ = foundation::Memory::construct<physics::ReactPhysicsWorld>();
#endif
#if VIOLET_PHYSICS_NEWTON
	  physics_world_ = foundation::Memory::construct<physics::NewtonPhysicsWorld>();
#endif

	  physics_world_->initialize(
        &world.getDebugRenderer(),
        &world
      );
    }
    void RigidBodySystem::deinitialize()
    {
			Vector<entity::Entity> entities;
			for (const auto& it : entity_to_data_)
				entities.push_back(it.first);

			for (const auto& entity : entities)
				removeComponent(entity);
			collectGarbage();

      collider_system_ = nullptr;
      transform_system_.reset();
      physics_world_->deinitialize();
	  foundation::Memory::destruct(physics_world_);
	  physics_world_ = nullptr;
    }
    void RigidBodySystem::update(const double& delta_time)
    {
      physics_world_->render();
    }
    void RigidBodySystem::fixedUpdate(const double& time_step)
    {
      physics_world_->update(time_step);
    }
    float RigidBodySystem::getMass(const entity::Entity& entity) const
    {
		return lookUpData(entity).collision_body->getMass();
    }
    void RigidBodySystem::setMass(const entity::Entity& entity, const float& mass)
    {
      RigidBodyData& data = lookUpData(entity);
	  lookUpData(entity).collision_body->setMass(mass);
    }
    glm::vec3 RigidBodySystem::getVelocity(const entity::Entity& entity) const
    {
		return lookUpData(entity).collision_body->getVelocity();
	}
    void RigidBodySystem::setVelocity(const entity::Entity& entity, const glm::vec3& velocity)
    {
	  if (std::isnan(velocity.x) || std::isnan(velocity.y) || std::isnan(velocity.z))
	    return;

	  lookUpData(entity).collision_body->setVelocity(velocity);
    }
    glm::vec3 RigidBodySystem::getAngularVelocity(const entity::Entity& entity) const
    {
      const RigidBodyData& data = lookUpData(entity);

	  return lookUpData(entity).collision_body->getAngularVelocity();
    }
    void RigidBodySystem::setAngularVelocity(const entity::Entity& entity, const glm::vec3& velocity)
    {
	  if (std::isnan(velocity.x) || std::isnan(velocity.y) || std::isnan(velocity.z))
        return;

	  lookUpData(entity).collision_body->setAngularVelocity(velocity);
    }
    uint8_t RigidBodySystem::getAngularConstraints(const entity::Entity& entity) const
    {
      return lookUpData(entity).collision_body->getAngularConstraints();
    }
    void RigidBodySystem::setAngularConstraints(const entity::Entity& entity, const uint8_t& constraints)
    {
		lookUpData(entity).collision_body->setAngularConstraints(constraints);
    }
    uint8_t RigidBodySystem::getVelocityConstraints(const entity::Entity& entity) const
    {
		return lookUpData(entity).collision_body->getVelocityConstraints();
    }
    void RigidBodySystem::setVelocityConstraints(const entity::Entity& entity, const uint8_t& constraints)
    {
		lookUpData(entity).collision_body->setVelocityConstraints(constraints);
	}
    void RigidBodySystem::applyImpulse(const entity::Entity& entity, const glm::vec3& impulse)
    {
		lookUpData(entity).collision_body->applyImpulse(impulse);
    }
	void RigidBodySystem::setFriction(const entity::Entity & entity, float friction)
	{
		lookUpData(entity).collision_body->setFriction(friction);
	}
	float RigidBodySystem::getFriction(const entity::Entity & entity) const
	{
		return lookUpData(entity).collision_body->getFriction();
	}
    RigidBodyData& RigidBodySystem::lookUpData(const entity::Entity& entity)
    {
      LMB_ASSERT(entity_to_data_.find(entity) != entity_to_data_.end(), ("RigidBody: could not find component: " + toString(entity)).c_str());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    const RigidBodyData& RigidBodySystem::lookUpData(const entity::Entity& entity) const
    {
      LMB_ASSERT(entity_to_data_.find(entity) != entity_to_data_.end(), ("RigidBody: could not find component: " + toString(entity)).c_str());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    RigidBodyComponent::RigidBodyComponent(const entity::Entity& entity, RigidBodySystem* system) :
      IComponent(entity), system_(system)
    {
    }
    RigidBodyComponent::RigidBodyComponent(const RigidBodyComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    RigidBodyComponent::RigidBodyComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    float RigidBodyComponent::getMass() const
    {
      return system_->getMass(entity_);
    }
    void RigidBodyComponent::setMass(const float& mass)
    {
      system_->setMass(entity_, mass);
    }
    glm::vec3 RigidBodyComponent::getVelocity() const
    {
      return system_->getVelocity(entity_);
    }
    void RigidBodyComponent::setVelocity(const glm::vec3& velocity)
    {
      system_->setVelocity(entity_, velocity);
    }
    glm::vec3 RigidBodyComponent::getAngularVelocity() const
    {
      return system_->getAngularVelocity(entity_);
    }
    void RigidBodyComponent::setAngularVelocity(const glm::vec3& velocity)
    {
      system_->setAngularVelocity(entity_, velocity);
    }
    uint8_t RigidBodyComponent::getAngularConstraints() const
    {
      return system_->getAngularConstraints(entity_);
    }
    void RigidBodyComponent::setAngularConstraints(const uint8_t& constraints)
    {
      system_->setAngularConstraints(entity_, constraints);
    }
    uint8_t RigidBodyComponent::getVelocityConstraints() const
    {
      return system_->getVelocityConstraints(entity_);
    }
    void RigidBodyComponent::setVelocityConstraints(const uint8_t& constraints)
    {
      system_->setVelocityConstraints(entity_, constraints);
    }
    void RigidBodyComponent::applyImpulse(const glm::vec3& impulse)
    {
      system_->applyImpulse(entity_, impulse);
    }
	void RigidBodyComponent::setFriction(float friction)
	{
		system_->setFriction(entity_, friction);
	}
	float RigidBodyComponent::getFriction() const
	{
		return system_->getFriction(entity_);
	}
    RigidBodyData::RigidBodyData(const RigidBodyData& other)
    {
	  collision_body       = other.collision_body;
	  entity               = other.entity;
	  valid                = other.valid;
    }
    RigidBodyData & RigidBodyData::operator=(const RigidBodyData & other)
    {
	  collision_body       = other.collision_body;
      entity               = other.entity;
	  valid                = other.valid;
      return *this;
    }
  }
}
