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

		////////////////////////////////////////////////////////////////////////////
		void Culler::cullDynamics(const BaseBVH& bvh, const Frustum& frustum)
		{
			dynamic_.data = nullptr;
			dynamic_.next = nullptr;
			dynamic_.previous = nullptr;
			LinkedNode* node_it = &dynamic_;

			BVHAABB aabb(frustum.getMin(), frustum.getMax());

			for (const void* user_data : bvh.getAllUserDataInAABB(aabb))
			{
				Renderable* data = (Renderable*)user_data;
				if (data->mesh && frustum.ContainsAABB(data->min, data->max))
				{
					/*Renderable* renderable = foundation::GetFrameHeap()->construct<Renderable>();
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
					renderable->model_matrix     = data->model_matrix;*/

					LinkedNode* node = foundation::GetFrameHeap()->construct<LinkedNode>();
					node_it->next = node;
					node->data = data;
					node->previous = node_it;
					node_it = node;
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		void Culler::cullStatics(const BaseBVH& bvh, const Frustum& frustum)
		{
			static_.data = nullptr;
			static_.next = nullptr;
			static_.previous = nullptr;
			LinkedNode* node_it = &static_;

			BVHAABB aabb(frustum.getMin(), frustum.getMax());

			for (const void* user_data : bvh.getAllUserDataInAABB(aabb))
			{
				Renderable* data = (Renderable*)user_data;
				if (data->mesh && frustum.ContainsAABB(data->min, data->max))
				{
					/*Renderable* renderable = foundation::GetFrameHeap()->construct<Renderable>();
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
					renderable->model_matrix     = data->model_matrix;*/

					LinkedNode* node = foundation::GetFrameHeap()->construct<LinkedNode>();
					node_it->next = node;
					node->data = data;
					node->previous = node_it;
					node_it = node;
				}
			}
		}
	}
}