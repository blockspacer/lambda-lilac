#include <scripting/binding/components/collider.h>
#include <scripting/binding/assets/mesh.h>
#include <systems/collider_system.h>
#include <systems/transform_system.h>
#include <systems/mesh_render_system.h>
#include <systems/entity_system.h>
#include <interfaces/iworld.h>
#include <platform/scene.h>

namespace lambda
{
  namespace scripting
  {
    namespace components
    {
      namespace collider
      {
        lambda::components::TransformSystem* g_transform_system = nullptr;
        lambda::components::ColliderSystem* g_collider_system = nullptr;
        lambda::components::MeshRenderSystem* g_mesh_render_system = nullptr;
        entity::EntitySystem* g_entity_system = nullptr;

        void Create(const uint64_t& id)
        {
          g_collider_system->addComponent((entity::Entity)id);
        }
        void Destroy(const uint64_t& id)
        {
          g_collider_system->removeComponent((entity::Entity)id);
        }
        void AddMeshCollider(entity::Entity entity, asset::MeshHandle mesh)
        {
          if (true == g_mesh_render_system->hasComponent(entity) &&
            mesh == g_mesh_render_system->getMesh(entity) &&
            mesh->getSubMeshes().at(g_mesh_render_system->getSubMesh(entity)).offsets[asset::MeshElements::kPositions].count > 0u)
          {
            if (false == (g_mesh_render_system->getAlbedoTexture(entity) != nullptr && 
              g_mesh_render_system->getAlbedoTexture(entity)->getLayer(0u).containsAlpha()))
            {
              if (false == g_collider_system->hasComponent(entity))
                g_collider_system->addComponent(entity);

              g_collider_system->makeMeshCollider(entity, mesh, g_mesh_render_system->getSubMesh(entity));
            }
          }

          for (const auto& child : g_transform_system->getChildren(entity))
            AddMeshCollider(child, mesh);
        }
        void MakeBoxCollider(const uint64_t& id)
        {
          g_collider_system->makeBox((uint32_t)id);
        }
        void MakeSphereCollider(const uint64_t& id)
        {
          g_collider_system->makeSphere((uint32_t)id);
        }
        void MakeCapsuleCollider(const uint64_t& id)
        {
          g_collider_system->makeCapsule((uint32_t)id);
        }
        void MakeMeshCollider(const uint64_t& id, const uint64_t& mesh_id)
        {
          AddMeshCollider((uint32_t)id, assets::mesh::Get(mesh_id));
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_transform_system = world->getScene().getSystem<lambda::components::TransformSystem>().get();
          g_collider_system = world->getScene().getSystem<lambda::components::ColliderSystem>().get();
          g_mesh_render_system = world->getScene().getSystem<lambda::components::MeshRenderSystem>().get();
          g_entity_system = world->getScene().getSystem<entity::EntitySystem>().get();

          return Map<lambda::String, void*>{
            { "void Violet_Components_Collider::Create(const uint64& in)",                             (void*)collider::Create },
            { "void Violet_Components_Collider::Destroy(const uint64& in)",                            (void*)collider::Destroy },
            { "void Violet_Components_Collider::MakeBoxCollider(const uint64& in)",                    (void*)collider::MakeBoxCollider },
            { "void Violet_Components_Collider::MakeSphereCollider(const uint64& in)",                 (void*)collider::MakeSphereCollider },
            { "void Violet_Components_Collider::MakeCapsuleCollider(const uint64& in)",                (void*)collider::MakeCapsuleCollider },
            { "void Violet_Components_Collider::MakeMeshCollider(const uint64& in, const uint64& in)", (void*)collider::MakeMeshCollider }
          };
        }

        void Unbind()
        {
          g_transform_system = nullptr;
          g_collider_system = nullptr;
          g_mesh_render_system = nullptr;
          g_entity_system = nullptr;
        }
      }
    }
  }
}
