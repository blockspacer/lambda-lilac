#pragma once
#include <containers/containers.h>
#include <systems/entity.h>
#include <glm/vec3.hpp>
#include <assets/mesh.h>

namespace lambda
{
#define VIOLET_PHYSICS_SCALE (1000.0f)
#define VIOLET_INV_PHYSICS_SCALE (1.0f / 1000.0f)

	namespace scene
	{
		struct Scene;
	}
  namespace platform
  {
    class DebugRenderer;
  }

  namespace physics
  {
	  struct Manifold
	  {
		  entity::Entity lhs;
		  entity::Entity rhs;
			struct ContactPoint
			{
				float depth;
				glm::vec3 normal;
				glm::vec3 point;
			};
			uint32_t num_contacts = 0;
			static constexpr uint32_t kMaxContacts = 4;
			ContactPoint contacts[kMaxContacts];

			bool operator==(const Manifold& other)
			{
				return lhs == other.lhs && rhs == other.rhs;
			}
	  };

	  class ICollisionBody
	  {
	  public:
		  virtual ~ICollisionBody() {};
		  virtual glm::vec3 getPosition() const = 0;
		  virtual void setPosition(glm::vec3 position) = 0;
		  virtual glm::quat getRotation() const = 0;
		  virtual void setRotation(glm::quat rotation) = 0;
		  virtual entity::Entity getEntity() const = 0;
			virtual void setEntity(entity::Entity entity) = 0;

		  virtual float getFriction() const = 0;
		  virtual void setFriction(float friction) = 0;
		  virtual float getMass() const = 0;
		  virtual void setMass(float mass) = 0;
		  virtual uint16_t getLayers() const = 0;
		  virtual void setLayers(uint16_t layers) = 0;
		  
		  virtual uint8_t getVelocityConstraints() const = 0;
		  virtual void setVelocityConstraints(uint8_t velocity_constraints) = 0;
		  virtual uint8_t getAngularConstraints() const = 0;
		  virtual void setAngularConstraints(uint8_t angular_constraints) = 0;

		  virtual glm::vec3 getVelocity() const = 0;
		  virtual void setVelocity(glm::vec3 velocity) = 0;
		  virtual glm::vec3 getAngularVelocity() const = 0;
		  virtual void setAngularVelocity(glm::vec3 velocity) = 0;

		  virtual void applyImpulse(glm::vec3 impulse) = 0;
		  virtual void applyImpulse(glm::vec3 impulse, glm::vec3 location) = 0;

		  virtual bool isCollider() const = 0;
		  virtual bool isRigidBody() const = 0;
		  virtual void makeCollider() = 0;
		  virtual void makeRigidBody() = 0;

		  virtual void makeBoxCollider() = 0;
		  virtual void makeSphereCollider() = 0;
		  virtual void makeCapsuleCollider() = 0;
		  virtual void makeMeshCollider(asset::VioletMeshHandle mesh, uint32_t sub_mesh_id) = 0;
	  };

	  class IPhysicsWorld
	  {
	  public:
		  virtual void initialize(scene::Scene& scene) = 0;
		  virtual void deinitialize() = 0;
		  virtual void render(scene::Scene& scene) = 0;
		  virtual void update(const double& time_step) = 0;

			virtual void createCollisionBody(entity::Entity entity) = 0;
			virtual void destroyCollisionBody(entity::Entity entity) = 0;
			virtual ICollisionBody& getCollisionBody(entity::Entity entity) = 0;

		  virtual Vector<Manifold> raycast(
			  const glm::vec3& start,
			  const glm::vec3& end
		  ) = 0;

		  virtual void setDebugDrawEnabled(bool debug_draw_enabled) = 0;
		  virtual bool getDebugDrawEnabled() const = 0;

		  virtual void setGravity(glm::vec3 gravity) = 0;
		  virtual glm::vec3 getGravity() const = 0;
	  };
  }
}