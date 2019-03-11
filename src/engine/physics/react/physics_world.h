#pragma once
#include "systems/entity.h"
#include <glm/glm.hpp>
#include <containers/containers.h>
#include "physics_visualizer.h"
#include <memory/memory.h>
#include <interfaces/iphysics.h>
#include <assets/mesh.h>

namespace reactphysics3d
{
  class DynamicsWorld;
  class RigidBody;
  class ProxyShape;
  class CollisionShape;
}

namespace lambda
{
  namespace components
  {
    class TransformSystem;
    class RigidBodySystem;
    class MonoBehaviourSystem;
  }

  namespace physics
  {
	  class ReactPhysicsWorld;
	  enum class CollisionBodyType
	  {
		  kNone,
		  kCollider,
		  kRigidBody
	  };
	  ///////////////////////////////////////////////////////////////////////////
	  class CollisionBody : public ICollisionBody
	  {
	  public:
		  CollisionBody(
			reactphysics3d::DynamicsWorld* dynamics_world,
			world::IWorld* world,
			ReactPhysicsWorld* physics_world,
			entity::Entity entity
		  );
		  ~CollisionBody();
		  virtual glm::vec3 getPosition() const override;
		  virtual void setPosition(glm::vec3 position) override;
		  virtual glm::quat getRotation() const override;
		  virtual void setRotation(glm::quat rotation) override;
		  virtual entity::Entity getEntity() const override;

		  virtual float getFriction() const override;
		  virtual void setFriction(float friction) override;
		  virtual float getMass() const override;
		  virtual void setMass(float mass) override;

		  virtual uint8_t getVelocityConstraints() const override;
		  virtual void setVelocityConstraints(uint8_t velocity_constraints) override;
		  virtual uint8_t getAngularConstraints() const override;
		  virtual void setAngularConstraints(uint8_t angular_constraints) override;

		  virtual glm::vec3 getVelocity() const override;
		  virtual void setVelocity(glm::vec3 velocity) override;
		  virtual glm::vec3 getAngularVelocity() const override;
		  virtual void setAngularVelocity(glm::vec3 velocity) override;

		  virtual void applyImpulse(glm::vec3 impulse) override;
		  virtual void applyImpulse(glm::vec3 impulse, glm::vec3 location) override;

		  virtual bool isCollider() const override;
		  virtual bool isRigidBody() const override;
		  virtual void makeCollider() override;
		  virtual void makeRigidBody() override;

		  virtual void makeBoxCollider() override;
		  virtual void makeSphereCollider() override;
		  virtual void makeCapsuleCollider() override;
		  virtual void makeMeshCollider(asset::MeshHandle mesh, uint32_t sub_mesh_id) override;

		  void setShape(reactphysics3d::CollisionShape* shape);

		  reactphysics3d::RigidBody* getBody() const;
		  
		  void destroyBody();

	  private:
		  reactphysics3d::RigidBody* body_;
		  Vector<reactphysics3d::ProxyShape*> proxy_shapes_;
		  Vector<reactphysics3d::CollisionShape*> collision_shapes_;
		  reactphysics3d::DynamicsWorld* dynamics_world_;
		  world::IWorld* world_;
		  ReactPhysicsWorld* physics_world_;
		  CollisionBodyType type_;
		  entity::Entity entity_;
		  uint8_t velocity_constraints_;
		  uint8_t angular_constraints_;
	  };

    ///////////////////////////////////////////////////////////////////////////
    class ReactPhysicsWorld : public IPhysicsWorld
    {
    public:
      ReactPhysicsWorld();
      ~ReactPhysicsWorld();
      
	  virtual void initialize(
				platform::DebugRenderer* debug_renderer,
				world::IWorld* world
      );
	  virtual void deinitialize() override;
	  virtual void render() override;
	  virtual void update(const double& time_step) override;

	  virtual Vector<Manifold> raycast(
		  const glm::vec3& start,
		  const glm::vec3& end
	  ) override;

	  virtual ICollisionBody* createCollisionBody(entity::Entity entity) override;
	  virtual void destroyCollisionBody(ICollisionBody* collision_body) override;

	  virtual void setDebugDrawEnabled(bool debug_draw_enabled) override;
	  virtual bool getDebugDrawEnabled() const override;

	  virtual void setGravity(glm::vec3 gravity) override;
	  virtual glm::vec3 getGravity() const override;

    private:
      foundation::SharedPointer<components::RigidBodySystem> 
        rigid_body_system_;
      
	  world::IWorld* world_;
	  PhysicVisualizer physics_visualizer_;

	  reactphysics3d::DynamicsWorld* dynamics_world_;
	  Vector<CollisionBody*> collision_bodies_;
    };
  }
}