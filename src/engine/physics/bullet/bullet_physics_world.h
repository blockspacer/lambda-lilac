#pragma once
#include "systems/entity.h"
#include <glm/glm.hpp>
#include <containers/containers.h>
#include <memory/memory.h>
#include <interfaces/iphysics.h>
#include <assets/mesh.h>
#include <physics/bullet/bullet_physics_visualizer.h>

class btDefaultCollisionConfiguration;
class btSequentialImpulseConstraintSolver;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btMotionState;
class btCollisionShape;
class btTriangleMesh;

namespace lambda
{
	namespace physics
	{
		class MyEventListener;
		class BulletPhysicsWorld;
		enum class BulletCollisionBodyType
		{
			kNone,
			kCollider,
			kRigidBody
		};
		///////////////////////////////////////////////////////////////////////////
		class BulletCollisionBody : public ICollisionBody
		{
		public:
			BulletCollisionBody(
				scene::Scene& scene,
				btDiscreteDynamicsWorld* dynamics_world,
				BulletPhysicsWorld* physics_world,
				entity::Entity entity
			);
			virtual ~BulletCollisionBody() override;
			virtual glm::vec3 getPosition() const override;
			virtual void setPosition(glm::vec3 position) override;
			virtual glm::quat getRotation() const override;
			virtual void setRotation(glm::quat rotation) override;
			virtual entity::Entity getEntity() const override;

			virtual float getFriction() const override;
			virtual void setFriction(float friction) override;
			virtual float getMass() const override;
			virtual void setMass(float mass) override;
			virtual uint16_t getLayers() const override;
			virtual void setLayers(uint16_t layers) override;

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
			virtual void makeMeshCollider(asset::VioletMeshHandle mesh, uint32_t sub_mesh_id) override;
			void makeShape(btCollisionShape* shape, btTriangleMesh* triangle_mesh = nullptr);

			void destroyBody();
			void createBody();

			btRigidBody* getBody();

		private:
			btRigidBody* body_;
			btMotionState* motion_state_;
			btCollisionShape* collision_shape_;
			btTriangleMesh* triangle_mesh_;
			scene::Scene& scene_;
			btDiscreteDynamicsWorld* dynamics_world_; // TODO (Hilze): Try btDynamicsWorld.
			BulletPhysicsWorld* physics_world_;
			BulletCollisionBodyType type_;
			entity::Entity entity_;
			uint8_t velocity_constraints_;
			uint8_t angular_constraints_;
			uint16_t layers_;
			float mass_;

			int* indices_;
			glm::vec3* vertices_;
		};

		///////////////////////////////////////////////////////////////////////////
		class BulletPhysicsWorld : public IPhysicsWorld
		{
		public:
			BulletPhysicsWorld();
			~BulletPhysicsWorld();

			virtual void initialize(scene::Scene& scene);
			virtual void deinitialize() override;
			virtual void render(scene::Scene& scene) override;
			virtual void update(const double& time_step) override;

			virtual Vector<Manifold> raycast(
				const glm::vec3& start,
				const glm::vec3& end
			) override;

			virtual void createCollisionBody(entity::Entity entity) override;
			virtual void destroyCollisionBody(entity::Entity entity) override;
			virtual ICollisionBody& getCollisionBody(entity::Entity entity) override;

			virtual void setDebugDrawEnabled(bool debug_draw_enabled) override;
			virtual bool getDebugDrawEnabled() const override;

			virtual void setGravity(glm::vec3 gravity) override;
			virtual glm::vec3 getGravity() const override;

			Vector<BulletCollisionBody>& getCollisionBodies() { return collision_bodies_; };

		private:
			scene::Scene* scene_;
			BulletPhysicVisualizer physics_visualizer_;
			MyEventListener* event_listener_;

			btDefaultCollisionConfiguration* collision_configuration_;

			btSequentialImpulseConstraintSolver* constraint_solver_;
			btCollisionDispatcher* dispatcher_;
			btBroadphaseInterface* pair_cache_;
			btDiscreteDynamicsWorld* dynamics_world_;

			Vector<BulletCollisionBody> collision_bodies_;
		};
	}
}