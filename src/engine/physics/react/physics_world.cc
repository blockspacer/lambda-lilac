#include "physics_world.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include "systems/transform_system.h"
#include "systems/rigid_body_system.h"
#include "systems/entity_system.h"
#include "systems/mono_behaviour_system.h"
#include "interfaces/iworld.h"

#if VIOLET_WIN32
#pragma warning(push, 0)
#endif
#include <reactphysics3d.h>
#include <collision/ContactManifold.h>
#include <constraint/ContactPoint.h>
#if VIOLET_WIN32
#pragma warning(pop)
#endif

namespace lambda
{
	namespace physics
	{
		///////////////////////////////////////////////////////////////////////////
		reactphysics3d::Vector3 toRp(glm::vec3 v)
		{
			return reactphysics3d::Vector3(v.x, v.y, v.z);
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 toGlm(reactphysics3d::Vector3 v)
		{
			return glm::vec3(v.x, v.y, v.z);
		}

		///////////////////////////////////////////////////////////////////////////
		reactphysics3d::Quaternion toRp(glm::quat q)
		{
			return reactphysics3d::Quaternion(q.x, q.y, q.z, q.w);
		}

		///////////////////////////////////////////////////////////////////////////
		glm::quat toGlm(reactphysics3d::Quaternion q)
		{
			return glm::quat(q.w, q.x, q.y, q.z);
		}





		///////////////////////////////////////////////////////////////////////////
		CollisionBody::CollisionBody(
			reactphysics3d::DynamicsWorld* dynamics_world,
			world::IWorld* world,
			ReactPhysicsWorld* physics_world,
			entity::Entity entity)
			: body_(nullptr)
			, dynamics_world_(dynamics_world)
			, physics_world_(physics_world)
			, world_(world)
			, type_(CollisionBodyType::kNone)
			, entity_(entity)
			, velocity_constraints_(0)
			, angular_constraints_(0)
		{
			reactphysics3d::Transform transform(
				toRp(world_->getScene().getSystem<components::TransformSystem>()->getWorldTranslation(entity_)),
				toRp(world_->getScene().getSystem<components::TransformSystem>()->getWorldRotation(entity_))
			);
			body_ = dynamics_world_->createRigidBody(transform);
			body_->setUserData(foundation::Memory::construct<entity::Entity>(entity_));
		}

		///////////////////////////////////////////////////////////////////////////
		CollisionBody::~CollisionBody()
		{
			destroyBody();
			for (reactphysics3d::CollisionShape* shape : collision_shapes_)
				foundation::Memory::destruct(shape);
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 CollisionBody::getPosition() const
		{
			return toGlm(body_->getTransform().getPosition());
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setPosition(glm::vec3 position)
		{
			auto transform = body_->getTransform();
			transform.setPosition(toRp(position));
			body_->setTransform(transform);
		}

		///////////////////////////////////////////////////////////////////////////
		glm::quat CollisionBody::getRotation() const
		{
			return toGlm(body_->getTransform().getOrientation());
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setRotation(glm::quat rotation)
		{
			auto transform = body_->getTransform();
			transform.setOrientation(toRp(rotation));
			body_->setTransform(transform);
		}

		///////////////////////////////////////////////////////////////////////////
		entity::Entity CollisionBody::getEntity() const
		{
			return entity_;
		}

		///////////////////////////////////////////////////////////////////////////
		float CollisionBody::getFriction() const
		{
			return body_->getMaterial().getFrictionCoefficient();
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setFriction(float friction)
		{
			body_->getMaterial().setFrictionCoefficient(friction);
		}

		///////////////////////////////////////////////////////////////////////////
		float CollisionBody::getMass() const
		{
			return body_->getMass();
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setMass(float mass)
		{
			body_->setMass(mass);
		}

		///////////////////////////////////////////////////////////////////////////
		uint8_t CollisionBody::getVelocityConstraints() const
		{
			return velocity_constraints_;
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setVelocityConstraints(uint8_t velocity_constraints)
		{
			velocity_constraints_ = velocity_constraints;
		}

		///////////////////////////////////////////////////////////////////////////
		uint8_t CollisionBody::getAngularConstraints() const
		{
			return angular_constraints_;
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setAngularConstraints(uint8_t angular_constraints)
		{
			angular_constraints_ = angular_constraints;
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 CollisionBody::getVelocity() const
		{
			return toGlm(body_->getLinearVelocity());
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setVelocity(glm::vec3 velocity)
		{
			body_->setLinearVelocity(toRp(velocity));
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 CollisionBody::getAngularVelocity() const
		{
			return toGlm(body_->getAngularVelocity());
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setAngularVelocity(glm::vec3 velocity)
		{
			body_->setAngularVelocity(toRp(velocity));
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::applyImpulse(glm::vec3 impulse)
		{
			body_->applyForceToCenterOfMass(toRp(impulse));
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::applyImpulse(glm::vec3 impulse, glm::vec3 location)
		{
			body_->applyForce(toRp(impulse), toRp(location));
		}

		///////////////////////////////////////////////////////////////////////////
		bool CollisionBody::isCollider() const
		{
			return type_ == CollisionBodyType::kCollider;
		}

		///////////////////////////////////////////////////////////////////////////
		bool CollisionBody::isRigidBody() const
		{
			return type_ == CollisionBodyType::kRigidBody;
		}

		///////////////////////////////////////////////////////////////////////////
		reactphysics3d::RigidBody* CollisionBody::getBody() const
		{
			return body_;
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::makeBoxCollider()
		{
			setShape(foundation::Memory::construct<reactphysics3d::BoxShape>(
				toRp(world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_) * 0.5f))
			);
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::makeSphereCollider()
		{
			setShape(foundation::Memory::construct<reactphysics3d::SphereShape>(
				reactphysics3d::decimal(world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_).x * 0.5f))
			);
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::makeCapsuleCollider()
		{
			setShape(foundation::Memory::construct<reactphysics3d::CapsuleShape>(
				reactphysics3d::decimal(world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_).x * 0.5f),
				reactphysics3d::decimal(world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_).y * 0.5f))
			);
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::makeMeshCollider(asset::MeshHandle mesh, uint32_t sub_mesh_id)
		{
			// Get the indices.
			glm::vec3 scale = world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_);
			asset::SubMesh sub_mesh = mesh->getSubMeshes().at(sub_mesh_id);
			auto index_offset = sub_mesh.offsets[asset::MeshElements::kIndices];
			auto vertex_offset = sub_mesh.offsets[asset::MeshElements::kPositions];
			int* indices = (int*)foundation::Memory::allocate(index_offset.count * sizeof(int));
			glm::vec3* vertices = (glm::vec3*)foundation::Memory::allocate(vertex_offset.count * sizeof(glm::vec3));

			auto mii = mesh->get(asset::MeshElements::kIndices);
			auto mpi = mesh->get(asset::MeshElements::kPositions);

			memcpy(vertices, (char*)mpi.data + vertex_offset.offset, vertex_offset.count * mpi.size);
			for (uint32_t i = 0; i < vertex_offset.count; ++i)
				vertices[i] *= scale;


			if (sizeof(uint16_t) == mii.size)
			{
				Vector<uint16_t> idx(index_offset.count);
				memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

				for (size_t i = 0u; i < index_offset.count; ++i)
					indices[i] = (int)idx.at(i);
			}
			else
			{
				Vector<uint32_t> idx(index_offset.count);
				memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

				for (size_t i = 0u; i < index_offset.count; ++i)
					indices[i] = (int)idx.at(i);
			}

			reactphysics3d::TriangleVertexArray* triangle_array = foundation::Memory::construct<reactphysics3d::TriangleVertexArray>(
				reactphysics3d::uint(vertex_offset.count),
				(float*)vertices,
				reactphysics3d::uint(3 * sizeof(float)),
				reactphysics3d::uint(index_offset.count / 3),
				indices,
				reactphysics3d::uint(3 * sizeof(int)),
				reactphysics3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
				reactphysics3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
				);

			reactphysics3d::TriangleMesh* triangle_mesh = foundation::Memory::construct<reactphysics3d::TriangleMesh>();
			triangle_mesh->addSubpart(triangle_array);

			setShape(foundation::Memory::construct<reactphysics3d::ConcaveMeshShape>(triangle_mesh));
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::setShape(reactphysics3d::CollisionShape* shape)
		{
			for (reactphysics3d::ProxyShape* proxy_shape : proxy_shapes_)
				body_->removeCollisionShape(proxy_shape);

			for (reactphysics3d::CollisionShape* collision_shape : collision_shapes_)
				foundation::Memory::destruct(collision_shape);

			proxy_shapes_.clear();
			collision_shapes_.clear();

			collision_shapes_.push_back(shape);
			proxy_shapes_.push_back(body_->addCollisionShape(collision_shapes_.back(), reactphysics3d::Transform::identity(), 1.0f));
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::makeCollider()
		{
			type_ = CollisionBodyType::kCollider;
			body_->setType(reactphysics3d::BodyType::STATIC);
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::makeRigidBody()
		{
			type_ = CollisionBodyType::kRigidBody;
			body_->setType(reactphysics3d::BodyType::DYNAMIC);
		}

		///////////////////////////////////////////////////////////////////////////
		void CollisionBody::destroyBody()
		{
			if (type_ != CollisionBodyType::kNone)
			{
				for (reactphysics3d::ProxyShape* proxy_shape : proxy_shapes_)
					body_->removeCollisionShape(proxy_shape);

				proxy_shapes_.clear();

				foundation::Memory::destruct<entity::Entity>((entity::Entity*)body_->getUserData());
				dynamics_world_->destroyRigidBody(body_);

				body_ = nullptr;
				type_ = CollisionBodyType::kNone;
			}
		}





    ///////////////////////////////////////////////////////////////////////////
    ReactPhysicsWorld::ReactPhysicsWorld()
			: dynamics_world_(nullptr)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    ReactPhysicsWorld::~ReactPhysicsWorld()
    {
    }

		///////////////////////////////////////////////////////////////////////////
#pragma optimize("", off)
		class MyEventListener : public reactphysics3d::EventListener
		{
		public:
			virtual void newContact(const reactphysics3d::CollisionCallback::CollisionCallbackInfo& collisionInfo)
			{
				Manifold manifold;
				manifold.lhs = *((entity::Entity*)collisionInfo.body1->getUserData());
				manifold.rhs = *((entity::Entity*)collisionInfo.body2->getUserData());
				manifold.contacts.resize(collisionInfo.contactManifoldElements->getContactManifold()->getNbContactPoints());

				for (uint32_t i = 0u; i < (uint32_t)collisionInfo.contactManifoldElements->getContactManifold()->getNbContactPoints(); ++i)
				{
					const reactphysics3d::ContactPoint& contact_point = collisionInfo.contactManifoldElements->getContactManifold()->getContactPoints()[i];
					manifold.contacts[i].normal = toGlm(contact_point.getNormal());
					manifold.contacts[i].depth  = float(contact_point.getPenetrationDepth());
					manifold.contacts[i].point  = toGlm(contact_point.getLocalPointOnShape1());
				}

				update(manifold);

				manifold.rhs = *((entity::Entity*)collisionInfo.body1->getUserData());
				manifold.lhs = *((entity::Entity*)collisionInfo.body2->getUserData());
				for (uint32_t i = 0u; i < (uint32_t)collisionInfo.contactManifoldElements->getContactManifold()->getNbContactPoints(); ++i)
				{
					const reactphysics3d::ContactPoint& contact_point = collisionInfo.contactManifoldElements->getContactManifold()->getContactPoints()[i];
					manifold.contacts[i].normal = -toGlm(contact_point.getNormal());
					manifold.contacts[i].depth  = float(contact_point.getPenetrationDepth());
					manifold.contacts[i].point  = toGlm(contact_point.getLocalPointOnShape2());
				}

				update(manifold);
			}

			void update(const Manifold& manifold)
			{
				auto found = eastl::find(unaccounted_for_.begin(), unaccounted_for_.end(), manifold);

				if (found == unaccounted_for_.end())
				{
					new_collisions_.push_back(manifold);
				}
				else
				{
					unaccounted_for_.erase(found);
					stay_collisions_.push_back(manifold);
				}
			}

			void endFrame(
				Vector<Manifold>& new_collisions,
				Vector<Manifold>& stay_collisions,
				Vector<Manifold>& end_collisions)
			{
				new_collisions  = new_collisions_;
				stay_collisions = stay_collisions_;
				end_collisions  = eastl::move(unaccounted_for_);
				
				unaccounted_for_ = eastl::move(stay_collisions_);
				unaccounted_for_.insert(unaccounted_for_.end(), new_collisions_.begin(), new_collisions_.end());
				new_collisions_.clear();
			}

			Vector<Manifold> new_collisions_;
			Vector<Manifold> stay_collisions_;
			Vector<Manifold> unaccounted_for_;
		};

    ///////////////////////////////////////////////////////////////////////////
    void ReactPhysicsWorld::initialize(
      platform::DebugRenderer* debug_renderer, 
	  world::IWorld* world)
    {
			world_ = world;
			physics_visualizer_.initialize(debug_renderer, false);
#if defined(_DEBUG) || defined(DEBUG)
			physics_visualizer_.setDrawEnabled(true);
#endif


			reactphysics3d::WorldSettings settings;
			settings.defaultVelocitySolverNbIterations = 15;
			settings.defaultPositionSolverNbIterations = 8;
			settings.worldName = "Default World";
			settings.isSleepingEnabled = false;
			settings.defaultBounciness = 0.0f;

			dynamics_world_ =
				foundation::Memory::construct<reactphysics3d::DynamicsWorld>(
					toRp(glm::vec3(0.0f, -9.81f, 0.0f)), 
					settings
			);

			event_listener_ = foundation::Memory::construct<MyEventListener>();
			dynamics_world_->setEventListener(event_listener_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void ReactPhysicsWorld::deinitialize()
    {
			foundation::Memory::destruct(event_listener_);
			event_listener_ = nullptr;
			foundation::Memory::destruct(dynamics_world_);
			dynamics_world_ = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
		void ReactPhysicsWorld::render()
		{
		}

    ///////////////////////////////////////////////////////////////////////////
		void ReactPhysicsWorld::update(const double& time_step)
		{
			uint32_t offset = 0u;
			struct SavData
			{
				glm::vec3 position;
				glm::vec3 rotation;
				glm::vec3 velocity;
				glm::vec3 angular;
			};
			Vector<SavData> sav_data(collision_bodies_.size());

			for (CollisionBody* rb : collision_bodies_)
			{
				if (!rb->isRigidBody())
					continue;

				entity::Entity entity = rb->getEntity();
				glm::vec3 position = world_->getScene().getSystem<components::TransformSystem>()->getWorldTranslation(entity);
				glm::quat rotation = world_->getScene().getSystem<components::TransformSystem>()->getWorldRotation(entity);

				if (rb->getVelocityConstraints() || rb->getAngularConstraints())
				{
					sav_data[offset].position  = position;
					sav_data[offset].rotation  = glm::eulerAngles(rotation);
					sav_data[offset].velocity  = rb->getVelocity();
					sav_data[offset++].angular = rb->getAngularVelocity();
				}

				reactphysics3d::Transform transform = rb->getBody()->getTransform();
				transform.setPosition(toRp(position));
				transform.setOrientation(toRp(rotation));
				rb->getBody()->setTransform(transform);
			}

			dynamics_world_->update(reactphysics3d::decimal(time_step));

			offset = 0;

			for (CollisionBody* rb : collision_bodies_)
			{
				if (!rb->isRigidBody())
					continue;

				entity::Entity entity = rb->getEntity();
				reactphysics3d::Transform transform = rb->getBody()->getTransform();
			
				if (rb->getVelocityConstraints())
				{
					const uint8_t lin_con = rb->getAngularConstraints();
					const glm::vec3 new_pos = toGlm(transform.getPosition());
					const glm::vec3 old_pos = sav_data[offset].position;
					const glm::vec3 com_pos = glm::vec3(
						(lin_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_pos.x : new_pos.x,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_pos.y : new_pos.y,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_pos.z : new_pos.z
					);
					const glm::vec3 new_vel = rb->getVelocity();
					const glm::vec3 old_vel = sav_data[offset].velocity;
					const glm::vec3 com_vel = glm::vec3(
						(lin_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_vel.x : new_vel.x,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_vel.y : new_vel.y,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_vel.z : new_vel.z
					);

					transform.setPosition(toRp(com_pos));
					rb->getBody()->setTransform(transform);
					rb->setVelocity(com_vel);
				}
				if (rb->getAngularConstraints())
				{
					const uint8_t ang_con = rb->getAngularConstraints();
					const glm::vec3 new_rot = glm::eulerAngles(toGlm(transform.getOrientation()));
					const glm::vec3 old_rot = sav_data[offset].rotation;
					const glm::vec3 com_rot = glm::vec3(
						(ang_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_rot.x : new_rot.x,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_rot.y : new_rot.y,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_rot.z : new_rot.z
					);
					const glm::vec3 new_ang = rb->getAngularVelocity();
					const glm::vec3 old_ang = sav_data[offset].angular;
					const glm::vec3 com_ang = glm::vec3(
						(ang_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_ang.x : new_ang.x,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_ang.y : new_ang.y,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_ang.z : new_ang.z
					);

					transform.setOrientation(toRp(glm::quat(com_rot)));
					rb->getBody()->setTransform(transform);
					rb->setAngularVelocity(com_ang);
				}
				world_->getScene().getSystem<components::TransformSystem>()->setWorldTranslation(entity, toGlm(transform.getPosition()));
				world_->getScene().getSystem<components::TransformSystem>()->setWorldRotation(entity, toGlm(transform.getOrientation()));
			}

			Vector<Manifold> new_collisions;
			Vector<Manifold> stay_collisions;
			Vector<Manifold> end_collisions;
			event_listener_->endFrame(new_collisions, stay_collisions, end_collisions);

			components::MonoBehaviourSystem* mono_behaviour_system = world_->getScene().getSystem<components::MonoBehaviourSystem>().get();

			for (const Manifold& manifold : new_collisions)
				mono_behaviour_system->onCollisionEnter(manifold.lhs, manifold.rhs, manifold.contacts[0].normal * manifold.contacts[0].depth);
			//for (const Manifold& manifold : stay_collisions)
			//	mono_behaviour_system->onCollisionStay(manifold.lhs, manifold.rhs, manifold.contacts[0].normal * manifold.contacts[0].depth);
			for (const Manifold& manifold : end_collisions)
				mono_behaviour_system->onCollisionExit(manifold.lhs, manifold.rhs, manifold.contacts[0].normal * manifold.contacts[0].depth);
		}

		///////////////////////////////////////////////////////////////////////////
		class MyRaycastCallback : public reactphysics3d::RaycastCallback
		{
			virtual reactphysics3d::decimal notifyRaycastHit(
				const reactphysics3d::RaycastInfo& raycastInfo) override
			{
				Manifold manifold;
				manifold.rhs    = *(entity::Entity*)raycastInfo.body->getUserData();
				manifold.contacts.resize(1);
				manifold.contacts[0].normal = toGlm(raycastInfo.worldNormal);
				manifold.contacts[0].point  = toGlm(raycastInfo.worldPoint);
				manifolds.push_back(manifold);

				return reactphysics3d::decimal(1.0);
			}

		public:
			Vector<Manifold> manifolds;
		};

    ///////////////////////////////////////////////////////////////////////////
    Vector<Manifold> ReactPhysicsWorld::raycast(
      const glm::vec3& start, 
      const glm::vec3& end)
    {
			reactphysics3d::Ray ray(toRp(start), toRp(end));
			MyRaycastCallback callback;
			dynamics_world_->raycast(ray, &callback);

			return callback.manifolds;
    }

	///////////////////////////////////////////////////////////////////////////
	ICollisionBody* ReactPhysicsWorld::createCollisionBody(entity::Entity entity)
	{
		CollisionBody* rb = foundation::Memory::construct<CollisionBody>(
			dynamics_world_,
			world_,
			this,
			entity
		);
		rb->makeCollider();
		collision_bodies_.push_back(rb);

		return rb;
	}

		///////////////////////////////////////////////////////////////////////////
	void ReactPhysicsWorld::destroyCollisionBody(ICollisionBody* collision_body)
	{
		auto it = eastl::find(collision_bodies_.begin(), collision_bodies_.end(), collision_body);
		if (it != collision_bodies_.end())
			collision_bodies_.erase(it);

		foundation::Memory::destruct(collision_body);
	}

	///////////////////////////////////////////////////////////////////////////
	void physics::ReactPhysicsWorld::setDebugDrawEnabled(bool debug_draw_enabled)
	{
		physics_visualizer_.setDrawEnabled(debug_draw_enabled);
	}

	///////////////////////////////////////////////////////////////////////////
	bool physics::ReactPhysicsWorld::getDebugDrawEnabled() const
	{
		return physics_visualizer_.getDrawEnabled();
	}

	///////////////////////////////////////////////////////////////////////////
	void physics::ReactPhysicsWorld::setGravity(glm::vec3 gravity)
	{
		dynamics_world_->setGravity(toRp(gravity));
	}

	///////////////////////////////////////////////////////////////////////////
	glm::vec3 physics::ReactPhysicsWorld::getGravity() const
	{
		return toGlm(dynamics_world_->getGravity());
    }
  }
}
