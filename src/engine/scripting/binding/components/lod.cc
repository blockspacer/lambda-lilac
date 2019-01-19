#include <scripting/binding/components/lod.h>
#include <scripting/binding/assets/mesh.h>
#include <systems/lod_system.h>
#include <systems/mesh_render_system.h>
#include <systems/transform_system.h>
#include <systems/entity_system.h>
#include <interfaces/iworld.h>

namespace lambda
{
  namespace scripting
  {
    namespace components
    {
      namespace lod
      {
        lambda::components::LODSystem* g_lod_system;
        lambda::components::TransformSystem* g_transform_system;
        lambda::components::MeshRenderSystem* g_mesh_render_system;
        entity::EntitySystem* g_entity_system;

        void Create(const uint64_t& id)
        {
          g_lod_system->addComponent((entity::Entity)id);
        }
        void Destroy(const uint64_t& id)
        {
          g_lod_system->removeComponent((entity::Entity)id);
        }
        void AddLOD(const uint64_t& id, const uint64_t& mesh_id, const float& distance)
        {
          lambda::components::LOD lod;
          lod.setDistance(distance);
          lod.setMesh(assets::mesh::Get(mesh_id));

          g_lod_system->addLOD((entity::Entity)id, lod);
        }
        void AddLOD_(entity::Entity entity, const lambda::components::LOD& lod, const float& distance)
        {
          if (g_mesh_render_system->hasComponent(entity) && 
            g_mesh_render_system->getMesh(entity) != nullptr)
          {
            if (false == g_lod_system->hasComponent(entity))
              g_lod_system->addComponent(entity);

            g_lod_system->addLOD(entity, lod);
          }

          for (const auto& child : g_transform_system->getChildren(entity))
            AddLOD_(child, lod, distance);
        }
        void AddLODRecursive(const uint64_t& id, const uint64_t& mesh_id, const float& distance)
        {
          lambda::components::LOD lod;
          lod.setDistance(distance);
          lod.setMesh(assets::mesh::Get(mesh_id));

          AddLOD_((entity::Entity)id, lod, distance);
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_lod_system = world->getScene().getSystem<lambda::components::LODSystem>().get();
          g_transform_system = world->getScene().getSystem<lambda::components::TransformSystem>().get();
          g_mesh_render_system = world->getScene().getSystem<lambda::components::MeshRenderSystem>().get();
          g_entity_system = world->getScene().getSystem<entity::EntitySystem>().get();

          return Map<lambda::String, void*>{
            { "void Violet_Components_LOD::Create(const uint64& in)",                                             (void*)Create },
            { "void Violet_Components_LOD::Destroy(const uint64& in)",                                            (void*)Destroy },
            { "void Violet_Components_LOD::AddLOD(const uint64& in, const uint64& in, const float& in)",          (void*)AddLOD },
            { "void Violet_Components_LOD::AddLODRecursive(const uint64& in, const uint64& in, const float& in)", (void*)AddLODRecursive }
          };
        }

        void Unbind()
        {
          g_lod_system = nullptr;
          g_transform_system = nullptr;
          g_mesh_render_system = nullptr;
          g_entity_system = nullptr;
        }
      }
    }
  }
}
