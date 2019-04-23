#include "vulkan_pipeline_state_manager.h"
#include "vulkan_device_manager.h"
#include <glm/glm.hpp>
#include <utils/console.h>
#include "vulkan_shader.h"

namespace lambda
{
	namespace linux
	{
		///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::initialize(VulkanDeviceManager* device_manager)
		{
			device_manager_ = device_manager;
		}

		void vertexInput()
		{
			VkPipelineVertexInputStateCreateInfo vertex_input_state{};
			vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertex_input_state.vertexBindingDescriptionCount = 0;
			vertex_input_state.pVertexBindingDescriptions = nullptr;
			vertex_input_state.vertexAttributeDescriptionCount = 0;
			vertex_input_state.pVertexAttributeDescriptions = nullptr;
		}

		void inputAssembly()
		{
			VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
			input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			input_assembly_state.primitiveRestartEnable = VK_FALSE;
		}

		void viewportAndScissor(glm::vec4 vp, glm::vec4 sr)
		{
			VkViewport viewport{};
			viewport.x = vp.x;
			viewport.y = vp.y;
			viewport.width = vp.z;
			viewport.height = vp.w;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset.x = (int32_t)sr.x;
			scissor.offset.y = (int32_t)sr.y;
			scissor.extent.width = (int32_t)sr.z;
			scissor.extent.height = (int32_t)sr.w;

			VkPipelineViewportStateCreateInfo viewport_state{};
			viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewport_state.viewportCount = 1;
			viewport_state.pViewports = &viewport;
			viewport_state.scissorCount = 1;
			viewport_state.pScissors = &scissor;
		}

		void rasterizer()
		{
			VkPipelineRasterizationStateCreateInfo rasterizer_state{};
			rasterizer_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer_state.depthClampEnable = VK_FALSE;
			rasterizer_state.rasterizerDiscardEnable = VK_FALSE;
			rasterizer_state.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer_state.lineWidth = 1.0f;
			rasterizer_state.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer_state.depthBiasEnable = VK_FALSE;
			rasterizer_state.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer_state.depthBiasClamp = 0.0f; // Optional
			rasterizer_state.depthBiasSlopeFactor = 0.0f; // Optional
		}

		void multisample()
		{
			VkPipelineMultisampleStateCreateInfo multisample_state{};
			multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisample_state.minSampleShading = 1.0f; // Optional
			multisample_state.pSampleMask = nullptr; // Optional
			multisample_state.alphaToCoverageEnable = VK_FALSE; // Optional
			multisample_state.alphaToOneEnable = VK_FALSE; // Optional
		}

		void colourBlend()
		{
			VkPipelineColorBlendAttachmentState colour_blend_attachment{};
			colour_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colour_blend_attachment.blendEnable = VK_FALSE;
			colour_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			colour_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			colour_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
			colour_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			colour_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			colour_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

			VkPipelineColorBlendStateCreateInfo colour_blend_state{};
			colour_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colour_blend_state.logicOpEnable = VK_FALSE;
			colour_blend_state.logicOp = VK_LOGIC_OP_COPY; // Optional
			colour_blend_state.attachmentCount = 1;
			colour_blend_state.pAttachments = &colour_blend_attachment;
			colour_blend_state.blendConstants[0] = 0.0f; // Optional
			colour_blend_state.blendConstants[1] = 0.0f; // Optional
			colour_blend_state.blendConstants[2] = 0.0f; // Optional
			colour_blend_state.blendConstants[3] = 0.0f; // Optional
		}

		void pipelineLayout(VkDevice device, VkAllocationCallbacks* allocator)
		{
			VkPipelineLayout pipeline_layout;

			VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
			pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipeline_layout_create_info.setLayoutCount = 0; // Optional
			pipeline_layout_create_info.pSetLayouts = nullptr; // Optional
			pipeline_layout_create_info.pushConstantRangeCount = 0; // Optional
			pipeline_layout_create_info.pPushConstantRanges = nullptr; // Optional

			VkResult result;
			result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, allocator, &pipeline_layout);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create pipeline layout | %s", vkErrorCode(result));
		}

		void VulkanPipelineStateManager::deinitialize()
		{
		}
		void VulkanPipelineStateManager::setShader(VulkanShader* shader)
		{
			if (state_.shader == shader)
				return;

			state_.shader = shader;
			makeDirty(kDirtyStateShader);
		}

		void VulkanPipelineStateManager::setVertexInput(VkPipelineVertexInputStateCreateInfo vertex_input)
		{
		}

		void VulkanPipelineStateManager::bindPipeline()
		{
			if (isDirty(kDirtyStateViewport) || isDirty(kDirtyScissorRects))
			{
				vk_state_.viewports.resize(state_.viewports.size());
				vk_state_.scissor_rects.resize(state_.scissor_rects.size());
				
				for (uint32_t i = 0; i < vk_state_.viewports.size(); ++i)
				{
					vk_state_.viewports[i].x        = state_.viewports[i].x;
					vk_state_.viewports[i].y        = state_.viewports[i].y;
					vk_state_.viewports[i].width    = state_.viewports[i].z;
					vk_state_.viewports[i].height   = state_.viewports[i].w;
					vk_state_.viewports[i].minDepth = 0.0f;
					vk_state_.viewports[i].maxDepth = 1.0f;
				}
				
				for (uint32_t i = 0; i < vk_state_.scissor_rects.size(); ++i)
				{
					vk_state_.scissor_rects[i].offset.x      = (int32_t)state_.scissor_rects[i].x;
					vk_state_.scissor_rects[i].offset.y      = (int32_t)state_.scissor_rects[i].y;
					vk_state_.scissor_rects[i].extent.width  = (int32_t)state_.scissor_rects[i].z;
					vk_state_.scissor_rects[i].extent.height = (int32_t)state_.scissor_rects[i].w;
				}

				VkPipelineViewportStateCreateInfo viewport_state{};
				viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewport_state.viewportCount = (uint32_t)vk_state_.viewports.size();
				viewport_state.pViewports    = vk_state_.viewports.data();
				viewport_state.scissorCount  = (uint32_t)vk_state_.scissor_rects.size();
				viewport_state.pScissors     = vk_state_.scissor_rects.data();
			}

			if (isDirty(kDirtyStateRenderTargets))
			{
				// Render targets.
				vk_state_.colour_attachments.resize(state_.render_targets.size(), {});
				vk_state_.colour_attachment_references.resize(state_.render_targets.size(), {});
				for (uint32_t i = 0; i < state_.render_targets.size(); ++i)
				{
					VkImageLayout final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					if (state_.render_targets[i]->format == VK_FORMAT_D16_UNORM ||
						  state_.render_targets[i]->format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
						  state_.render_targets[i]->format == VK_FORMAT_D32_SFLOAT ||
					    state_.render_targets[i]->format == VK_FORMAT_S8_UINT ||
					    state_.render_targets[i]->format == VK_FORMAT_D16_UNORM_S8_UINT ||
						  state_.render_targets[i]->format == VK_FORMAT_D24_UNORM_S8_UINT ||
						  state_.render_targets[i]->format == VK_FORMAT_D32_SFLOAT_S8_UINT)
						final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					
					vk_state_.colour_attachments[i].format         = state_.render_targets[i]->format;
					vk_state_.colour_attachments[i].samples        = VK_SAMPLE_COUNT_1_BIT;
					vk_state_.colour_attachments[i].loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
					vk_state_.colour_attachments[i].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
					vk_state_.colour_attachments[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
					vk_state_.colour_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
					vk_state_.colour_attachments[i].initialLayout  = state_.render_targets[i]->layout;
					vk_state_.colour_attachments[i].finalLayout    = final_layout;
					state_.render_targets[i]->layout               = vk_state_.colour_attachments[i].finalLayout;

					vk_state_.colour_attachment_references[i].attachment = i;
					vk_state_.colour_attachment_references[i].layout     = final_layout;
				}

				VkSubpassDescription subpass{};
				subpass.colorAttachmentCount = (uint32_t)vk_state_.colour_attachment_references.size();
				subpass.pColorAttachments    = vk_state_.colour_attachment_references.data();

				VkRenderPassCreateInfo render_pass_create_info{};
				render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				render_pass_create_info.attachmentCount = (uint32_t)vk_state_.colour_attachments.size();
				render_pass_create_info.pAttachments    = vk_state_.colour_attachments.data();
				render_pass_create_info.subpassCount    = 1;
				render_pass_create_info.pSubpasses      = &subpass;

				VkResult result;
				result = vkCreateRenderPass(device_manager_->getDevice(), &render_pass_create_info, device_manager_->getAllocator(), &vk_state_.render_pass);
				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create render pass | %s", vkErrorCode(result));
			}

			// TODO (Hilze): Add constant buffer support here.
			if (isDirty(kDirtyStateShader))
			{
				VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
				pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipeline_layout_create_info.setLayoutCount         = 0; // Optional
				pipeline_layout_create_info.pSetLayouts            = nullptr; // Optional
				pipeline_layout_create_info.pushConstantRangeCount = 0; // Optional
				pipeline_layout_create_info.pPushConstantRanges    = nullptr; // Optional

				VkResult result;
				result = vkCreatePipelineLayout(device_manager_->getDevice(), &pipeline_layout_create_info, device_manager_->getAllocator(), &vk_state_.pipeline_layout);
				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create pipeline layout | %s", vkErrorCode(result));
			}

			VkGraphicsPipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipeline_create_info.stageCount          = (uint32_t)state_.shader->getShaderStages().size(); // DONE
			pipeline_create_info.pStages             = state_.shader->getShaderStages().data();           // DONE
			pipeline_create_info.pVertexInputState   = &vk_state_.vertex_input;                           // <-- TODO
			pipeline_create_info.pInputAssemblyState = &vk_state_.input_assembly;                         // <-- TODO
			pipeline_create_info.pViewportState      = &vk_state_.viewport_state;                         // DONE
			pipeline_create_info.pRasterizationState = &vk_state_.rasterizer;                             // <-- TODO
			pipeline_create_info.pMultisampleState   = &vk_state_.multisample;                            // <-- TODO
			pipeline_create_info.pDepthStencilState  = nullptr;                                           // DONE
			pipeline_create_info.pColorBlendState    = &vk_state_.colour_blend;                           // <-- TODO
			pipeline_create_info.pDynamicState       = nullptr;                                           // DONE
			pipeline_create_info.layout              = vk_state_.pipeline_layout;                         // <-- TODO
			pipeline_create_info.renderPass          = vk_state_.render_pass;                             // DONE
			pipeline_create_info.subpass             = 0;                                                 // DONE
			pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;                                    // DONE
			pipeline_create_info.basePipelineIndex   = -1;                                                // DONE

			VkResult result;
			result = vkCreateGraphicsPipelines(device_manager_->getDevice(), VK_NULL_HANDLE, 1, &pipeline_create_info, device_manager_->getAllocator(), &pipeline_);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create graphics pipeline | %s", vkErrorCode(result));
		}

		void VulkanPipelineStateManager::setRenderTargets(const Vector<VulkanWrapperImage*>& render_targets)
		{
			bool equal = state_.render_targets.size() == render_targets.size();
			
			for (uint32_t i = 0; i < render_targets.size() && equal; ++i)
				if (render_targets[i]->view != state_.render_targets[i]->view)
					equal = false;

			if (!equal)
			{
				state_.render_targets = render_targets;
				makeDirty(kDirtyStateRenderTargets);
			}
		}
		
		void VulkanPipelineStateManager::setViewports(const Vector<glm::vec4>& viewports)
		{
			bool equal = state_.viewports.size() == viewports.size();

			for (uint32_t i = 0; i < viewports.size() && equal; ++i)
				if (viewports[i].x != state_.viewports[i].x ||
					  viewports[i].y != state_.viewports[i].y ||
					  viewports[i].z != state_.viewports[i].z ||
					  viewports[i].w != state_.viewports[i].w)
					equal = false;

			if (!equal)
			{
				state_.viewports = viewports;
				makeDirty(kDirtyStateViewport);
			}
		}

		void VulkanPipelineStateManager::setScissorRects(const Vector<glm::vec4>& scissor_rects)
		{
			bool equal = state_.scissor_rects.size() == scissor_rects.size();

			for (uint32_t i = 0; i < scissor_rects.size() && equal; ++i)
				if (scissor_rects[i].x != state_.scissor_rects[i].x ||
					  scissor_rects[i].y != state_.scissor_rects[i].y ||
					  scissor_rects[i].z != state_.scissor_rects[i].z ||
					  scissor_rects[i].w != state_.scissor_rects[i].w)
					equal = false;

			if (!equal)
			{
				state_.scissor_rects = scissor_rects;
				makeDirty(kDirtyScissorRects);
			}
		}
	}
}