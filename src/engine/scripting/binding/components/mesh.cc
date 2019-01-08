#include <scripting/binding/components/mesh.h>
#include <scripting/script_vector.h>
#include <systems/transform_system.h>
#include <systems/mesh_render_system.h>
#include <systems/entity_system.h>
#include <interfaces/iworld.h>
#include <platform/scene.h>
#include <scripting/binding/assets/mesh.h>
#include <scripting/binding/assets/texture.h>

namespace lambda
{
  namespace scripting
  {
    namespace components
    {
      namespace meshrender
      {
        lambda::components::TransformSystem* g_transform_system = nullptr;
        lambda::components::MeshRenderSystem* g_mesh_render_system = nullptr;
        entity::EntitySystem* g_entity_system = nullptr;

        void Create(const uint64_t& id)
        {
          g_mesh_render_system->addComponent(entity::Entity(id, g_entity_system));
        }
        void Destroy(const uint64_t& id)
        {
          g_mesh_render_system->removeComponent(entity::Entity(id, g_entity_system));
        }
        void Attach(const uint64_t& id, const uint64_t& mesh_id)
        {
          g_mesh_render_system->getComponent(entity::Entity(id, g_entity_system)).attachMesh(assets::mesh::Get(mesh_id));
        }
        void SetMesh(const uint64_t& id, const uint64_t& mesh_id)
        {
          g_mesh_render_system->setMesh(entity::Entity(id, g_entity_system), assets::mesh::Get(mesh_id));
        }
        void SetSubMesh(const uint64_t& id, const uint16_t& sub_mesh)
        {
          g_mesh_render_system->setSubMesh(entity::Entity(id, g_entity_system), (uint32_t)sub_mesh);
        }
        void SetAlbedoTexture(const uint64_t& id, const uint64_t& texture)
        {
          g_mesh_render_system->setAlbedoTexture(entity::Entity(id, g_entity_system), assets::texture::Get(texture));
        }
        void SetNormalTexture(const uint64_t& id, const uint64_t& texture)
        {
          g_mesh_render_system->setNormalTexture(entity::Entity(id, g_entity_system), assets::texture::Get(texture));
        }
        void SetMetallicRoughnessTexture(const uint64_t& id, const uint64_t& texture)
        {
          g_mesh_render_system->setMetallicRoughnessTexture(entity::Entity(id, g_entity_system), assets::texture::Get(texture));
        }
        void MakeStatic(const uint64_t& id)
        {
          g_mesh_render_system->makeStatic(entity::Entity(id, g_entity_system));
        }
        void MakeStaticRecursive(const uint64_t& id)
        {
          const entity::Entity e(id, g_entity_system);
          if (g_mesh_render_system->hasComponent(e))
            g_mesh_render_system->makeStatic(e);

          if (g_transform_system->hasComponent(e))
            for (const auto& child : g_transform_system->getChildren(e))
              MakeStaticRecursive(child.id());
        }
        void MakeDynamic(const uint64_t& id)
        {
          g_mesh_render_system->makeDynamic(entity::Entity(id, g_entity_system));
        }
        void MakeDynamicRecursive(const uint64_t& id)
        {
          const entity::Entity e(id, g_entity_system);
          if (g_mesh_render_system->hasComponent(e))
            g_mesh_render_system->makeDynamic(e);

          if (g_transform_system->hasComponent(e))
            for (const auto& child : g_transform_system->getChildren(e))
              MakeDynamicRecursive(child.id());
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_entity_system = world->getScene().getSystem<entity::EntitySystem>().get();
          g_mesh_render_system = world->getScene().getSystem<lambda::components::MeshRenderSystem>().get();
          g_transform_system = world->getScene().getSystem<lambda::components::TransformSystem>().get();
          
          return Map<lambda::String, void*> {
            { "void Violet_Components_MeshRender::Create(const uint64& in)",                                        (void*)Create },
            { "void Violet_Components_MeshRender::Destroy(const uint64& in)",                                       (void*)Destroy },
            { "void Violet_Components_MeshRender::Attach(const uint64& in, const uint64& in)",                      (void*)Attach },
            { "void Violet_Components_MeshRender::SetMesh(const uint64& in, const uint64& in)",                     (void*)SetMesh },
            { "void Violet_Components_MeshRender::SetSubMesh(const uint64& in, const uint16& in)",                  (void*)SetSubMesh },
            { "void Violet_Components_MeshRender::SetAlbedoTexture(const uint64& in, const uint64& in)",            (void*)SetAlbedoTexture },
            { "void Violet_Components_MeshRender::SetNormalTexture(const uint64& in, const uint64& in)",            (void*)SetNormalTexture },
            { "void Violet_Components_MeshRender::SetMetallicRoughnessTexture(const uint64& in, const uint64& in)", (void*)SetMetallicRoughnessTexture },
            { "void Violet_Components_MeshRender::MakeStatic(const uint64& in)",                                    (void*)MakeStatic },
            { "void Violet_Components_MeshRender::MakeStaticRecursive(const uint64& in)",                           (void*)MakeStaticRecursive },
            { "void Violet_Components_MeshRender::MakeDynamic(const uint64& in)",                                   (void*)MakeDynamic },
            { "void Violet_Components_MeshRender::MakeDynamicRecursive(const uint64& in)",                          (void*)MakeDynamicRecursive }
          };
        }

        extern void Unbind()
        {
          g_entity_system = nullptr;
          g_mesh_render_system = nullptr;
          g_transform_system = nullptr;
        }
      }
    }
  }
}
