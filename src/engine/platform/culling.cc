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
			memset(&dynamic_, 0, sizeof(dynamic_));
			LinkedNode* node_it = &dynamic_;

			BVHAABB aabb(frustum.getMin(), frustum.getMax());

			for (const void* user_data : bvh.getAllUserDataInFrustum(frustum))
			{
				Renderable* data = (Renderable*)user_data;
				if (data->mesh)
				{
					LinkedNode* node = foundation::GetFrameHeap()->construct<LinkedNode>();
					node_it->next  = node;
					node->data     = data;
					node->previous = node_it;
					node_it        = node;
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		void Culler::cullStatics(const BaseBVH& bvh, const Frustum& frustum)
		{
			memset(&static_, 0, sizeof(static_));
			LinkedNode* node_it = &static_;

			BVHAABB aabb(frustum.getMin(), frustum.getMax());

			for (const void* user_data : bvh.getAllUserDataInFrustum(frustum))
			{
				Renderable* data = (Renderable*)user_data;
				if (data->mesh)
				{
					LinkedNode* node = foundation::GetFrameHeap()->construct<LinkedNode>();
					node_it->next  = node;
					node->data     = data;
					node->previous = node_it;
					node_it        = node;
				}
			}
		}
	}
}