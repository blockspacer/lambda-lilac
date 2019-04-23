#include "vulkan_pipeline_state_manager.h"
#include "vulkan_device_manager.h"
#include <glm/glm.hpp>
#include <utils/console.h>

namespace lambda
{
  namespace linux
  {
    ///////////////////////////////////////////////////////////////////////////
	void VulkanPipelineStateManager::initialize(VulkanDeviceManager* device_manager)
	{
		device_manager_ = device_manager;

		// Render targets.
		VkAttachmentDescription colour_attachment{};
		colour_attachment.format         = device_manager_->getSwapchainFormat();
		colour_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		colour_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colour_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		colour_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colour_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		colour_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Textures.
		VkAttachmentReference colour_attachment_reference{};
		colour_attachment_reference.attachment = 0;
		colour_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass{};
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments    = &colour_attachment_reference;

		VkRenderPassCreateInfo render_pass_create_info{};
		render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments    = &colour_attachment;
		render_pass_create_info.subpassCount    = 1;
		render_pass_create_info.pSubpasses      = &subpass;

		VkResult result;
		VkRenderPass render_pass;
		result = vkCreateRenderPass(device_manager_->getDevice(), &render_pass_create_info, device_manager_->getAllocator(), &render_pass);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create render pass | %s", vkErrorCode(result));



		VkGraphicsPipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//pipeline_create_info.stageCount          = 2;
		//pipeline_create_info.pStages             = state_.shader_stages;
		pipeline_create_info.pVertexInputState   = &state_.vertex_input;
		pipeline_create_info.pInputAssemblyState = &state_.input_assembly;
		pipeline_create_info.pViewportState      = &state_.viewport;
		pipeline_create_info.pRasterizationState = &state_.rasterizer;
		pipeline_create_info.pMultisampleState   = &state_.multisample;
		pipeline_create_info.pDepthStencilState  = nullptr; // Optional
		pipeline_create_info.pColorBlendState    = &state_.colour_blend;
		pipeline_create_info.pDynamicState       = nullptr; // Optional
		pipeline_create_info.layout              = state_.pipeline_layout;
		pipeline_create_info.renderPass          = state_.render_pass;
		pipeline_create_info.subpass             = 0;
		pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE; // Optional
		pipeline_create_info.basePipelineIndex   = -1; // Optional

		VkResult result;
		result = vkCreateGraphicsPipelines(device_manager_->getDevice(), VK_NULL_HANDLE, 1, &pipeline_create_info, device_manager_->getAllocator(), &pipeline_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create graphics pipeline | %s", vkErrorCode(result));
	}

	void vertexInput()
	{
		VkPipelineVertexInputStateCreateInfo vertex_input_state{};
		vertex_input_state.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state.vertexBindingDescriptionCount   = 0;
		vertex_input_state.pVertexBindingDescriptions      = nullptr;
		vertex_input_state.vertexAttributeDescriptionCount = 0;
		vertex_input_state.pVertexAttributeDescriptions    = nullptr;
	}

	void inputAssembly()
	{
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
		input_assembly_state.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state.primitiveRestartEnable = VK_FALSE;
	}

	void viewportAndScissor(glm::vec4 vp, glm::vec4 sr)
	{
		VkViewport viewport{};
		viewport.x        = vp.x;
		viewport.y        = vp.y;
		viewport.width    = vp.z;
		viewport.height   = vp.w;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset.x      = sr.x;
		scissor.offset.y      = sr.y;
		scissor.extent.width  = sr.z;
		scissor.extent.height = sr.w;

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports    = &viewport;
		viewport_state.scissorCount  = 1;
		viewport_state.pScissors     = &scissor;
	}

	void rasterizer()
	{
		VkPipelineRasterizationStateCreateInfo rasterizer_state{};
		rasterizer_state.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer_state.depthClampEnable        = VK_FALSE;
		rasterizer_state.rasterizerDiscardEnable = VK_FALSE;
		rasterizer_state.polygonMode             = VK_POLYGON_MODE_FILL;
		rasterizer_state.lineWidth               = 1.0f;
		rasterizer_state.cullMode                = VK_CULL_MODE_BACK_BIT;
		rasterizer_state.frontFace               = VK_FRONT_FACE_CLOCKWISE;
		rasterizer_state.depthBiasEnable         = VK_FALSE;
		rasterizer_state.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer_state.depthBiasClamp          = 0.0f; // Optional
		rasterizer_state.depthBiasSlopeFactor    = 0.0f; // Optional
	}

	void multisample()
	{
		VkPipelineMultisampleStateCreateInfo multisample_state{};
		multisample_state.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.sampleShadingEnable   = VK_FALSE;
		multisample_state.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		multisample_state.minSampleShading      = 1.0f; // Optional
		multisample_state.pSampleMask           = nullptr; // Optional
		multisample_state.alphaToCoverageEnable = VK_FALSE; // Optional
		multisample_state.alphaToOneEnable      = VK_FALSE; // Optional
	}

	void colourBlend()
	{
		VkPipelineColorBlendAttachmentState colour_blend_attachment{};
		colour_blend_attachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colour_blend_attachment.blendEnable         = VK_FALSE;
		colour_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colour_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colour_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD; // Optional
		colour_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colour_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colour_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colour_blend_state{};
		colour_blend_state.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colour_blend_state.logicOpEnable     = VK_FALSE;
		colour_blend_state.logicOp           = VK_LOGIC_OP_COPY; // Optional
		colour_blend_state.attachmentCount   = 1;
		colour_blend_state.pAttachments      = &colour_blend_attachment;
		colour_blend_state.blendConstants[0] = 0.0f; // Optional
		colour_blend_state.blendConstants[1] = 0.0f; // Optional
		colour_blend_state.blendConstants[2] = 0.0f; // Optional
		colour_blend_state.blendConstants[3] = 0.0f; // Optional
	}

	void dynamicState()
	{
		VkDynamicState dynamic_states[] {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]);
		dynamic_state.pDynamicStates    = dynamic_states;
	}

	void pipelineLayout(VkDevice device, VkAllocationCallbacks* allocator)
	{
		VkPipelineLayout pipeline_layout;

		VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount         = 0; // Optional
		pipeline_layout_create_info.pSetLayouts            = nullptr; // Optional
		pipeline_layout_create_info.pushConstantRangeCount = 0; // Optional
		pipeline_layout_create_info.pPushConstantRanges    = nullptr; // Optional

		VkResult result;
		result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, allocator, &pipeline_layout);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create pipeline layout | %s", vkErrorCode(result));
	}

	void VulkanPipelineStateManager::deinitialize()
	{
	}
  }
}