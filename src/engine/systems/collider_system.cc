#include "collider_system.h"
#include "rigid_body_system.h"
#include "transform_system.h"
#include "interfaces/iworld.h"
#include "utils/mesh_decimator.h"

//#define DECIMATE
#ifdef DECIMATE
#include <mdMeshDecimator.h>
#endif

#include <btBulletDynamicsCommon.h>

namespace lambda
{
  namespace components
  {
    ColliderComponent ColliderSystem::addComponent(const entity::Entity& entity)
    {
      require(transform_system_.get(), entity);

			if (!unused_data_entries_.empty())
			{
				uint32_t idx = unused_data_entries_.front();
				unused_data_entries_.pop();

				data_[idx] = ColliderData(entity);
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}
			else
			{
				data_.push_back(ColliderData(entity));
				uint32_t idx = (uint32_t)data_.size() - 1u;
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}

      /** Init start */
      ColliderData& data = lookUpData(entity);
      
      glm::quat rotation = transform_system_->getWorldRotation(entity);
      glm::vec3 translation = transform_system_->getWorldTranslation(entity);
      btQuaternion bt_rotation(rotation.x, rotation.y, rotation.z, rotation.w);
      btVector3 bt_translation(translation.x, translation.y, translation.z);

      data.motion_state = foundation::Memory::construct<btDefaultMotionState>(btTransform(bt_rotation, bt_translation));
      btRigidBody::btRigidBodyConstructionInfo ground_rigid_body_ci(
        /*mass*/         0.0f,
        /*motion_state*/ data.motion_state,
        /*shape*/        data.collision_shape,
        /*inertia*/      btVector3(0.0f, 0.0f, 0.0f)
      );
      data.rigid_body = foundation::Memory::construct<btRigidBody>(ground_rigid_body_ci);
      data.rigid_body->setUserIndex((int)entity);
      data.rigid_body->setCollisionFlags(btRigidBody::CF_STATIC_OBJECT);
      /** Init end */

      makeBox(entity);

      rigid_body_system_->GetPhysicsWorld().dynamicsWorld()->addCollisionObject(data.rigid_body);

      return ColliderComponent(entity, this);
    }
    ColliderComponent ColliderSystem::getComponent(const entity::Entity& entity)
    {
      return ColliderComponent(entity, this);
    }
    bool ColliderSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void ColliderSystem::removeComponent(const entity::Entity& entity)
    {
      if (true == rigid_body_system_->hasComponent(entity))
        rigid_body_system_->removeComponent(entity);
			marked_for_delete_.insert(entity);
    }
		void ColliderSystem::collectGarbage()
		{
			if (!marked_for_delete_.empty())
			{
				for (entity::Entity entity : marked_for_delete_)
				{
					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						{
							ColliderData& data = data_.at(it->second);
							rigid_body_system_->GetPhysicsWorld().dynamicsWorld()->removeCollisionObject(data.rigid_body);
							foundation::Memory::destruct(data.collision_shape);
							foundation::Memory::destruct(data.motion_state);
							foundation::Memory::destruct(data.rigid_body);
						}

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
    void ColliderSystem::initialize(world::IWorld& world)
    {
      transform_system_  = world.getScene().getSystem<TransformSystem>();
      rigid_body_system_ = world.getScene().getSystem<RigidBodySystem>();
    }
    void ColliderSystem::deinitialize()
    {
      rigid_body_system_.reset();
      transform_system_.reset();
    }
    void ColliderSystem::make(const entity::Entity& entity, btCollisionShape* shape)
    {
      ColliderData& data = lookUpData(entity);

      if (data.collision_shape != nullptr)
      {
        foundation::Memory::destruct(data.collision_shape);
        data.collision_shape = nullptr;
      }

      data.collision_shape = shape;
      data.rigid_body->setCollisionShape(data.collision_shape);
    }
    void ColliderSystem::makeBox(const entity::Entity& entity)
    {
      glm::vec3 scale = transform_system_->getWorldScale(entity);
      btVector3 half_extends(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);
      btCollisionShape* shape = foundation::Memory::construct<btBoxShape>(half_extends);
      lookUpData(entity).type = ColliderType::kBox;
      make(entity, shape);
    }
    void ColliderSystem::makeSphere(const entity::Entity& entity)
    {
      glm::vec3 scale = transform_system_->getWorldScale(entity);
      btScalar radius = (scale.x + scale.z) / 4.0f;
      btCollisionShape* shape = foundation::Memory::construct<btSphereShape>(radius);
      lookUpData(entity).type = ColliderType::kSphere;
      make(entity, shape);
    }
    void ColliderSystem::makeCapsule(const entity::Entity& entity)
    {
      glm::vec3 scale = transform_system_->getWorldScale(entity);
      btScalar radius = (scale.x * 0.5f + scale.z * 0.5f) * 0.5f;
      btScalar height = scale.y * 0.5f;
      btCollisionShape* shape = foundation::Memory::construct<btCapsuleShape>(radius, height);
      lookUpData(entity).type = ColliderType::kCapsule;
      make(entity, shape);
    }

    void ColliderSystem::makeMeshCollider(const entity::Entity& entity, asset::MeshHandle mesh, const uint32_t& sub_mesh_id)
    {
      auto it_sub_mesh = mesh_colliders_[mesh.id].find(sub_mesh_id);
      if (it_sub_mesh == mesh_colliders_[mesh.id].end())
      {
        // Get the indices.
        asset::SubMesh sub_mesh = mesh->getSubMeshes().at(sub_mesh_id);
        auto index_offset  = sub_mesh.offsets[asset::MeshElements::kIndices];
        auto vertex_offset = sub_mesh.offsets[asset::MeshElements::kPositions];
        int* indices = (int*)foundation::Memory::allocate(index_offset.count * sizeof(int));
        glm::vec3* vertices = (glm::vec3*)foundation::Memory::allocate(vertex_offset.count * sizeof(glm::vec3));

        auto mii = mesh->get(asset::MeshElements::kIndices);
        auto mpi = mesh->get(asset::MeshElements::kPositions);

        memcpy(vertices, (char*)mpi.data + vertex_offset.offset, vertex_offset.count * mpi.size);
      
        if (sizeof(uint16_t) == mii.size)
        {
          Vector<uint16_t> idx(index_offset.count);
          memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

          for (size_t i = 0u; i < index_offset.count; ++i)
          {
            indices[i] = (int)idx.at(i);
          }
        }
        else
        {
          Vector<uint32_t> idx(index_offset.count);
          memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

          for (size_t i = 0u; i < index_offset.count; ++i)
          {
            indices[i] = (int)idx.at(i);
          }
        }

#ifdef DECIMATE
        // DECIMATE!
        MeshDecimation::MeshDecimator decimator;
        decimator.Initialize(vertex_offset.count, index_offset.count / 3u, (MeshDecimation::Vec3<float>*)vertices, (MeshDecimation::Vec3<int>*)indices);

        size_t desired_vertex_count = rangeDivider(vertex_offset.count);
        size_t desired_index_count  = rangeDivider(index_offset.count / 2u);
        decimator.Decimate(desired_vertex_count, desired_index_count, 1.0);

        size_t new_index_count = decimator.GetNTriangles() * 3u;
        int* new_indices = (int*)foundation::Memory::allocate(new_index_count * sizeof(int));
        size_t new_vertex_count = decimator.GetNVertices();
        glm::vec3* new_vertices = (glm::vec3*)foundation::Memory::allocate(new_vertex_count * sizeof(glm::vec3));
        decimator.GetMeshData((MeshDecimation::Vec3<float>*)new_vertices, (MeshDecimation::Vec3<int>*)new_indices);

        foundation::Memory::deallocate(vertices);
        foundation::Memory::deallocate(indices);

        // Generate tri-list
        btTriangleIndexVertexArray* index_vertex_array = foundation::Memory::construct<btTriangleIndexVertexArray>(
          new_index_count / 3u, // TODO (Hilze): Is this correct?
          new_indices,
          sizeof(int) * 3u,
        
          new_vertex_count * mpi.size,
          (float*)new_vertices,
          sizeof(glm::vec3)
        );
#else
        // Generate tri-list
        btTriangleIndexVertexArray* index_vertex_array = foundation::Memory::construct<btTriangleIndexVertexArray>(
          (int)(index_offset.count / 3u), 
          indices,
          (int)(sizeof(int) * 3u),
        
          (int)(vertex_offset.count * mpi.size),
          (float*)vertices,
          (int)sizeof(glm::vec3)
        );
#endif

        mesh_colliders_[mesh.id].insert(eastl::make_pair(sub_mesh_id, index_vertex_array));
        it_sub_mesh = mesh_colliders_[mesh.id].find(sub_mesh_id);
      }

      // Create the new shape and set it.
      btCollisionShape* shape = foundation::Memory::construct<btBvhTriangleMeshShape>(it_sub_mesh->second, false);
      glm::vec3 scale = transform_system_->getWorldScale(entity);
      shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
      lookUpData(entity).type = ColliderType::kMesh;
      make(entity, shape);
    }

	void ColliderSystem::setFriction(const entity::Entity& entity, float friction)
	{
		lookUpData(entity).rigid_body->setFriction(friction);
	}

	float ColliderSystem::getFriction(const entity::Entity& entity) const
	{
		return lookUpData(entity).rigid_body->getFriction();
	}

	void ColliderSystem::setMass(const entity::Entity& entity, float mass)
	{
		lookUpData(entity).rigid_body->setMassProps(
			mass,
			lookUpData(entity).rigid_body->getLocalInertia()
		);
	}

	float ColliderSystem::getMass(const entity::Entity& entity) const
	{
		return 1.0f / lookUpData(entity).rigid_body->getInvMass();
	}

	ColliderData& ColliderSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    const ColliderData& ColliderSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    ColliderComponent::ColliderComponent(const entity::Entity& entity, ColliderSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    ColliderComponent::ColliderComponent(const ColliderComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    ColliderComponent::ColliderComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    void ColliderComponent::makeBoxCollider()
    {
      system_->makeBox(entity_);
    }
    void ColliderComponent::makeSphereCollider()
    {
      system_->makeSphere(entity_);
    }
    void ColliderComponent::makeCapsuleCollider()
    {
      system_->makeCapsule(entity_);
    }
    void ColliderComponent::makeMeshCollider(asset::MeshHandle mesh, const uint32_t& sub_mesh_id)
    {
      system_->makeMeshCollider(entity_, mesh, sub_mesh_id);
    }

	void ColliderComponent::setFriction(float friction)
	{
		system_->setFriction(entity_, friction);
	}

	float ColliderComponent::getFriction() const
	{
		return system_->getFriction(entity_);
	}

	void ColliderComponent::setMass(float mass)
	{
		system_->setMass(entity_, mass);
	}

	float ColliderComponent::getMass() const
	{
		return system_->getMass(entity_);
	}

	ColliderData::ColliderData(const ColliderData& other)
    {
      type            = other.type;
      collision_shape = other.collision_shape;
      motion_state    = other.motion_state;
      rigid_body      = other.rigid_body;
      is_trigger      = other.is_trigger;
      entity          = other.entity;
			valid           = other.valid;
    }
    ColliderData & ColliderData::operator=(const ColliderData & other)
    {
      type            = other.type;
      collision_shape = other.collision_shape;
      motion_state    = other.motion_state;
      rigid_body      = other.rigid_body;
      is_trigger      = other.is_trigger;
      entity          = other.entity;
			valid           = other.valid;
			return *this;
    }
}
}