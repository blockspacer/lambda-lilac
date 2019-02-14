#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include <containers/containers.h>
#include <memory/memory.h>
#include "assets/mesh.h"

class btCollisionShape;
struct btDefaultMotionState;
class btRigidBody;
class btTriangleIndexVertexArray;

namespace lambda
{
  namespace components
  {
    class ColliderSystem;
    class RigidBodySystem;
    class TransformSystem;

    enum class ColliderType : uint8_t
    {
      kBox     = 0,
      kSphere  = 1,
      kCapsule = 2,
      kMesh    = 3
    };

    class ColliderComponent : public IComponent
    {
    public:
      ColliderComponent(const entity::Entity& entity, ColliderSystem* system);
      ColliderComponent(const ColliderComponent& other);
      ColliderComponent();

      void makeBoxCollider();
      void makeSphereCollider();
      void makeCapsuleCollider();
      void makeMeshCollider(asset::MeshHandle mesh, const uint32_t& sub_mesh_id);

    private:
      ColliderSystem* system_;
    };

    struct ColliderData
    {
      ColliderData(const entity::Entity& entity) : entity(entity) {};
      ColliderData(const ColliderData& other);
      ColliderData& operator=(const ColliderData& other);

      ColliderType          type = ColliderType::kCapsule; // TODO (Hilze): Remove this
      btCollisionShape*     collision_shape = nullptr;
      btDefaultMotionState* motion_state = nullptr;
      btRigidBody*          rigid_body = nullptr;
      bool                  is_trigger = false;
			bool                  valid = true;

      entity::Entity entity;
    };

    class ColliderSystem : public ISystem
    {
    public:
      friend class RigidBodySystem;
    public:
      static size_t systemId() { return (size_t)SystemIds::kColliderSystem; };
      ColliderComponent addComponent(const entity::Entity& entity);
      ColliderComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
			virtual void collectGarbage() override;
			virtual ~ColliderSystem() override {};
      void make(const entity::Entity& entity, btCollisionShape* shape);
      void makeBox(const entity::Entity& entity);
      void makeSphere(const entity::Entity& entity);
      void makeCapsule(const entity::Entity& entity);
      void makeMeshCollider(const entity::Entity& entity, asset::MeshHandle mesh, const uint32_t& sub_mesh_id);
      
    protected:
      ColliderData& lookUpData(const entity::Entity& entity);
      const ColliderData& lookUpData(const entity::Entity& entity) const;

    private:
      Vector<ColliderData> data_;
			Map<entity::Entity, uint32_t> entity_to_data_;
			Map<uint32_t, entity::Entity> data_to_entity_;
			Set<entity::Entity> marked_for_delete_;
			Queue<uint32_t> unused_data_entries_;

      Map<size_t, Map<uint64_t, btTriangleIndexVertexArray*>> mesh_colliders_;

      foundation::SharedPointer<TransformSystem> transform_system_;
      foundation::SharedPointer<RigidBodySystem> rigid_body_system_;
    };
  }
}