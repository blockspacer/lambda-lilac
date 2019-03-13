#include "physics/newton/newton_physics_world.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include "systems/transform_system.h"
#include "systems/rigid_body_system.h"
#include "systems/entity_system.h"
#include "systems/mono_behaviour_system.h"
#include "interfaces/iworld.h"

#include <dgNewton/Newton.h>

namespace lambda
{
	namespace physics
	{
		///////////////////////////////////////////////////////////////////////////
		void* newtonAlloc(int sizeInBytes)
		{
			return foundation::Memory::allocate(sizeInBytes);
		}

		///////////////////////////////////////////////////////////////////////////
		void newtonFree(void* const ptr, int sizeInBytes)
		{
			foundation::Memory::deallocate(ptr);
		}
		 
		///////////////////////////////////////////////////////////////////////////
		NewtonCollisionBody::NewtonCollisionBody(
			world::IWorld* world,
			NewtonPhysicsWorld* physics_world,
			NewtonWorld* dynamics_world,
			entity::Entity entity)
			: physics_world_(physics_world)
			, dynamics_world_(dynamics_world)
			, world_(world)
			, type_(NewtonCollisionBodyType::kNone)
			, entity_(entity)
			, velocity_constraints_(0)
			, angular_constraints_(0)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		NewtonCollisionBody::~NewtonCollisionBody()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 NewtonCollisionBody::getPosition() const
		{
			return glm::vec3();
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setPosition(glm::vec3 position)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		glm::quat NewtonCollisionBody::getRotation() const
		{
			return glm::quat();
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setRotation(glm::quat rotation)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		entity::Entity NewtonCollisionBody::getEntity() const
		{
			return entity_;
		}

		///////////////////////////////////////////////////////////////////////////
		float NewtonCollisionBody::getFriction() const
		{
			return 0.0f;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setFriction(float friction)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		float NewtonCollisionBody::getMass() const
		{
			return 0.0f;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setMass(float mass)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		uint16_t NewtonCollisionBody::getLayers() const
		{
			return 0;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setLayers(uint16_t layers)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		uint8_t NewtonCollisionBody::getVelocityConstraints() const
		{
			return 0;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setVelocityConstraints(uint8_t velocity_constraints)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		uint8_t NewtonCollisionBody::getAngularConstraints() const
		{
			return 0;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setAngularConstraints(uint8_t angular_constraints)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 NewtonCollisionBody::getVelocity() const
		{
			return glm::vec3();
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setVelocity(glm::vec3 velocity)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		glm::vec3 NewtonCollisionBody::getAngularVelocity() const
		{
			return glm::vec3();
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::setAngularVelocity(glm::vec3 velocity)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::applyImpulse(glm::vec3 impulse)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::applyImpulse(glm::vec3 impulse, glm::vec3 location)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		bool NewtonCollisionBody::isCollider() const
		{
			return type_ == NewtonCollisionBodyType::kCollider;
		}

		///////////////////////////////////////////////////////////////////////////
		bool NewtonCollisionBody::isRigidBody() const
		{
			return type_ == NewtonCollisionBodyType::kRigidBody;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::makeBoxCollider()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::makeSphereCollider()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::makeCapsuleCollider()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::makeMeshCollider(asset::MeshHandle mesh, uint32_t sub_mesh_id)
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
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::makeCollider()
		{
			type_ = NewtonCollisionBodyType::kCollider;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::makeRigidBody()
		{
			type_ = NewtonCollisionBodyType::kRigidBody;
		}

		///////////////////////////////////////////////////////////////////////////
		void NewtonCollisionBody::destroyBody()
		{
			if (type_ != NewtonCollisionBodyType::kNone)
			{
			}
		}





    ///////////////////////////////////////////////////////////////////////////
    NewtonPhysicsWorld::NewtonPhysicsWorld()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    NewtonPhysicsWorld::~NewtonPhysicsWorld()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void NewtonPhysicsWorld::initialize(
      platform::DebugRenderer* debug_renderer, 
	  world::IWorld* world)
    {
			world_ = world;
			physics_visualizer_.initialize(debug_renderer, false);
#if defined(_DEBUG) || defined(DEBUG)
			physics_visualizer_.setDrawEnabled(true);
#endif


			NewtonSetMemorySystem(newtonAlloc, newtonFree);
			dynamics_world_ = NewtonCreate();
    }

    ///////////////////////////////////////////////////////////////////////////
    void NewtonPhysicsWorld::deinitialize()
    {
		NewtonDestroy(dynamics_world_);
		dynamics_world_ = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
		void NewtonPhysicsWorld::render()
		{
		}

    ///////////////////////////////////////////////////////////////////////////
		void NewtonPhysicsWorld::update(const double& time_step)
		{
			// TODO (Hilze): Move this to a more logical location ASAP!
			// This will decrease performance drastically.
			NewtonInvalidateCache(dynamics_world_);
			NewtonUpdate(dynamics_world_, (dFloat)time_step);
		}

    ///////////////////////////////////////////////////////////////////////////
    Vector<Manifold> NewtonPhysicsWorld::raycast(
      const glm::vec3& start, 
      const glm::vec3& end)
    {
		return Vector<Manifold>();
    }

	///////////////////////////////////////////////////////////////////////////
	ICollisionBody* NewtonPhysicsWorld::createCollisionBody(entity::Entity entity)
	{
		NewtonCollisionBody* rb = foundation::Memory::construct<NewtonCollisionBody>(
			world_,
			this,
			dynamics_world_,
			entity
		);
		rb->makeCollider();
		collision_bodies_.push_back(rb);

		return rb;
	}

		///////////////////////////////////////////////////////////////////////////
	void NewtonPhysicsWorld::destroyCollisionBody(ICollisionBody* collision_body)
	{
		auto it = eastl::find(collision_bodies_.begin(), collision_bodies_.end(), collision_body);
		if (it != collision_bodies_.end())
			collision_bodies_.erase(it);

		foundation::Memory::destruct(collision_body);
	}

	///////////////////////////////////////////////////////////////////////////
	void physics::NewtonPhysicsWorld::setDebugDrawEnabled(bool debug_draw_enabled)
	{
		physics_visualizer_.setDrawEnabled(debug_draw_enabled);
	}

	///////////////////////////////////////////////////////////////////////////
	bool physics::NewtonPhysicsWorld::getDebugDrawEnabled() const
	{
		return physics_visualizer_.getDrawEnabled();
	}

	///////////////////////////////////////////////////////////////////////////
	void physics::NewtonPhysicsWorld::setGravity(glm::vec3 gravity)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	glm::vec3 physics::NewtonPhysicsWorld::getGravity() const
	{
		return glm::vec3();
    }
  }
}
