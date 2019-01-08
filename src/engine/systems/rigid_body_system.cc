#include "rigid_body_system.h"
#include "transform_system.h"
#include "collider_system.h"
#include "entity_system.h"
#include "interfaces/iworld.h"
#include <btBulletDynamicsCommon.h>
#include <glm/gtx/functions.hpp>

namespace lambda
{
  namespace components
  {
    RigidBodyComponent RigidBodySystem::addComponent(const entity::Entity& entity)
    {
      require(transform_system_.get(), entity);
      require(collider_system_, entity);

      data_.push_back(RigidBodyData(entity));
      data_to_entity_[(uint32_t)data_.size() - 1u] = entity.id();
      entity_to_data_[entity.id()] = (uint32_t)data_.size() - 1u;

      /** Init start */
      RigidBodyData& data = lookUpData(entity);
      ColliderData& collider = collider_system_->lookUpData(entity);
      data.rigid_body = collider.rigid_body;

      btVector3 inertia;
      collider.collision_shape->calculateLocalInertia(data.mass, inertia);
      data.inertia = glm::vec3(inertia.x(), inertia.x(), inertia.z());

      data.rigid_body->setMassProps(data.mass, inertia);
      data.rigid_body->setCollisionFlags(0);

      physics_world_.dynamicsWorld()->removeCollisionObject(data.rigid_body);
      physics_world_.dynamicsWorld()->addRigidBody(data.rigid_body);
      /** Init end */

      return RigidBodyComponent(entity, this);
    }
    RigidBodyComponent RigidBodySystem::getComponent(const entity::Entity& entity)
    {
      return RigidBodyComponent(entity, this);
    }
    bool RigidBodySystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity.id()) != entity_to_data_.end();
    }
    void RigidBodySystem::removeComponent(const entity::Entity& entity)
    {
      const auto& it = entity_to_data_.find(entity.id());
      if (it != entity_to_data_.end())
      {
        {
          RigidBodyData& data = data_.at(it->second);
          physics_world_.dynamicsWorld()->removeRigidBody(data.rigid_body);
          physics_world_.dynamicsWorld()->addCollisionObject(data.rigid_body);
        }

        uint32_t id = it->second;

        for (auto i = data_to_entity_.find(id); i != data_to_entity_.end(); i++)
        {
          entity_to_data_.at(i->second)--;
        }

        data_.erase(data_.begin() + id);
        entity_to_data_.erase(it);
        data_to_entity_.erase(id);
      }
    }
    physics::PhysicsWorld& RigidBodySystem::GetPhysicsWorld()
    {
      return physics_world_;
    }
    void RigidBodySystem::initialize(world::IWorld& world)
    {
      transform_system_ = world.getScene().getSystem<TransformSystem>();
      collider_system_ = world.getScene().getSystem<ColliderSystem>().get();
      physics_world_.initialize(
        &world.getDebugRenderer(),
        world.getScene().getSystem<entity::EntitySystem>(),
        transform_system_,
        world.getScene().getSystem<RigidBodySystem>()
      );
    }
    void RigidBodySystem::deinitialize()
    {
      collider_system_ = nullptr;
      transform_system_.reset();
      physics_world_.deinitialize();
    }
    void RigidBodySystem::update(const double& delta_time)
    {
      physics_world_.render();
    }
    void RigidBodySystem::fixedUpdate(const double& time_step)
    {
      physics_world_.update(time_step);
    }
    float RigidBodySystem::getMass(const entity::Entity& entity) const
    {
      return lookUpData(entity).mass;
    }
    void RigidBodySystem::setMass(const entity::Entity& entity, const float& mass)
    {
      RigidBodyData& data = lookUpData(entity);
      data.mass = mass;

      btVector3 inertia(0.0f, 0.0f, 0.0f);
      if (mass > 0.0f)
      {
        ColliderData& shape = collider_system_->lookUpData(entity);
        shape.collision_shape->calculateLocalInertia(data.mass, inertia);
      }
      data.inertia = glm::vec3(inertia.x(), inertia.x(), inertia.z());
      data.rigid_body->setMassProps(data.mass, inertia);
    }
    glm::vec3 RigidBodySystem::getVelocity(const entity::Entity& entity) const
    {
      const RigidBodyData& data = lookUpData(entity);
      btVector3 velocity = data.rigid_body->getLinearVelocity();
      return glm::vec3(
        velocity.x(),
        velocity.y(),
        velocity.z()
      );
    }
    void RigidBodySystem::setVelocity(const entity::Entity& entity, const glm::vec3& velocity)
    {
      if (std::isnan(velocity.x) || std::isnan(velocity.y) || std::isnan(velocity.z) || glm::dot(velocity, velocity) == 0.0f)
      {
        return;
      }
      RigidBodyData& data = lookUpData(entity);
      data.rigid_body->setLinearVelocity(
        btVector3(
          velocity.x,
          velocity.y,
          velocity.z
        )
      );
      data.rigid_body->activate();
    }
    glm::vec3 RigidBodySystem::getAngularVelocity(const entity::Entity& entity) const
    {
      const RigidBodyData& data = lookUpData(entity);
      btVector3 velocity = data.rigid_body->getAngularVelocity();
      return glm::vec3(
        velocity.x(),
        velocity.y(),
        velocity.z()
      );
    }
    void RigidBodySystem::setAngularVelocity(const entity::Entity& entity, const glm::vec3& velocity)
    {
      if (std::isnan(velocity.x) || std::isnan(velocity.y) || std::isnan(velocity.z) || glm::dot(velocity, velocity) == 0.0f)
      {
        return;
      }
      RigidBodyData& data = lookUpData(entity);
      data.rigid_body->setAngularVelocity(
        btVector3(
          velocity.x,
          velocity.y,
          velocity.z
        )
      );
      data.rigid_body->activate();
    }
    uint8_t RigidBodySystem::getAngularConstraints(const entity::Entity& entity) const
    {
      return lookUpData(entity).angular_constraints;
    }
    void RigidBodySystem::setAngularConstraints(const entity::Entity& entity, const uint8_t& constraints)
    {
      RigidBodyData& data = lookUpData(entity);

      data.rigid_body->setAngularFactor(btVector3(
        (constraints& (uint8_t)RigidBodyConstraints::kX) == 0 ? 0.0f : 1.0f,
        (constraints& (uint8_t)RigidBodyConstraints::kY) == 0 ? 0.0f : 1.0f,
        (constraints& (uint8_t)RigidBodyConstraints::kZ) == 0 ? 0.0f : 1.0f
      ));
    }
    uint8_t RigidBodySystem::getVelocityConstraints(const entity::Entity& entity) const
    {
      return lookUpData(entity).velocity_constraints;
    }
    void RigidBodySystem::setVelocityConstraints(const entity::Entity& entity, const uint8_t& constraints)
    {
      RigidBodyData& data = lookUpData(entity);

      data.rigid_body->setLinearFactor(btVector3(
        (constraints& (uint8_t)RigidBodyConstraints::kX) == 0 ? 0.0f : 1.0f,
        (constraints& (uint8_t)RigidBodyConstraints::kY) == 0 ? 0.0f : 1.0f,
        (constraints& (uint8_t)RigidBodyConstraints::kZ) == 0 ? 0.0f : 1.0f
      ));
    }
    void RigidBodySystem::applyImpulse(const entity::Entity& entity, const glm::vec3& impulse)
    {
      lookUpData(entity).rigid_body->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
    }
    RigidBodyData& RigidBodySystem::lookUpData(const entity::Entity& entity)
    {
      LMB_ASSERT(entity_to_data_.find(entity.id()) != entity_to_data_.end(), ("RigidBody: could not find component: " + toString(entity.id())).c_str());
      return data_.at(entity_to_data_.at(entity.id()));
    }
    const RigidBodyData& RigidBodySystem::lookUpData(const entity::Entity& entity) const
    {
      LMB_ASSERT(entity_to_data_.find(entity.id()) != entity_to_data_.end(), ("RigidBody: could not find component: " + toString(entity.id())).c_str());
      return data_.at(entity_to_data_.at(entity.id()));
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
    RigidBodyData::RigidBodyData(const RigidBodyData& other)
    {
      rigid_body           = other.rigid_body;
      inertia              = other.inertia;
      mass                 = other.mass;
      velocity_constraints = other.velocity_constraints;
      angular_constraints  = other.angular_constraints;
      entity               = other.entity;
    }
    RigidBodyData & RigidBodyData::operator=(const RigidBodyData & other)
    {
      rigid_body           = other.rigid_body;
      inertia              = other.inertia;
      mass                 = other.mass;
      velocity_constraints = other.velocity_constraints;
      angular_constraints  = other.angular_constraints;
      entity               = other.entity;
      return *this;
    }
  }
}
