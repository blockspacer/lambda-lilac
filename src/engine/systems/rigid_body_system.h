#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include <containers/containers.h>
#include <glm/vec3.hpp>
#include "physics/physics_world.h"

class btRigidBody;
struct btDefaultMotionState;

namespace lambda
{
  namespace components
  {
    class RigidBodySystem;
    class TransformSystem;
    class ColliderSystem;

    enum class RigidBodyConstraints : uint8_t
    {
      kNone = 0,
      kX = 1 << 0,
      kY = 1 << 1,
      kZ = 1 << 2,
    };

    class RigidBodyComponent : public IComponent
    {
    public:
      RigidBodyComponent(const entity::Entity& entity, RigidBodySystem* system);
      RigidBodyComponent(const RigidBodyComponent& other);
      RigidBodyComponent();

      float getMass() const;
      void setMass(const float& mass);
      glm::vec3 getVelocity() const;
      void setVelocity(const glm::vec3& velocity);
      glm::vec3 getAngularVelocity() const;
      void setAngularVelocity(const glm::vec3& velocity);
      uint8_t getAngularConstraints() const;
      void setAngularConstraints(const uint8_t& constraints);
      uint8_t getVelocityConstraints() const;
      void setVelocityConstraints(const uint8_t& constraints);
      void applyImpulse(const glm::vec3& impulse);

    private:
      RigidBodySystem* system_;
    };

    struct RigidBodyData
    {
      RigidBodyData(const entity::Entity& entity) : entity(entity) {};
      RigidBodyData(const RigidBodyData& other);
      RigidBodyData& operator=(const RigidBodyData& other);

      btRigidBody* rigid_body = nullptr;
      glm::vec3 inertia     = glm::vec3(0.0f);
      float     mass        = 1.0f;
      uint8_t   velocity_constraints = 0u;
      uint8_t   angular_constraints  = 0u;
      
			bool valid = true;
      entity::Entity entity;
    };

    class RigidBodySystem : public ISystem
    {
    public:
      friend class physics::PhysicsWorld;
    public:
      static size_t systemId() { return (size_t)SystemIds::kRigidBodySystem; };
      RigidBodyComponent addComponent(const entity::Entity& entity);
      RigidBodyComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);
      physics::PhysicsWorld& GetPhysicsWorld();
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void update(const double& delta_time) override;
      virtual void fixedUpdate(const double& time_step) override;
			virtual void collectGarbage() override;
			virtual ~RigidBodySystem() override {};

      float getMass(const entity::Entity& entity) const;
      void setMass(const entity::Entity& entity, const float& mass);
      glm::vec3 getVelocity(const entity::Entity& entity) const;
      void setVelocity(const entity::Entity& entity, const glm::vec3& velocity);
      glm::vec3 getAngularVelocity(const entity::Entity& entity) const;
      void setAngularVelocity(const entity::Entity& entity, const glm::vec3& velocity);
      uint8_t getAngularConstraints(const entity::Entity& entity) const;
      void setAngularConstraints(const entity::Entity& entity, const uint8_t& constraints);
      uint8_t getVelocityConstraints(const entity::Entity& entity) const;
      void setVelocityConstraints(const entity::Entity& entity, const uint8_t& constraints);
      void applyImpulse(const entity::Entity& entity, const glm::vec3& impulse);

    protected:
      RigidBodyData& lookUpData(const entity::Entity& entity);
      const RigidBodyData& lookUpData(const entity::Entity& entity) const;

    private:
      Vector<RigidBodyData> data_;
			Map<entity::Entity, uint32_t> entity_to_data_;
			Map<uint32_t, entity::Entity> data_to_entity_;
			Set<entity::Entity> marked_for_delete_;
			Queue<uint32_t> unused_data_entries_;

      foundation::SharedPointer<TransformSystem> transform_system_;
      ColliderSystem* collider_system_; // This should be a shared pointer. But the engine would not compile with this as a shared pointer.
      physics::PhysicsWorld physics_world_;
    };
  }
}