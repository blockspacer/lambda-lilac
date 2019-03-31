#pragma once
#include "platform/rasterizer_state.h"
#include "platform/blend_state.h"
#include "platform/sampler_state.h"
#include "platform/depth_stencil_state.h"
#include <containers/containers.h>
#include "assets/mesh.h"
#include "vulkan.h"

namespace lambda
{
  namespace linux
  {
	  class VulkanRenderer;

		struct ColorBlendState
		{
			VezColorBlendState blend_state;
			VezColorBlendAttachmentState attachments[8];
		};

    ///////////////////////////////////////////////////////////////////////////
    class VulkanStateManager
    {
    public:
      void initialize(VulkanRenderer* renderer);
			void update(Vector<VulkanReflectionInfo> samplers, uint32_t attachment_count);
			void bindRasterizerState(
        const platform::RasterizerState& rasterizer_state
      );
      void bindBlendState(const platform::BlendState& blend_state);
      void bindDepthStencilState(
        const platform::DepthStencilState& depth_stencil_state
      );
      void bindSamplerState(
        const platform::SamplerState& sampler_state, 
        unsigned char slots
      );
      void bindTopology(const asset::Topology& topology);

    private:
      UnorderedMap<platform::RasterizerState, 
        VezRasterizationState> rasterizer_states_;
      UnorderedMap<platform::BlendState, 
				ColorBlendState> blend_states_;
      UnorderedMap<platform::SamplerState, 
        VkSampler> sampler_states_;
			UnorderedMap<platform::DepthStencilState,
				VezPipelineDepthStencilState> depth_stencil_states_;
			UnorderedMap<asset::Topology,
				VezInputAssemblyState> input_assembly_states_;

			VulkanRenderer* renderer_;
	  
			bool     dirty_rasterizer_;
			bool     dirty_blend_;
			bool     dirty_depth_stencil_;
			bool     dirty_input_assembly_;
			uint16_t dirty_sampler_;
			VezRasterizationState        rasterizer_;
			ColorBlendState              blend_;
			VkSampler                    samplers_[16];
			VezPipelineDepthStencilState depth_stencil_;
			VezInputAssemblyState        input_assembly_;
    };
  }
}