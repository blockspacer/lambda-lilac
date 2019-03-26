#include "vulkan_state_manager.h"
#include "assets/mesh.h"
#include <utils/console.h>

namespace lambda
{
  namespace linux
  {
    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::initialize(VulkanRenderer* renderer)
    {
      renderer_ = renderer;
      blend_states_.clear();
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
	  // Get the state.
	  VezRasterizationState rasterization_state;
	  const auto& it = rasterizer_states_.find(rasterizer_state);
      if (it == rasterizer_states_.end())
      {
		rasterization_state.cullMode;
		rasterization_state.depthBiasEnable;
		rasterization_state.depthClampEnable;
		rasterization_state.frontFace;
		rasterization_state.pNext;
		rasterization_state.polygonMode;
		rasterization_state.rasterizerDiscardEnable;

		switch (rasterizer_state.getCullMode())
        {
        case platform::RasterizerState::CullMode::kBack: 
		  rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;  break;
        case platform::RasterizerState::CullMode::kFront: 
		  rasterization_state.cullMode = VK_CULL_MODE_FRONT_BIT;  break;
        case platform::RasterizerState::CullMode::kNone: 
		  rasterization_state.cullMode = VK_CULL_MODE_NONE;  break;
        }
        switch (rasterizer_state.getFillMode())
        {
        case platform::RasterizerState::FillMode::kSolid:     
		  rasterization_state.polygonMode = VK_POLYGON_MODE_FILL; break;
        case platform::RasterizerState::FillMode::kWireframe: 
		  rasterization_state.polygonMode = VK_POLYGON_MODE_LINE; break;
        }

        rasterizer_states_.insert(
          eastl::make_pair(rasterizer_state, rasterization_state)
        );
      }
      else
        rasterization_state = it->second;
	  
      bound_rasterizer_state_ = rasterization_state;

      // Set the state.
	  vezCmdSetRasterizationState(&bound_rasterizer_state_);
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
	  VezColorBlendState color_blend_state;
      const auto& it = blend_states_.find(blend_state);
      if (it == blend_states_.end())
      {
		// TODO (Hilze): Fix this memory leak.
		VezColorBlendAttachmentState* attachment = 
			foundation::Memory::construct<VezColorBlendAttachmentState>();
		attachment->alphaBlendOp = getBlendOp(blend_state.getBlendOpAlpha());
		attachment->blendEnable = blend_state.getBlendEnable();
		attachment->colorBlendOp = getBlendOp(blend_state.getBlendOp());
		attachment->colorWriteMask = blend_state.getWriteMask();
		attachment->dstAlphaBlendFactor = getBlendFactor(blend_state.getDestBlendAlpha());
		attachment->dstColorBlendFactor = getBlendFactor(blend_state.getDestBlend());
		attachment->srcAlphaBlendFactor = getBlendFactor(blend_state.getSrcBlendAlpha());
		attachment->srcColorBlendFactor = getBlendFactor(blend_state.getSrcBlend());
        color_blend_state.attachmentCount = 1;
		color_blend_state.logicOp;
		color_blend_state.logicOpEnable = false;
		color_blend_state.pAttachments = attachment;

        blend_states_.insert(eastl::make_pair(blend_state, color_blend_state));
      }
      else
		  color_blend_state = it->second;

      bound_blend_state_ = color_blend_state;

      // Set the state.
	  vezCmdSetColorBlendState(&bound_blend_state_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
      // Get the state.
      VezPipelineDepthStencilState pipeline_depth_stencil_state;
      const auto& it = depth_stencil_states_.find(depth_stencil_state);
      if (it == depth_stencil_states_.end())
      {
		pipeline_depth_stencil_state.depthBoundsTestEnable = VK_TRUE;
		pipeline_depth_stencil_state.depthCompareOp        = getDepthCompareOp(depth_stencil_state.getDepthCompareOp());
		pipeline_depth_stencil_state.depthTestEnable       = VK_TRUE;
		pipeline_depth_stencil_state.depthWriteEnable      = depth_stencil_state.getDepthWritemode() ? VK_TRUE : VK_FALSE;
		pipeline_depth_stencil_state.stencilTestEnable     = VK_FALSE;
        
        depth_stencil_states_.insert(
          eastl::make_pair(depth_stencil_state, pipeline_depth_stencil_state)
        );
      }
      else
		  pipeline_depth_stencil_state = it->second;

      bound_depth_stencil_state_ = pipeline_depth_stencil_state;

	  vezCmdSetDepthStencilState(&bound_depth_stencil_state_);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
      // Get the state.
      VkSampler sampler;
      const auto& it = sampler_states_.find(sampler_state);
      if (it == sampler_states_.end())
      {
		VezSamplerCreateInfo sampler_create_info{};
		sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.maxLod = FLT_MAX;

        switch (sampler_state.getClampMode())
        {
        case platform::SamplerState::ClampMode::kBorder: 
		  sampler_create_info.addressModeU = sampler_create_info.addressModeV =
		    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		  break;
        case platform::SamplerState::ClampMode::kClamp:  
          sampler_create_info.addressModeU = sampler_create_info.addressModeV =
		    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		  break;
        case platform::SamplerState::ClampMode::kWrap:
		  sampler_create_info.addressModeU = sampler_create_info.addressModeV =
		    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		  break;
        }
        switch (sampler_state.getSampleMode())
        {
        case platform::SamplerState::SampleMode::kAnisotropic:
		  sampler_create_info.minFilter = sampler_create_info.magFilter = VkFilter::VK_FILTER_LINEAR; break;
        case platform::SamplerState::SampleMode::kLinear:     
          sampler_create_info.minFilter = sampler_create_info.magFilter = VkFilter::VK_FILTER_LINEAR; break;
        case platform::SamplerState::SampleMode::kPoint:     
          sampler_create_info.minFilter = sampler_create_info.magFilter = VkFilter::VK_FILTER_NEAREST; break;
        }
		VkResult result;
		result = vezCreateSampler(renderer_->getDevice(), &sampler_create_info, &sampler);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create sampler | %s", vkErrorCode(result));
      }
      else
		  sampler = it->second;

      bound_sampler_states_[slot] = sampler;

	  vezCmdBindSampler(bound_sampler_states_[slot], 0, 0, slot); // TODO (Hilze): Fix this ASAP!
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanStateManager::bindTopology(const asset::Topology& topology)
    {
		// Get the state.
		VezInputAssemblyState input_assembly_state;
		const auto& it = input_assembly_states_.find(topology);
		if (it == input_assembly_states_.end())
		{
			switch (topology)
			{
			case asset::Topology::kLines:
				input_assembly_state.topology =
					VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				break;
			case asset::Topology::kTriangles:
				input_assembly_state.topology =
					VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				break;
			}
		}
		else
			input_assembly_state = it->second;

		bound_input_assembly_state_ = input_assembly_state;

	    vezCmdSetInputAssemblyState(&bound_input_assembly_state_);
    }
  }
}