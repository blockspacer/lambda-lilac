#if VIOLET_WIN32
#pragma warning(disable: 4503)
#pragma warning(push, 0)
#pragma warning(disable: 4996)
#endif
#include "react_physics_world.h"
#if VIOLET_WIN32
#pragma warning(pop)
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include "systems/transform_system.h"
#include "systems/rigid_body_system.h"
#include "systems/entity_system.h"
#include "systems/mono_behaviour_system.h"
#include "interfaces/iworld.h"

#if VIOLET_WIN32
#pragma warning(push, 0)
#pragma warning(disable: 4996)
#endif
#include <reactphysics3d.h>
#include <collision/ContactManifold.h>
#include <constraint/ContactPoint.h>
#if VIOLET_WIN32
#pragma warning(pop)
#endif

namespace lambda
{
	// TODO (Hilze): Support physics scale for React.
#undef VIOLET_PHYSICS_SCALE
#define VIOLET_PHYSICS_SCALE (1.0f)
#undef VIOLET_INV_PHYSICS_SCALE
#define VIOLET_INV_PHYSICS_SCALE (1.0f)
	namespace physics
	{
		reactphysics3d::DynamicsWorld* k_reactDynamicsWorld = nullptr;
		scene::Scene*                  k_reactScene         = nullptr;
		ReactPhysicsWorld*             k_reactPhysicsWorld  = nullptr;

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
		inline static bool isNaN(const reactphysics3d::Vector3& v)
		{
			return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
		}





		///////////////////////////////////////////////////////////////////////////
		ReactCollisionBody::ReactCollisionBody(
			scene::Scene* scene,
			reactphysics3d::DynamicsWorld* dynamics_world,
			ReactPhysicsWorld* physics_world,
			entity::Entity entity)
			: body_(nullptr)
			, dynamics_world_(dynamics_world)
			, physics_world_(physics_world)
			, scene_(scene)
			, type_(ReactCollisionBodyType::kNone)
			, entity_(entity)
			, velocity_constraints_(0)
			, angular_constraints_(0)
		{
			createBody();
		}

		///////////////////////////////////////////////////////////////////////////
		ReactCollisionBody::ReactCollisionBody(const ReactCollisionBody& other)
		{
			mesh_                 = other.mesh_;
			sub_mesh_id_          = other.sub_mesh_id_;
			dynamics_world_       = other.dynamics_world_;
			scene_                = other.scene_;
			physics_world_        = other.physics_world_;
			type_                 = other.type_;
			collider_type_        = other.collider_type_;
			entity_               = other.entity_;
			velocity_constraints_ = other.velocity_constraints_;
			angular_constraints_  = other.angular_constraints_;

			ensureExists(dynamics_world_, scene_, physics_world_);
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::operator=(const ReactCollisionBody& other)
		{
			mesh_                 = other.mesh_;
			sub_mesh_id_          = other.sub_mesh_id_;
			dynamics_world_       = other.dynamics_world_;
			scene_                = other.scene_;
			physics_world_        = other.physics_world_;
			type_                 = other.type_;
			collider_type_        = other.collider_type_;
			entity_               = other.entity_;
			velocity_constraints_ = other.velocity_constraints_;
			angular_constraints_  = other.angular_constraints_;

			ensureExists(dynamics_world_, scene_, physics_world_);
		}

		///////////////////////////////////////////////////////////////////////////
		ReactCollisionBody::~ReactCollisionBody()
		{
			destroyBody();

			if (indices_)
			{
				foundation::Memory::deallocate(indices_);
				indices_ = nullptr;
			}
			if (vertices_)
			{
				foundation::Memory::deallocate(vertices_);
				vertices_ = nullptr;
			}

			for (reactphysics3d::CollisionShape* shape : collision_shapes_)
				foundation::Memory::destruct(shape);
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 ReactCollisionBody::getPosition() const
		{
			return toGlm(body_->getTransform().getPosition()) * VIOLET_INV_PHYSICS_SCALE;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setPosition(glm::vec3 position)
		{
			auto transform = body_->getTransform();
			transform.setPosition(toRp(position * VIOLET_PHYSICS_SCALE));
			body_->setTransform(transform);
		}

		///////////////////////////////////////////////////////////////////////////
		glm::quat ReactCollisionBody::getRotation() const
		{
			return toGlm(body_->getTransform().getOrientation());
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setRotation(glm::quat rotation)
		{
			auto transform = body_->getTransform();
			transform.setOrientation(toRp(rotation));
			body_->setTransform(transform);
		}

		///////////////////////////////////////////////////////////////////////////
		entity::Entity ReactCollisionBody::getEntity() const
		{
			return entity_;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setEntity(entity::Entity entity)
		{
			entity_ = entity;
		}

		///////////////////////////////////////////////////////////////////////////
		float ReactCollisionBody::getFriction() const
		{
			return body_->getMaterial().getFrictionCoefficient();
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setFriction(float friction)
		{
			body_->getMaterial().setFrictionCoefficient(friction);
		}

		///////////////////////////////////////////////////////////////////////////
		float ReactCollisionBody::getMass() const
		{
			return body_->getMass();
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setMass(float mass)
		{
			body_->setMass(mass);
		}

		///////////////////////////////////////////////////////////////////////////
		uint16_t ReactCollisionBody::getLayers() const
		{
			for (reactphysics3d::ProxyShape* proxy_shape : proxy_shapes_)
				return proxy_shape->getCollisionCategoryBits();
			return 0;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setLayers(uint16_t layers)
		{
			for (reactphysics3d::ProxyShape* proxy_shape : proxy_shapes_)
			{
				proxy_shape->setCollisionCategoryBits(layers);
				proxy_shape->setCollideWithMaskBits(layers);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		uint8_t ReactCollisionBody::getVelocityConstraints() const
		{
			return velocity_constraints_;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setVelocityConstraints(uint8_t velocity_constraints)
		{
			velocity_constraints_ = velocity_constraints;
		}

		///////////////////////////////////////////////////////////////////////////
		uint8_t ReactCollisionBody::getAngularConstraints() const
		{
			return angular_constraints_;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setAngularConstraints(uint8_t angular_constraints)
		{
			angular_constraints_ = angular_constraints;
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 ReactCollisionBody::getVelocity() const
		{
			return toGlm(body_->getLinearVelocity()) * VIOLET_INV_PHYSICS_SCALE;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setVelocity(glm::vec3 velocity)
		{
			body_->setLinearVelocity(toRp(velocity * VIOLET_PHYSICS_SCALE));
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 ReactCollisionBody::getAngularVelocity() const
		{
			return toGlm(body_->getAngularVelocity()) * VIOLET_INV_PHYSICS_SCALE;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setAngularVelocity(glm::vec3 velocity)
		{
			body_->setAngularVelocity(toRp(velocity * VIOLET_PHYSICS_SCALE));
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::applyImpulse(glm::vec3 impulse)
		{
			body_->applyForceToCenterOfMass(toRp(impulse * (1.0f / (float)physics_world_->getTimeStep()) * VIOLET_PHYSICS_SCALE));
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::applyImpulse(glm::vec3 impulse, glm::vec3 location)
		{
			body_->applyForce(toRp(impulse * (1.0f / (float)physics_world_->getTimeStep()) * VIOLET_PHYSICS_SCALE), toRp(location * VIOLET_PHYSICS_SCALE));
		}

		///////////////////////////////////////////////////////////////////////////
		bool ReactCollisionBody::isCollider() const
		{
			return type_ == ReactCollisionBodyType::kCollider;
		}

		///////////////////////////////////////////////////////////////////////////
		bool ReactCollisionBody::isRigidBody() const
		{
			return type_ == ReactCollisionBodyType::kRigidBody;
		}

		///////////////////////////////////////////////////////////////////////////
		reactphysics3d::RigidBody* ReactCollisionBody::getBody()
		{
			return body_;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::makeBoxCollider()
		{
			collider_type_ = ReactCollisionColliderType::kBox;
			setShape(foundation::Memory::construct<reactphysics3d::BoxShape>(
				toRp(components::TransformSystem::getWorldScale(entity_, *scene_) * 0.5f * VIOLET_PHYSICS_SCALE))
			);
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::makeSphereCollider()
		{
			collider_type_ = ReactCollisionColliderType::kSphere;
			setShape(foundation::Memory::construct<reactphysics3d::SphereShape>(
				reactphysics3d::decimal(components::TransformSystem::getWorldScale(entity_, *scene_).x * 0.5f * VIOLET_PHYSICS_SCALE))
			);
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::makeCapsuleCollider()
		{
			collider_type_ = ReactCollisionColliderType::kCapsule;
			setShape(foundation::Memory::construct<reactphysics3d::CapsuleShape>(
				reactphysics3d::decimal(components::TransformSystem::getWorldScale(entity_, *scene_).x * 0.5f * VIOLET_PHYSICS_SCALE),
				reactphysics3d::decimal(components::TransformSystem::getWorldScale(entity_, *scene_).y * 0.5f * VIOLET_PHYSICS_SCALE))
			);
		}

		bool closeEnough(const glm::vec3& lhs, const glm::vec3& rhs)
		{
			float epsilon = 0.0001f;
			glm::vec3 diff = lhs - rhs;
			return (abs(diff.x) < epsilon) && (abs(diff.y) < epsilon) && (abs(diff.z) < epsilon);
		}

		bool allCornersClose(const glm::vec3& point, const glm::vec3& min, const glm::vec3& max)
		{
			return  closeEnough(point, glm::vec3(min.x, min.y, min.z)) ||
				closeEnough(point, glm::vec3(min.x, min.y, max.z)) ||
				closeEnough(point, glm::vec3(min.x, max.y, min.z)) ||
				closeEnough(point, glm::vec3(min.x, max.y, max.z)) ||
				closeEnough(point, glm::vec3(max.x, min.y, min.z)) ||
				closeEnough(point, glm::vec3(max.x, min.y, max.z)) ||
				closeEnough(point, glm::vec3(max.x, max.y, min.z)) ||
				closeEnough(point, glm::vec3(max.x, max.y, max.z));
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::makeMeshCollider(asset::VioletMeshHandle mesh, uint32_t sub_mesh_id)
		{
			if (indices_)
			{
				foundation::Memory::deallocate(indices_);
				indices_ = nullptr;
			}
			if (vertices_)
			{
				foundation::Memory::deallocate(vertices_);
				vertices_ = nullptr;
			}

			// Get the indices.
			glm::vec3 scale = components::TransformSystem::getWorldScale(entity_, *scene_);
			asset::SubMesh sub_mesh = mesh->getSubMeshes().at(sub_mesh_id);
			auto index_offset = sub_mesh.offsets[asset::MeshElements::kIndices];
			auto vertex_offset = sub_mesh.offsets[asset::MeshElements::kPositions];
			indices_ = (int*)foundation::Memory::allocate(index_offset.count * sizeof(int));
			vertices_ = (glm::vec3*)foundation::Memory::allocate(vertex_offset.count * sizeof(glm::vec3));

			auto mii = mesh->get(asset::MeshElements::kIndices);
			auto mpi = mesh->get(asset::MeshElements::kPositions);

			memcpy(vertices_, (char*)mpi.data + vertex_offset.offset, vertex_offset.count * mpi.size);
			for (uint32_t i = 0; i < vertex_offset.count; ++i)
				vertices_[i] *= scale;

			if (sizeof(uint16_t) == mii.size)
			{
				Vector<uint16_t> idx(index_offset.count);
				memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

				for (size_t i = 0u; i < index_offset.count; ++i)
					indices_[i] = (int)idx.at(i);
			}
			else
			{
				Vector<uint32_t> idx(index_offset.count);
				memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

				for (size_t i = 0u; i < index_offset.count; ++i)
					indices_[i] = (int)idx.at(i);
			}

			glm::vec3 min = sub_mesh.min * scale;
			glm::vec3 max = sub_mesh.max * scale;

			bool is_aabb = true;
			for (uint32_t i = 0; i < index_offset.count; ++i)
			{
				const glm::vec3& vertex = vertices_[indices_[i]];
				if (!allCornersClose(vertex, min, max))
					is_aabb = false;
			}

			if (is_aabb)
			{
				glm::vec3 center = (max + min) * 0.5f;
				glm::vec3 size = max - min;
				setPosition(getPosition() + center);
				setShape(foundation::Memory::construct<reactphysics3d::BoxShape>(toRp(size * 0.5f * VIOLET_PHYSICS_SCALE)));
				return;
			}

			for (uint32_t i = 0; i < vertex_offset.count; ++i)
				vertices_[i] *= VIOLET_PHYSICS_SCALE;

			reactphysics3d::TriangleVertexArray* triangle_array = foundation::Memory::construct<reactphysics3d::TriangleVertexArray>(
				reactphysics3d::uint(vertex_offset.count),
				(float*)vertices_,
				reactphysics3d::uint(3 * sizeof(float)),
				reactphysics3d::uint(index_offset.count / 3),
				indices_,
				reactphysics3d::uint(3 * sizeof(int)),
				reactphysics3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
				reactphysics3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
				);

			reactphysics3d::TriangleMesh* triangle_mesh = foundation::Memory::construct<reactphysics3d::TriangleMesh>();
			triangle_mesh->addSubpart(triangle_array);

			mesh_        = mesh;
			sub_mesh_id_ = sub_mesh_id;
			collider_type_ = ReactCollisionColliderType::kMesh;
			setShape(foundation::Memory::construct<reactphysics3d::ConcaveMeshShape>(triangle_mesh));
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::setShape(reactphysics3d::CollisionShape* shape)
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
		void ReactCollisionBody::makeCollider()
		{
			type_ = ReactCollisionBodyType::kCollider;
			body_->setType(reactphysics3d::BodyType::STATIC);
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::makeRigidBody()
		{
			type_ = ReactCollisionBodyType::kRigidBody;
			body_->setType(reactphysics3d::BodyType::DYNAMIC);
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactCollisionBody::destroyBody()
		{
			if (type_ != ReactCollisionBodyType::kNone && body_)
			{
				for (reactphysics3d::ProxyShape* proxy_shape : proxy_shapes_)
					body_->removeCollisionShape(proxy_shape);

				proxy_shapes_.clear();

				foundation::Memory::destruct<entity::Entity>((entity::Entity*)body_->getUserData());
				dynamics_world_->destroyRigidBody(body_);

				body_ = nullptr;
				type_ = ReactCollisionBodyType::kNone;
			}
		}

		void ReactCollisionBody::ensureExists(reactphysics3d::DynamicsWorld* dynamics_world, scene::Scene* scene, ReactPhysicsWorld* physics_world)
		{
			if (body_ == nullptr)
			{
				dynamics_world_ = dynamics_world;
				scene_          = scene;
				physics_world_  = physics_world;

				createBody();

				switch (collider_type_)
				{
				case ReactCollisionColliderType::kBox:     makeBoxCollider();                     break;
				case ReactCollisionColliderType::kSphere:  makeSphereCollider();                  break;
				case ReactCollisionColliderType::kCapsule: makeCapsuleCollider();                 break;
				case ReactCollisionColliderType::kMesh:    makeMeshCollider(mesh_, sub_mesh_id_); break;
				}

				if (type_ == ReactCollisionBodyType::kCollider)
					makeCollider();
				else if (type_ == ReactCollisionBodyType::kRigidBody)
					makeRigidBody();
			}
		}

		void ReactCollisionBody::createBody()
		{
			reactphysics3d::Transform transform(
				toRp(components::TransformSystem::getWorldTranslation(entity_, *scene_) * VIOLET_PHYSICS_SCALE),
				toRp(components::TransformSystem::getWorldRotation(entity_, *scene_))
			);
			body_ = dynamics_world_->createRigidBody(transform);
			body_->setUserData(foundation::Memory::construct<entity::Entity>(entity_));
		}

		asset::VioletMeshHandle ReactCollisionBody::metaGetMesh() const
		{
			return mesh_;
		}

		uint32_t ReactCollisionBody::metaGetSubMeshId() const
		{
			return sub_mesh_id_;
		}

		ReactCollisionBodyType ReactCollisionBody::metaGetType() const
		{
			return type_;
		}

		ReactCollisionColliderType ReactCollisionBody::metaGetColliderType() const
		{
			return collider_type_;
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
		class MyEventListener : public reactphysics3d::EventListener
		{
		public:
			virtual void newContact(const reactphysics3d::CollisionCallback::CollisionCallbackInfo& collisionInfo)
			{
				Manifold manifold;
				manifold.lhs = *((entity::Entity*)collisionInfo.body1->getUserData());
				manifold.rhs = *((entity::Entity*)collisionInfo.body2->getUserData());
				manifold.num_contacts = std::min((uint32_t)collisionInfo.contactManifoldElements->getContactManifold()->getNbContactPoints(), Manifold::kMaxContacts);

				for (uint32_t i = 0u; i < manifold.num_contacts; ++i)
				{
					const reactphysics3d::ContactPoint& contact_point = collisionInfo.contactManifoldElements->getContactManifold()->getContactPoints()[i];
					manifold.contacts[i].normal = toGlm(contact_point.getNormal());
					manifold.contacts[i].depth = float(contact_point.getPenetrationDepth()) * VIOLET_INV_PHYSICS_SCALE;
					manifold.contacts[i].point = toGlm(contact_point.getLocalPointOnShape1()) * VIOLET_INV_PHYSICS_SCALE;
				}

				update(manifold);

				manifold.rhs = *((entity::Entity*)collisionInfo.body1->getUserData());
				manifold.lhs = *((entity::Entity*)collisionInfo.body2->getUserData());
				for (uint32_t i = 0u; i < manifold.num_contacts; ++i)
				{
					const reactphysics3d::ContactPoint& contact_point = collisionInfo.contactManifoldElements->getContactManifold()->getContactPoints()[i];
					manifold.contacts[i].normal = -toGlm(contact_point.getNormal());
					manifold.contacts[i].depth = float(contact_point.getPenetrationDepth()) * VIOLET_INV_PHYSICS_SCALE;
					manifold.contacts[i].point = toGlm(contact_point.getLocalPointOnShape2()) * VIOLET_INV_PHYSICS_SCALE;
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
				new_collisions = new_collisions_;
				stay_collisions = stay_collisions_;
				end_collisions = eastl::move(unaccounted_for_);

				unaccounted_for_ = eastl::move(stay_collisions_);
				unaccounted_for_.insert(unaccounted_for_.end(), new_collisions_.begin(), new_collisions_.end());
				new_collisions_.clear();
			}

			Vector<Manifold> new_collisions_;
			Vector<Manifold> stay_collisions_;
			Vector<Manifold> unaccounted_for_;
		};

		///////////////////////////////////////////////////////////////////////////
		void ReactPhysicsWorld::initialize(scene::Scene& scene)
		{
			scene_ = &scene;
			physics_visualizer_.initialize(&scene_->debug_renderer, false);
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
					toRp(glm::vec3(0.0f, -9.81f, 0.0f) * VIOLET_PHYSICS_SCALE),
					settings
					);

			event_listener_ = foundation::Memory::construct<MyEventListener>();
			dynamics_world_->setEventListener(event_listener_);

			k_reactDynamicsWorld = dynamics_world_;
			k_reactScene         = scene_;
			k_reactPhysicsWorld  = this;
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
		void ReactPhysicsWorld::render(scene::Scene& scene)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactPhysicsWorld::update(const double& time_step)
		{
			time_step_ = time_step;
			uint32_t offset = 0u;
			struct SavData
			{
				glm::vec3 position;
				glm::vec3 rotation;
				glm::vec3 velocity;
				glm::vec3 angular;
			};
			Vector<SavData> sav_data(collision_bodies_.size());

			for (ReactCollisionBody& rb : collision_bodies_)
			{
				if (!rb.isRigidBody())
					continue;

				entity::Entity entity = rb.getEntity();
				glm::vec3 position = components::TransformSystem::getWorldTranslation(entity, *scene_) * VIOLET_PHYSICS_SCALE;
				glm::quat rotation = components::TransformSystem::getWorldRotation(entity, *scene_);

				if (rb.getVelocityConstraints() || rb.getAngularConstraints())
				{
					sav_data[offset].position = position;
					sav_data[offset].rotation = glm::eulerAngles(rotation);
					sav_data[offset].velocity = rb.getVelocity() * VIOLET_PHYSICS_SCALE;
					sav_data[offset++].angular = rb.getAngularVelocity() * VIOLET_PHYSICS_SCALE;
				}

				reactphysics3d::Transform transform = rb.getBody()->getTransform();
				transform.setPosition(toRp(position));
				transform.setOrientation(toRp(rotation));
				rb.getBody()->setTransform(transform);
			}

			dynamics_world_->update(reactphysics3d::decimal(time_step));

			offset = 0;

			for (ReactCollisionBody& rb : collision_bodies_)
			{
				if (!rb.isRigidBody())
					continue;

				entity::Entity entity = rb.getEntity();
				reactphysics3d::Transform transform = rb.getBody()->getTransform();

				if (rb.getVelocityConstraints())
				{
					const uint8_t   lin_con = rb.getAngularConstraints();
					const glm::vec3 new_pos = toGlm(transform.getPosition());
					const glm::vec3 old_pos = sav_data[offset].position;
					const glm::vec3 com_pos = glm::vec3(
						(lin_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_pos.x : new_pos.x,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_pos.y : new_pos.y,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_pos.z : new_pos.z
					);
					const glm::vec3 new_vel = rb.getVelocity() * VIOLET_PHYSICS_SCALE;
					const glm::vec3 old_vel = sav_data[offset].velocity;
					const glm::vec3 com_vel = glm::vec3(
						(lin_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_vel.x : new_vel.x,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_vel.y : new_vel.y,
						(lin_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_vel.z : new_vel.z
					);

					transform.setPosition(toRp(com_pos));
					rb.getBody()->setTransform(transform);
					rb.setVelocity(com_vel);
				}
				if (rb.getAngularConstraints())
				{
					const uint8_t ang_con = rb.getAngularConstraints();
					const glm::vec3 new_rot = glm::eulerAngles(toGlm(transform.getOrientation()));
					const glm::vec3 old_rot = sav_data[offset].rotation;
					const glm::vec3 com_rot = glm::vec3(
						(ang_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_rot.x : new_rot.x,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_rot.y : new_rot.y,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_rot.z : new_rot.z
					);
					const glm::vec3 new_ang = rb.getAngularVelocity() * VIOLET_PHYSICS_SCALE;
					const glm::vec3 old_ang = sav_data[offset].angular;
					const glm::vec3 com_ang = glm::vec3(
						(ang_con & (uint8_t)components::RigidBodyConstraints::kX) ? old_ang.x : new_ang.x,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kY) ? old_ang.y : new_ang.y,
						(ang_con & (uint8_t)components::RigidBodyConstraints::kZ) ? old_ang.z : new_ang.z
					);

					transform.setOrientation(toRp(glm::quat(com_rot)));
					rb.getBody()->setTransform(transform);
					rb.setAngularVelocity(com_ang);
				}

				if (rb.getVelocityConstraints() || rb.getAngularConstraints())
					offset++;

				components::TransformSystem::setWorldTranslation(entity, toGlm(transform.getPosition()) * VIOLET_INV_PHYSICS_SCALE, *scene_);
				components::TransformSystem::setWorldRotation(entity, toGlm(transform.getOrientation()), *scene_);
			}

			Vector<Manifold> new_collisions;
			Vector<Manifold> stay_collisions;
			Vector<Manifold> end_collisions;
			event_listener_->endFrame(new_collisions, stay_collisions, end_collisions);

			for (const Manifold& manifold : new_collisions)
				components::MonoBehaviourSystem::onCollisionEnter(manifold.lhs, manifold.rhs, manifold.contacts[0].normal * manifold.contacts[0].depth, *scene_);
			//for (const Manifold& manifold : stay_collisions)
			//	components::MonoBehaviourSystem::onCollisionStay(manifold.lhs, manifold.rhs, manifold.contacts[0].normal * manifold.contacts[0].depth, *scene_);
			for (const Manifold& manifold : end_collisions)
				components::MonoBehaviourSystem::onCollisionExit(manifold.lhs, manifold.rhs, manifold.contacts[0].normal * manifold.contacts[0].depth, *scene_);
		}

		///////////////////////////////////////////////////////////////////////////
		class MyRaycastCallback : public reactphysics3d::RaycastCallback
		{
			virtual reactphysics3d::decimal notifyRaycastHit(
				const reactphysics3d::RaycastInfo& raycastInfo) override
			{
				if (isNaN(raycastInfo.worldPoint) || isNaN(raycastInfo.worldNormal))
					return reactphysics3d::decimal(0.0);

				Manifold manifold;
				manifold.rhs = *(entity::Entity*)raycastInfo.body->getUserData();
				manifold.num_contacts = 1;
				manifold.contacts[0].normal = toGlm(raycastInfo.worldNormal);
				manifold.contacts[0].point = toGlm(raycastInfo.worldPoint);
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
			reactphysics3d::Ray ray(toRp(start * VIOLET_PHYSICS_SCALE), toRp(end * VIOLET_PHYSICS_SCALE));
			MyRaycastCallback callback;
			dynamics_world_->raycast(ray, &callback);

			return callback.manifolds;
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactPhysicsWorld::createCollisionBody(entity::Entity entity)
		{
			size_t entry = scene_->collider.entity_to_data[entity];
			if (collision_bodies_.size() < entry + 1ull)
				collision_bodies_.resize(entry + 1ull);

			collision_bodies_[entry] = ReactCollisionBody(
				scene_,
				dynamics_world_,
				this,
				entity
			);
			collision_bodies_[entry].makeCollider();
		}

		///////////////////////////////////////////////////////////////////////////
		void ReactPhysicsWorld::destroyCollisionBody(entity::Entity entity)
		{
			size_t entry = scene_->collider.entity_to_data[entity];
			collision_bodies_[entry] = ReactCollisionBody();
		}

		///////////////////////////////////////////////////////////////////////////
		ICollisionBody& ReactPhysicsWorld::getCollisionBody(entity::Entity entity)
		{
			size_t entry = scene_->collider.entity_to_data[entity];
			return collision_bodies_[entry];
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
			dynamics_world_->setGravity(toRp(gravity * VIOLET_PHYSICS_SCALE));
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 physics::ReactPhysicsWorld::getGravity() const
		{
			return toGlm(dynamics_world_->getGravity()) * VIOLET_INV_PHYSICS_SCALE;
		}
		double ReactPhysicsWorld::getTimeStep() const
		{
			return time_step_;
		}
	}
}