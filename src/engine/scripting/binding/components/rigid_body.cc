#include <scripting/binding/components/rigid_body.h>
#include <systems/rigid_body_system.h>
#include <systems/entity_system.h>
#include <interfaces/iworld.h>

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace scripting
  {
    namespace components
    {
      namespace rigidbody
      {
        lambda::components::RigidBodySystem* g_rigid_body_system;
        entity::EntitySystem* g_entity_system;

        void Create(const uint64_t& id)
        {
          g_rigid_body_system->addComponent(entity::Entity(id, g_entity_system));
        }
        void Destroy(const uint64_t& id)
        {
          g_rigid_body_system->removeComponent(entity::Entity(id, g_entity_system));
        }
        ScriptVec3 GetVelocity(const uint64_t& id)
        {
          return g_rigid_body_system->getVelocity(entity::Entity(id, g_entity_system));
        }
        void SetVelocity(const uint64_t& id, const ScriptVec3& v)
        {
          g_rigid_body_system->setVelocity(entity::Entity(id, g_entity_system), v);
        }
        ScriptVec3 GetAngularVelocity(const uint64_t& id)
        {
          return g_rigid_body_system->getAngularVelocity(entity::Entity(id, g_entity_system));
        }
        void SetAngularVelocity(const uint64_t& id, const ScriptVec3& v)
        {
          g_rigid_body_system->setAngularVelocity(entity::Entity(id, g_entity_system), v);
        }
        uint8_t GetVelocityConstraints(const uint64_t& id)
        {
          return g_rigid_body_system->getVelocityConstraints(entity::Entity(id, g_entity_system));
        }
        void SetVelocityConstraints(const uint64_t& id, const uint8_t& constraints)
        {
          g_rigid_body_system->setVelocityConstraints(entity::Entity(id, g_entity_system), constraints);
        }
        uint8_t GetAngularConstraints(const uint64_t& id)
        {
          return g_rigid_body_system->getAngularConstraints(entity::Entity(id, g_entity_system));
        }
        void SetAngularConstraints(const uint64_t& id, const uint8_t& constraints)
        {
          g_rigid_body_system->setAngularConstraints(entity::Entity(id, g_entity_system), constraints);
        }
        void ApplyImpulse(const uint64_t& id, const ScriptVec3& impulse)
        {
          g_rigid_body_system->applyImpulse(entity::Entity(id, g_entity_system), impulse);
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_rigid_body_system = world->getScene().getSystem<lambda::components::RigidBodySystem>().get();
          g_entity_system = world->getScene().getSystem<entity::EntitySystem>().get();

          return Map<lambda::String, void*>{
            { "void Violet_Components_RigidBody::Create(const uint64& in)", (void*)Create },
            { "void Violet_Components_RigidBody::Destroy(const uint64& in)", (void*)Destroy },
            { "void Violet_Components_RigidBody::SetVelocity(const uint64& in, const Vec3& in)", (void*)SetVelocity },
            { "Vec3 Violet_Components_RigidBody::GetVelocity(const uint64& in)", (void*)GetVelocity },
            { "void Violet_Components_RigidBody::SetAngularVelocity(const uint64& in, const Vec3& in)", (void*)SetAngularVelocity },
            { "Vec3 Violet_Components_RigidBody::GetAngularVelocity(const uint64& in)", (void*)GetAngularVelocity },
            { "void Violet_Components_RigidBody::SetVelocityConstraints(const uint64& in, const uint8& in)", (void*)SetVelocityConstraints },
            { "uint8 Violet_Components_RigidBody::GetVelocityConstraints(const uint64& in)", (void*)GetVelocityConstraints },
            { "void Violet_Components_RigidBody::SetAngularConstraints(const uint64& in, const uint8& in)", (void*)SetAngularConstraints },
            { "uint8 Violet_Components_RigidBody::GetAngularConstraints(const uint64& in)", (void*)GetAngularConstraints },
            { "void Violet_Components_RigidBody::ApplyImpulse(const uint64& in, const Vec3& in)", (void*)ApplyImpulse },
          };
        }
        
        void Unbind()
        {
          g_rigid_body_system = nullptr;
          g_entity_system = nullptr;
        }
      }
    }
  }
}
