#include "vulkan_state_manager.h"
#include "assets/mesh.h"
#include <utils/console.h>
#include "vulkan_renderer.h"

namespace lambda
{
  namespace linux
  {
    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::initialize(VulkanRenderer* renderer)
    {
      renderer_ = renderer;

	  dirty_rasterizer_ = false;
	  dirty_blend_ = false;
	  dirty_depth_stencil_ = false;
	  dirty_input_assembly_ = false;
	  dirty_sampler_ = 0u;
	  rasterizer_ = {};
	  blend_ = {};
	  memset(samplers_, 0, sizeof(samplers_));
	  depth_stencil_ = {};
	  input_assembly_ = {};
    }

		///////////////////////////////////////////////////////////////////////////
		void VulkanStateManager::update(Vector<VulkanReflectionInfo> samplers, uint32_t attachment_count)
		{
			//if (dirty_rasterizer_)
			{
				//vezCmdSetRasterizationState(&rasterizer_->rasterization);
				dirty_rasterizer_ = false;
			}
			//if (dirty_blend_)
			{
				//vezCmdSetColorBlendState(&blend_->blend_state);
				dirty_blend_ = false;
			}
			//if (dirty_depth_stencil_)
			{
				//vezCmdSetDepthStencilState(&depth_stencil_->depth_stencil);
				dirty_depth_stencil_ = false;
			}
			//if (dirty_input_assembly_)
			{
				//vezCmdSetInputAssemblyState(&input_assembly_->input_assembly);
				dirty_input_assembly_ = false;
			}
			//if (dirty_sampler_)
			//{
			//	for (uint16_t i = 0; i < 16 && dirty_sampler_ != 0; ++i)
			//	{
			//		//if ((dirty_sampler_ & (1 << i)))
			//		{
			//			for (const VulkanReflectionInfo& sampler : samplers)
			//			{
			//				if (sampler.slot == i)
			//				{
			//					//vezCmdBindSampler(samplers_[i], sampler.set, sampler.binding, 0);
			//				}
			//			}
			//
			//			dirty_sampler_ &= ~(1 << i);
			//		}
			//	}
			//}
	
			//for (const VulkanReflectionInfo& sampler : samplers)
				//vezCmdBindSampler(samplers_[sampler.slot]->sampler, sampler.set, sampler.binding, 0);
		}
   
    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
      // Get the state.
      auto it = rasterizer_states_.find(rasterizer_state);
      if (it == rasterizer_states_.end())
      {
		VulkanRasterizationState* vk_rasterization = foundation::Memory::construct<VulkanRasterizationState>();
		/*switch (rasterizer_state.getCullMode())
        {
        case platform::RasterizerState::CullMode::kBack: 
          vk_rasterization->rasterization.cullMode = VK_CULL_MODE_BACK_BIT;  break;
        case platform::RasterizerState::CullMode::kFront: 
          vk_rasterization->rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;  break;
        case platform::RasterizerState::CullMode::kNone: 
          vk_rasterization->rasterization.cullMode = VK_CULL_MODE_NONE;  break;
        }
        switch (rasterizer_state.getFillMode())
        {
        case platform::RasterizerState::FillMode::kSolid:     
          vk_rasterization->rasterization.polygonMode = VK_POLYGON_MODE_FILL; break;
        case platform::RasterizerState::FillMode::kWireframe: 
          vk_rasterization->rasterization.polygonMode = VK_POLYGON_MODE_LINE; break;
        }*/

        rasterizer_states_.insert(eastl::make_pair(rasterizer_state, vk_rasterization));
		it = rasterizer_states_.find(rasterizer_state);
	  }
	  
      rasterizer_ = it->second;
      dirty_rasterizer_ = true;
    }

	static VkBlendOp getBlendOp(platform::BlendState::BlendOp blend_op)
	{
		switch (blend_op)
		{
		default:
		case platform::BlendState::BlendOp::kAdd:         return VkBlendOp::VK_BLEND_OP_ADD;
		case platform::BlendState::BlendOp::kMin:         return VkBlendOp::VK_BLEND_OP_MIN;
		case platform::BlendState::BlendOp::kMax:         return VkBlendOp::VK_BLEND_OP_MAX;
		case platform::BlendState::BlendOp::kRevSubtract: return VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT;
		case platform::BlendState::BlendOp::kSubtract:    return VkBlendOp::VK_BLEND_OP_SUBTRACT;
		}
	}

	static VkCompareOp getDepthCompareOp(platform::DepthStencilState::DepthCompareOp compare_op)
	{
		switch (compare_op)
		{
		default:
		case platform::DepthStencilState::DepthCompareOp::kAlways:       return VkCompareOp::VK_COMPARE_OP_ALWAYS;
		case platform::DepthStencilState::DepthCompareOp::kNever:        return VkCompareOp::VK_COMPARE_OP_NEVER;
		case platform::DepthStencilState::DepthCompareOp::kEqual:        return VkCompareOp::VK_COMPARE_OP_EQUAL;
		case platform::DepthStencilState::DepthCompareOp::kNotEqual:     return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
		case platform::DepthStencilState::DepthCompareOp::kLess:         return VkCompareOp::VK_COMPARE_OP_LESS;
		case platform::DepthStencilState::DepthCompareOp::kLessEqual:    return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
		case platform::DepthStencilState::DepthCompareOp::kGreater:      return VkCompareOp::VK_COMPARE_OP_GREATER;
		case platform::DepthStencilState::DepthCompareOp::kGreaterEqual: return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
		}
	}

	static VkBlendFactor getBlendFactor(platform::BlendState::BlendMode blend_mode)
	{
		switch (blend_mode)
		{
		default:
		case platform::BlendState::BlendMode::kZero:           return VkBlendFactor::VK_BLEND_FACTOR_ZERO;
		case platform::BlendState::BlendMode::kOne:            return VkBlendFactor::VK_BLEND_FACTOR_ONE;
		case platform::BlendState::BlendMode::kSrcColour:      return VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR;
		case platform::BlendState::BlendMode::kInvSrcColour:   return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case platform::BlendState::BlendMode::kSrcAlpha:       return VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
		case platform::BlendState::BlendMode::kInvSrcAlpha:    return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case platform::BlendState::BlendMode::kDestAlpha:      return VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA;
		case platform::BlendState::BlendMode::kInvDestAlpha:   return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case platform::BlendState::BlendMode::kDestColour:     return VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR;
		case platform::BlendState::BlendMode::kInvDestColour:  return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case platform::BlendState::BlendMode::kSrcAlphaSat:    return VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case platform::BlendState::BlendMode::kBlendFactor:    return VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR;
		case platform::BlendState::BlendMode::kInvBlendFactor: return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case platform::BlendState::BlendMode::kSrc1Colour:     return VkBlendFactor::VK_BLEND_FACTOR_SRC1_COLOR;
		case platform::BlendState::BlendMode::kInvSrc1Colour:  return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case platform::BlendState::BlendMode::kSrc1Alpha:      return VkBlendFactor::VK_BLEND_FACTOR_SRC1_ALPHA;
		case platform::BlendState::BlendMode::kInvSrc1Alpha:   return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindBlendState(
      const platform::BlendState& blend_state)
    {
      // Get the state.
      auto it = blend_states_.find(blend_state);
      if (it == blend_states_.end())
      {
        VulkanColorBlendState* vk_blend = foundation::Memory::construct<VulkanColorBlendState>();

        /*for (uint32_t i = 0; i < 8; ++i)
        {
          vk_blend->attachments[i] = {};
          vk_blend->attachments[i].alphaBlendOp        = getBlendOp(blend_state.getBlendOpAlpha());
          vk_blend->attachments[i].blendEnable         = blend_state.getBlendEnable();
          vk_blend->attachments[i].colorBlendOp        = getBlendOp(blend_state.getBlendOp());
          vk_blend->attachments[i].colorWriteMask      = blend_state.getWriteMask();
          vk_blend->attachments[i].dstAlphaBlendFactor = getBlendFactor(blend_state.getDestBlendAlpha());
          vk_blend->attachments[i].dstColorBlendFactor = getBlendFactor(blend_state.getDestBlend());
          vk_blend->attachments[i].srcAlphaBlendFactor = getBlendFactor(blend_state.getSrcBlendAlpha());
          vk_blend->attachments[i].srcColorBlendFactor = getBlendFactor(blend_state.getSrcBlend());
        }

        vk_blend->blend_state = {};
        vk_blend->blend_state.logicOpEnable   = false;
		vk_blend->blend_state.attachmentCount = 8;
		vk_blend->blend_state.pAttachments    = &vk_blend->attachments[0];*/

        blend_states_.insert(eastl::make_pair(blend_state, vk_blend));
		it = blend_states_.find(blend_state);
	  }

	  blend_ = it->second;
	  dirty_blend_ = true;
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
      // Get the state.
      auto it = depth_stencil_states_.find(depth_stencil_state);
      if (it == depth_stencil_states_.end())
      {
        VulkanPipelineDepthStencilState* vk_depth_stencil = foundation::Memory::construct<VulkanPipelineDepthStencilState>();

		/*vk_depth_stencil->depth_stencil.depthBoundsTestEnable = VK_TRUE;
		vk_depth_stencil->depth_stencil.depthCompareOp        = getDepthCompareOp(depth_stencil_state.getDepthCompareOp());
		vk_depth_stencil->depth_stencil.depthTestEnable       = VK_TRUE;
		vk_depth_stencil->depth_stencil.depthWriteEnable      = depth_stencil_state.getDepthWritemode() ? VK_TRUE : VK_FALSE;
		vk_depth_stencil->depth_stencil.stencilTestEnable     = VK_FALSE;*/
        
        depth_stencil_states_.insert(eastl::make_pair(depth_stencil_state, vk_depth_stencil));
		it = depth_stencil_states_.find(depth_stencil_state);
	  }

      depth_stencil_ = it->second;
	  dirty_depth_stencil_ = true;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
      // Get the state.
      auto it = sampler_states_.find(sampler_state);
      if (it == sampler_states_.end())
      {
        VulkanSampler* vk_sampler = foundation::Memory::construct<VulkanSampler>();
		
		//vezSamplerCreateInfo sampler_create_info{};
		/*sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.maxLod = FLT_MAX;

        switch (sampler_state.getClampMode())
        {
        case platform::SamplerState::ClampMode::kBorder: 
          sampler_create_info.addressModeU = sampler_create_info.addressModeV = sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; break;
        case platform::SamplerState::ClampMode::kClamp:  
          sampler_create_info.addressModeU = sampler_create_info.addressModeV = sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
        case platform::SamplerState::ClampMode::kWrap:
          sampler_create_info.addressModeU = sampler_create_info.addressModeV = sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
        }
        switch (sampler_state.getSampleMode())
        {
        case platform::SamplerState::SampleMode::kAnisotropic:
          sampler_create_info.minFilter = sampler_create_info.magFilter = VkFilter::VK_FILTER_LINEAR; break;
        case platform::SamplerState::SampleMode::kLinear:     
          sampler_create_info.minFilter = sampler_create_info.magFilter = VkFilter::VK_FILTER_LINEAR; break;
        case platform::SamplerState::SampleMode::kPoint:     
          sampler_create_info.minFilter = sampler_create_info.magFilter = VkFilter::VK_FILTER_NEAREST; break;
        }*/
		
		/*VkResult result;
		result = vezCreateSampler(renderer_->getDevice(), &sampler_create_info, &vk_sampler->sampler);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create sampler | %s", vkErrorCode(result));*/

		sampler_states_.insert(eastl::make_pair(sampler_state, vk_sampler));
		it = sampler_states_.find(sampler_state);
	  }

      samplers_[slot] = it->second;
	  dirty_sampler_ |= (1 << slot);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindTopology(const asset::Topology& topology)
    {
		// Get the state.
		auto it = input_assembly_states_.find(topology);
		if (it == input_assembly_states_.end())
		{
          VulkanInputAssemblyState* vk_topology = foundation::Memory::construct<VulkanInputAssemblyState>();

		  /*switch (topology)
		  {
		  case asset::Topology::kLines:
            vk_topology->input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
		  case asset::Topology::kTriangles:
            vk_topology->input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
		  }*/

		  input_assembly_states_.insert(eastl::make_pair(topology, vk_topology));
		  it = input_assembly_states_.find(topology);
		}

		input_assembly_  = it->second;
		dirty_input_assembly_ = true;
    }
  }
}