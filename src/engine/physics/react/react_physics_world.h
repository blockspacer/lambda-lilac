#pragma once
#include "systems/entity.h"
#include <glm/glm.hpp>
#include <containers/containers.h>
#include <memory/memory.h>
#include <interfaces/iphysics.h>
#include <assets/mesh.h>
#include <physics/react/react_physics_visualizer.h>

namespace reactphysics3d
{
  class DynamicsWorld;
  class RigidBody;
  class ProxyShape;
  class CollisionShape;
}

namespace lambda
{
	namespace physics
	{
		class MyEventListener;
		class ReactPhysicsWorld;
		enum class ReactCollisionBodyType
		{
			kNone,
			kCollider,
			kRigidBody
		};
		enum class ReactCollisionColliderType
		{
			kBox,
			kSphere,
			kCapsule,
			kMesh,
		};

		extern reactphysics3d::DynamicsWorld* k_reactDynamicsWorld;
		extern scene::Scene*                  k_reactScene;
		extern ReactPhysicsWorld*             k_reactPhysicsWorld;

		///////////////////////////////////////////////////////////////////////////
		class ReactCollisionBody : public ICollisionBody
		{
		public:
			ReactCollisionBody() {};
			ReactCollisionBody(
				scene::Scene* scene,
				reactphysics3d::DynamicsWorld* dynamics_world,
				ReactPhysicsWorld* physics_world,
				entity::Entity entity
			);
			ReactCollisionBody(const ReactCollisionBody& other);
			void operator=(const ReactCollisionBody& other);
			~ReactCollisionBody();
			virtual glm::vec3 getPosition() const override;
			virtual void setPosition(glm::vec3 position) override;
			virtual glm::quat getRotation() const override;
			virtual void setRotation(glm::quat rotation) override;
			virtual entity::Entity getEntity() const override;
			virtual void setEntity(entity::Entity entity) override;

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

			void setShape(reactphysics3d::CollisionShape* shape);

			reactphysics3d::RigidBody* getBody();

			void destroyBody();

			void ensureExists(reactphysics3d::DynamicsWorld* dynamics_world, scene::Scene* scene, ReactPhysicsWorld* physics_world);
			void createBody();

			asset::VioletMeshHandle metaGetMesh() const;
			uint32_t metaGetSubMeshId() const;
			ReactCollisionBodyType metaGetType() const;
			ReactCollisionColliderType metaGetColliderType() const;

		private:
			reactphysics3d::RigidBody* body_ = nullptr;
			Vector<reactphysics3d::ProxyShape*> proxy_shapes_;
			Vector<reactphysics3d::CollisionShape*> collision_shapes_;

			asset::VioletMeshHandle mesh_;
			uint32_t sub_mesh_id_;
			int*       indices_  = nullptr;
			glm::vec3* vertices_ = nullptr;

			reactphysics3d::DynamicsWorld* dynamics_world_ = nullptr;
			scene::Scene* scene_ = nullptr;
			ReactPhysicsWorld* physics_world_ = nullptr;

			ReactCollisionBodyType type_;
			ReactCollisionColliderType collider_type_;
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
			double getTimeStep() const;

			Vector<ReactCollisionBody>& getCollisionBodies() { return collision_bodies_; };

		private:
			scene::Scene* scene_;
			ReactPhysicVisualizer physics_visualizer_;
			MyEventListener* event_listener_;

			reactphysics3d::DynamicsWorld* dynamics_world_;
			Vector<ReactCollisionBody> collision_bodies_;
			double time_step_;
		};
	}
}