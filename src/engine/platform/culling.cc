#include "culling.h"
#include "systems/transform_system.h"
#include <glm/gtx/norm.hpp>
#include "systems/mesh_render_system.h"
#include "utils/zone_manager.h"
#include "utils/renderable.h"
#include <memory/frame_heap.h>
#include "frustum.h"
#include <algorithm>
#include <platform/scene.h>

namespace lambda
{
  namespace utilities
  {
    ///////////////////////////////////////////////////////////////////////////
    Culler::Culler()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    Culler::~Culler()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void Culler::setShouldCull(const bool& should_cull)
    {
      cull_ = should_cull;
    }

    ///////////////////////////////////////////////////////////////////////////
    void Culler::setCullShadowCasters(const bool& cull_shadow_casters)
    {
      cull_shadow_casters_ = cull_shadow_casters;
    }

    ///////////////////////////////////////////////////////////////////////////
    void Culler::setCullFrequency(const uint8_t& cull_frequency)
    {
      cull_frequency_ = cull_frequency;
    }

    ///////////////////////////////////////////////////////////////////////////
    void Culler::cullDynamics(scene::Scene& scene, const Frustum& frustum)
    {
      dynamic_.data = nullptr;
      dynamic_.next = nullptr;
      dynamic_.previous = nullptr;

      CullData cull_data;
      CullType cull_type = CullType::kAABB;
      LinkedNode* node_it = &dynamic_;
      
      for (const entity::Entity& entity : scene.mesh_render.dynamic_renderables)
      {
		  const components::MeshRenderSystem::Data* data =
			  &scene.mesh_render.get(entity);
		  
        bool visible = true;

        // If not culling, at least remove invisible or invalid objects.
        if (false == cull_)
        {
          visible = (data->mesh && data->visible);
        }
        else if (cull_shadow_casters_ && !data->cast_shadows)
          // TODO (Hilze): Fix this hack!
        {
          visible = false;
        }
        else
        {
          visible = (data->mesh && data->visible);
        }

        if (visible)
        {
          const asset::SubMesh& sub_mesh = 
            (data->sub_mesh < data->mesh->getSubMeshes().size()) ?
            data->mesh->getSubMeshes()[data->sub_mesh] : asset::SubMesh();
          const glm::vec3 position = 
            components::TransformSystem::getWorldTranslation(data->entity, scene);
          const glm::vec3 scale    = 
			  components::TransformSystem::getWorldScale(data->entity, scene);
          const float r_min2       = glm::length2(sub_mesh.min);
          const float r_max2       = glm::length2(sub_mesh.max);

          if (cull_type == CullType::kSphere)
          {
            cull_data.sphere.position = position;
            cull_data.sphere.radius   = 
              sqrtf(r_min2 > r_max2 ? r_min2 : r_max2) * glm::length(scale);
          }
          else
          {
            const glm::mat4 world = components::TransformSystem::getWorld(data->entity, scene);

            const glm::vec3 sm_min = world * glm::vec4(sub_mesh.min * 2.0f, 1.0f);
            const glm::vec3 sm_max = world * glm::vec4(sub_mesh.max * 2.0f, 1.0f);

            cull_data.aabb.min = glm::vec3(
              std::fminf(sm_min.x, sm_max.x),
              std::fminf(sm_min.y, sm_max.y),
              std::fminf(sm_min.z, sm_max.z)
            );
            cull_data.aabb.max = glm::vec3(
              std::fmaxf(sm_min.x, sm_max.x),
              std::fmaxf(sm_min.y, sm_max.y),
              std::fmaxf(sm_min.z, sm_max.z)
            );
          }

          frustum.Contains(&cull_data, 1u, cull_type);
          if (cull_data.visible)
          {
            Renderable* renderable = foundation::GetFrameHeap()->construct<Renderable>();
            renderable->entity           = data->entity;
            renderable->mesh             = data->mesh;
            renderable->sub_mesh         = data->sub_mesh;
            renderable->albedo_texture   = data->albedo_texture;
						renderable->normal_texture   = data->normal_texture;
						renderable->dmra_texture     = data->dmra_texture;
						renderable->emissive_texture = data->emissive_texture;
						renderable->metallicness     = data->metallicness;
            renderable->roughness        = data->roughness;
            renderable->emissiveness     = data->emissiveness;
            renderable->model_matrix     = components::TransformSystem::getWorld(data->entity, scene);
            
						LinkedNode* node = foundation::GetFrameHeap()->construct<LinkedNode>();
            node_it->next  = node;
            node->data     = renderable;
            node->previous = node_it;
            node_it = node;
          }
        }
        data++;
      }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    void Culler::cullStatics(
      ZoneManager& zone_manager, 
      const Frustum& frustum)
    {
      static_.data = nullptr;
      static_.next = nullptr;
      static_.previous = nullptr;
      LinkedNode* node_it = &static_;

      for (const utilities::Token& token : zone_manager.getTokens(frustum))
      {
        Renderable* data = (Renderable*)token.getUserData();
        if (data->mesh && frustum.ContainsAABB(data->min, data->max))
        {
          Renderable* renderable = foundation::GetFrameHeap()->construct<Renderable>();
          renderable->entity           = data->entity;
          renderable->mesh             = data->mesh;
          renderable->sub_mesh         = data->sub_mesh;
          renderable->albedo_texture   = data->albedo_texture;
					renderable->normal_texture   = data->normal_texture;
					renderable->dmra_texture     = data->dmra_texture;
					renderable->emissive_texture = data->emissive_texture;
					renderable->metallicness     = data->metallicness;
          renderable->roughness        = data->roughness;
          renderable->emissiveness     = data->emissiveness;
          renderable->model_matrix     = data->model_matrix;
            
					LinkedNode* node = foundation::GetFrameHeap()->construct<LinkedNode>();
          node_it->next  = node;
          node->data     = renderable;
          node->previous = node_it;
          node_it = node;
        }
      }
    }
  }
}