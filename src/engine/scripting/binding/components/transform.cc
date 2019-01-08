#include <scripting/binding/components/transform.h>
#include <scripting/script_vector.h>
#include <systems/transform_system.h>
#include <systems/entity_system.h>
#include <interfaces/iworld.h>
#include <platform/scene.h>

namespace lambda
{
  namespace scripting
  {
    namespace components
    {
      namespace transform
      {
        lambda::components::TransformSystem* g_transform_system = nullptr;
        entity::EntitySystem* g_entity_system = nullptr;

        //// Construction ///////////////////////////////////////////////////////////////////////////
        void Create(const uint64_t& id)
        {
          g_transform_system->addComponent(entity::Entity(id, g_entity_system));
        }
        void Destroy(const uint64_t& id)
        {
          g_transform_system->removeComponent(entity::Entity(id, g_entity_system));
        }
        //// Translation ////////////////////////////////////////////////////////////////////////////
        void SetWorldPosition(const uint64_t& id, const ScriptVec3& v)
        {
          g_transform_system->setWorldTranslation(entity::Entity(id, g_entity_system), v);
        }
        ScriptVec3 GetWorldPosition(const uint64_t& id)
        {
          return g_transform_system->getWorldTranslation(entity::Entity(id, g_entity_system));
        }
        void SetLocalPosition(const uint64_t& id, const ScriptVec3& v)
        {
          g_transform_system->setLocalTranslation(entity::Entity(id, g_entity_system), v);
        }
        ScriptVec3 GetLocalPosition(const uint64_t& id)
        {
          return g_transform_system->getLocalTranslation(entity::Entity(id, g_entity_system));
        }
        //// Rotation ///////////////////////////////////////////////////////////////////////////////
        void SetWorldRotation(const uint64_t& id, const ScriptQuat& r)
        {
          g_transform_system->getComponent(entity::Entity(id, g_entity_system)).setWorldRotation(r);
        }
        ScriptQuat GetWorldRotation(const uint64_t& id)
        {
          return g_transform_system->getWorldRotation(entity::Entity(id, g_entity_system));
        }
        void SetWorldRotationEuler(const uint64_t& id, const ScriptVec3& r)
        {
          g_transform_system->getComponent(entity::Entity(id, g_entity_system)).setWorldRotation(r);
        }
        ScriptVec3 GetWorldRotationEuler(const uint64_t& id)
        {
          return glm::eulerAngles(g_transform_system->getWorldRotation(entity::Entity(id, g_entity_system)));
        }
        void SetLocalRotation(const uint64_t& id, const ScriptQuat& q)
        {
          g_transform_system->setLocalRotation(entity::Entity(id, g_entity_system), q);
        }
        ScriptQuat GetLocalRotation(const uint64_t& id)
        {
          return g_transform_system->getLocalRotation(entity::Entity(id, g_entity_system));
        }
        void SetLocalRotationEuler(const uint64_t& id, const ScriptVec3& r)
        {
          g_transform_system->setLocalRotation(entity::Entity(id, g_entity_system), r);
        }
        ScriptVec3 GetLocalRotationEuler(const uint64_t& id)
        {
          return glm::eulerAngles(g_transform_system->getLocalRotation(entity::Entity(id, g_entity_system)));
        }
        //// Scale //////////////////////////////////////////////////////////////////////////////////
        void SetWorldScale(const uint64_t& id, const ScriptVec3& s)
        {
          g_transform_system->setWorldScale(entity::Entity(id, g_entity_system), s);
        }
        ScriptVec3 GetWorldScale(const uint64_t& id)
        {
          return g_transform_system->getWorldScale(entity::Entity(id, g_entity_system));
        }
        void SetLocalScale(const uint64_t& id, const ScriptVec3& s)
        {
          g_transform_system->setLocalScale(entity::Entity(id, g_entity_system), s);
        }
        ScriptVec3 GetLocalScale(const uint64_t& id)
        {
          return g_transform_system->getLocalScale(entity::Entity(id, g_entity_system));
        }
        //// Parenting //////////////////////////////////////////////////////////////////////////////
        void SetParent(const uint64_t& id, const uint64_t& parent_id)
          {
            g_transform_system->setParent(entity::Entity(id, g_entity_system), entity::Entity(parent_id, g_entity_system));
          }

        lambda::Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_transform_system = world->getScene().getSystem<lambda::components::TransformSystem>().get();
          g_entity_system    = world->getScene().getSystem<lambda::entity::EntitySystem>().get();

          return lambda::Map<lambda::String, void*>{
            { "void Violet_Components_Transform::Create(const uint64&in)",                                (void*)Create },
            { "void Violet_Components_Transform::Destroy(const uint64&in)",                               (void*)Destroy },
            { "void Violet_Components_Transform::SetWorldPosition(const uint64&in, const Vec3&in)",       (void*)SetWorldPosition },
            { "Vec3 Violet_Components_Transform::GetWorldPosition(const uint64&in)",                      (void*)GetWorldPosition },
            { "void Violet_Components_Transform::SetLocalPosition(const uint64&in, const Vec3&in)",       (void*)SetLocalPosition },
            { "Vec3 Violet_Components_Transform::GetLocalPosition(const uint64&in)",                      (void*)GetLocalPosition },
            { "void Violet_Components_Transform::SetWorldRotation(const uint64&in, const Quat&in)",       (void*)SetWorldRotation },
            { "Quat Violet_Components_Transform::GetWorldRotation(const uint64&in)",                      (void*)GetWorldRotation },
            { "void Violet_Components_Transform::SetWorldRotationEuler(const uint64&in, const Vec3&in)",  (void*)SetWorldRotationEuler },
            { "Vec3 Violet_Components_Transform::GetWorldRotationEuler(const uint64&in)",                 (void*)GetWorldRotationEuler },
            { "void Violet_Components_Transform::SetLocalRotation(const uint64&in, const Quat&in)",       (void*)SetLocalRotation },
            { "Quat Violet_Components_Transform::GetLocalRotation(const uint64&in)",                      (void*)GetLocalRotation },
            { "void Violet_Components_Transform::SetLocalRotationEuler(const uint64&in, const Vec3&in)",  (void*)SetLocalRotationEuler },
            { "Vec3 Violet_Components_Transform::GetLocalRotationEuler(const uint64&in)",                 (void*)GetLocalRotationEuler },
            { "void Violet_Components_Transform::SetWorldScale(const uint64&in, const Vec3&in)",          (void*)SetWorldScale },
            { "Vec3 Violet_Components_Transform::GetWorldScale(const uint64&in)",                         (void*)GetWorldScale },
            { "void Violet_Components_Transform::SetLocalScale(const uint64&in, const Vec3&in)",          (void*)SetLocalScale },
            { "Vec3 Violet_Components_Transform::GetLocalScale(const uint64&in)",                         (void*)GetLocalScale },
            { "void Violet_Components_Transform::SetParent(const uint64&in, const uint64&in)",            (void*)SetParent },
          };
        }

        void Unbind()
        {
          g_transform_system = nullptr;
          g_entity_system    = nullptr;
        }
      }
    }
  }
}
