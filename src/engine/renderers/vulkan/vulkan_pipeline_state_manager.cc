#include "vulkan_pipeline_state_manager.h"
#include "vulkan_device_manager.h"
#include <utils/console.h>
#include "vulkan_shader.h"

namespace lambda
{
	namespace linux
	{
		namespace memory
		{
			size_t Framebuffer::hash() const
			{
				size_t hash = 0ull;
				hashCombine(hash, width);
				hashCombine(hash, height);
				hashCombine(hash, render_pass);
				for (VkImageView view : views)
					hashCombine(hash, view);
				return hash;
			}

			size_t memory::RenderPass::hash() const
			{
				size_t hash = 0ull;
				for (VulkanWrapperImage* wrapper : render_targets)
					hashCombine(hash, wrapper->view);
				return hash;
			}

			size_t PipelineLayout::hash() const
			{
				size_t hash = 0ull;
				for (const VkDescriptorSetLayout& layout : descriptor_set_layouts)
					hashCombine(hash, layout);
				for (const VkPushConstantRange& range : push_constant_ranges)
				{
					hashCombine(hash, range.offset);
					hashCombine(hash, range.size);
					hashCombine(hash, range.stageFlags);
				}
				return hash;
			}

			size_t Pipeline::hash() const
			{
				size_t hash = 0ull;
				size_t thash = 0ull;

				// Shader stages.
				{
					thash = 0ull;
					for (const VkPipelineShaderStageCreateInfo& shader_stage : shader_stages)
					{
						hashCombine(thash, shader_stage.flags);
						hashCombine(thash, shader_stage.module);
						hashCombine(thash, shader_stage.stage);
					}
					hashCombine(hash, thash);
				}

				// Vertex input.
				{
					thash = 0ull;
					hashCombine(thash, vertex_input.flags);
					for (uint32_t i = 0; i < vertex_input.vertexAttributeDescriptionCount; ++i)
					{
						hashCombine(thash, vertex_input.pVertexAttributeDescriptions[i].binding);
						hashCombine(thash, vertex_input.pVertexAttributeDescriptions[i].format);
						hashCombine(thash, vertex_input.pVertexAttributeDescriptions[i].location);
						hashCombine(thash, vertex_input.pVertexAttributeDescriptions[i].offset);
					}
					for (uint32_t i = 0; i < vertex_input.vertexBindingDescriptionCount; ++i)
					{
						hashCombine(thash, vertex_input.pVertexBindingDescriptions[i].binding);
						hashCombine(thash, vertex_input.pVertexBindingDescriptions[i].inputRate);
						hashCombine(thash, vertex_input.pVertexBindingDescriptions[i].stride);
					}
					hashCombine(hash, thash);
				}

				// Input assembly.
				{
					thash = 0ull;
					hashCombine(thash, input_assembly.flags);
					hashCombine(thash, input_assembly.primitiveRestartEnable);
					hashCombine(thash, input_assembly.topology);
					hashCombine(hash, thash);
				}

				// Rasterizer.
				{
					thash = 0ull;
					hashCombine(thash, rasterizer.cullMode);
					hashCombine(thash, rasterizer.depthBiasClamp);
					hashCombine(thash, rasterizer.depthBiasConstantFactor);
					hashCombine(thash, rasterizer.depthBiasEnable);
					hashCombine(thash, rasterizer.depthBiasSlopeFactor);
					hashCombine(thash, rasterizer.depthClampEnable);
					hashCombine(thash, rasterizer.flags);
					hashCombine(thash, rasterizer.frontFace);
					hashCombine(thash, rasterizer.lineWidth);
					hashCombine(thash, rasterizer.polygonMode);
					hashCombine(thash, rasterizer.rasterizerDiscardEnable);
					hashCombine(hash, thash);
				}

				// Multisample.
				{
					thash = 0ull;
					hashCombine(thash, multisample.alphaToCoverageEnable);
					hashCombine(thash, multisample.alphaToOneEnable);
					hashCombine(thash, multisample.flags);
					hashCombine(thash, multisample.minSampleShading);
					hashCombine(thash, multisample.rasterizationSamples);
					hashCombine(thash, multisample.sampleShadingEnable);
					hashCombine(hash, thash);
				}

				// Colour blend.
				{
					thash = 0ull;
					hashCombine(thash, colour_blend.blendConstants[0]);
					hashCombine(thash, colour_blend.blendConstants[1]);
					hashCombine(thash, colour_blend.blendConstants[2]);
					hashCombine(thash, colour_blend.blendConstants[3]);
					hashCombine(thash, colour_blend.flags);
					hashCombine(thash, colour_blend.logicOp);
					hashCombine(thash, colour_blend.logicOpEnable);
					for (uint32_t i = 0; i < colour_blend.attachmentCount; ++i)
					{
						hashCombine(thash, colour_blend.pAttachments[i].alphaBlendOp);
						hashCombine(thash, colour_blend.pAttachments[i].blendEnable);
						hashCombine(thash, colour_blend.pAttachments[i].colorBlendOp);
						hashCombine(thash, colour_blend.pAttachments[i].colorWriteMask);
						hashCombine(thash, colour_blend.pAttachments[i].dstAlphaBlendFactor);
						hashCombine(thash, colour_blend.pAttachments[i].dstColorBlendFactor);
						hashCombine(thash, colour_blend.pAttachments[i].srcAlphaBlendFactor);
						hashCombine(thash, colour_blend.pAttachments[i].srcColorBlendFactor);
					}
					hashCombine(hash, thash);
				}

				hashCombine(hash, pipeline_layout);
				hashCombine(hash, render_pass);

				return hash;
			}
		}

		///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::initialize(VulkanDeviceManager* device_manager)
		{
			device_manager_ = device_manager;
			invalidateAll();
		}

		///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::deinitialize()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::setShader(VulkanShader* shader)
		{
			if (state_.shader == shader)
				return;

			state_.shader = shader;
			makeDirty(kDirtyStateShader);
		}

        ///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::setRasterizer(platform::RasterizerState rasterizer)
		{
			if (rasterizer == state_.rasterizer)
				return;

			state_.rasterizer = rasterizer;
			makeDirty(kDirtyStateRasterizer);
		}

        ///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::setBlendState(platform::BlendState blend_state)
		{
			if (blend_state == state_.blend_state)
				return;

			state_.blend_state = blend_state;
			makeDirty(kDirtyStateBlend);
		}
		
        ///////////////////////////////////////////////////////////////////////////
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
		
        ///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::setTopology(const asset::Topology& topology)
		{
			if (state_.topology == topology)
				return;

			state_.topology = topology;
			makeDirty(kDirtyStateTopology);
		}

#pragma optimize ("", off)
		///////////////////////////////////////////////////////////////////////////
		void VulkanPipelineStateManager::bindPipeline()
		{
			if (isDirty(kDirtyStateRenderTargets))
			{
				memory::RenderPass render_pass{};
				render_pass.render_targets = state_.render_targets;
				vk_state_.pipeline.render_pass = memory_.getRenderPass(render_pass, device_manager_->getDevice(), device_manager_->getAllocator());
			}

			if (isDirty(kDirtyStateRasterizer))
			{
				switch (state_.rasterizer.getCullMode())
				{
				case platform::RasterizerState::CullMode::kBack:
					vk_state_.pipeline.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
					break;
				case platform::RasterizerState::CullMode::kFront:
					vk_state_.pipeline.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
					break;
				case platform::RasterizerState::CullMode::kNone:
					vk_state_.pipeline.rasterizer.cullMode = VK_CULL_MODE_NONE;
					break;
				}
				switch (state_.rasterizer.getFillMode())
				{
				case platform::RasterizerState::FillMode::kSolid:
					vk_state_.pipeline.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
					break;
				case platform::RasterizerState::FillMode::kWireframe:
					vk_state_.pipeline.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
					break;
				}
				vk_state_.pipeline.rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				vk_state_.pipeline.rasterizer.depthClampEnable        = VK_FALSE;
				vk_state_.pipeline.rasterizer.rasterizerDiscardEnable = VK_FALSE;
				vk_state_.pipeline.rasterizer.lineWidth               = 1.0f;
				vk_state_.pipeline.rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
				vk_state_.pipeline.rasterizer.depthBiasEnable         = VK_FALSE;
				vk_state_.pipeline.rasterizer.depthBiasConstantFactor = 0.0f;
				vk_state_.pipeline.rasterizer.depthBiasClamp          = 0.0f;
				vk_state_.pipeline.rasterizer.depthBiasSlopeFactor    = 0.0f;
			}

			if (isDirty(kDirtyStateBlend))
			{
				static const auto getBlendOp = [](platform::BlendState::BlendOp op) {
					switch (op)
					{
					default:
					case platform::BlendState::BlendOp::kAdd:         return VK_BLEND_OP_ADD;
					case platform::BlendState::BlendOp::kSubtract:    return VK_BLEND_OP_SUBTRACT;
					case platform::BlendState::BlendOp::kMin:         return VK_BLEND_OP_MIN;
					case platform::BlendState::BlendOp::kMax:         return VK_BLEND_OP_MAX;
					case platform::BlendState::BlendOp::kRevSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
					}
				};
				static const auto getBlendMode = [](platform::BlendState::BlendMode mode) {
					switch (mode)
					{
					default:
					case platform::BlendState::BlendMode::kZero:           return VK_BLEND_FACTOR_ZERO;
					case platform::BlendState::BlendMode::kOne:            return VK_BLEND_FACTOR_ONE;
					case platform::BlendState::BlendMode::kSrcColour:      return VK_BLEND_FACTOR_SRC_COLOR;
					case platform::BlendState::BlendMode::kInvSrcColour:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
					case platform::BlendState::BlendMode::kSrcAlpha:       return VK_BLEND_FACTOR_SRC_ALPHA;
					case platform::BlendState::BlendMode::kInvSrcAlpha:    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					case platform::BlendState::BlendMode::kDestAlpha:      return VK_BLEND_FACTOR_DST_ALPHA;
					case platform::BlendState::BlendMode::kInvDestAlpha:   return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
					case platform::BlendState::BlendMode::kDestColour:     return VK_BLEND_FACTOR_DST_COLOR;
					case platform::BlendState::BlendMode::kInvDestColour:  return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
					case platform::BlendState::BlendMode::kSrcAlphaSat:    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
					case platform::BlendState::BlendMode::kBlendFactor:    return VK_BLEND_FACTOR_CONSTANT_COLOR;
					case platform::BlendState::BlendMode::kInvBlendFactor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
					case platform::BlendState::BlendMode::kSrc1Colour:     return VK_BLEND_FACTOR_SRC1_COLOR;
					case platform::BlendState::BlendMode::kInvSrc1Colour:  return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
					case platform::BlendState::BlendMode::kSrc1Alpha:      return VK_BLEND_FACTOR_SRC1_ALPHA;
					case platform::BlendState::BlendMode::kInvSrc1Alpha:   return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
					}
				};

				vk_state_.pipeline.colour_blend_attachment.colorWriteMask = 0;
				if (state_.blend_state.getWriteMask() & (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableRed)
					vk_state_.pipeline.colour_blend_attachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
				if (state_.blend_state.getWriteMask() & (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableGreen)
					vk_state_.pipeline.colour_blend_attachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
				if (state_.blend_state.getWriteMask() & (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableBlue)
					vk_state_.pipeline.colour_blend_attachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
				if (state_.blend_state.getWriteMask() & (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableAlpha)
					vk_state_.pipeline.colour_blend_attachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

				vk_state_.pipeline.colour_blend_attachment.blendEnable         = state_.blend_state.getBlendEnable() ? VK_TRUE : VK_FALSE;
				vk_state_.pipeline.colour_blend_attachment.srcColorBlendFactor = getBlendMode(state_.blend_state.getSrcBlend());
				vk_state_.pipeline.colour_blend_attachment.dstColorBlendFactor = getBlendMode(state_.blend_state.getDestBlend());
				vk_state_.pipeline.colour_blend_attachment.colorBlendOp        = getBlendOp(state_.blend_state.getBlendOp());
				vk_state_.pipeline.colour_blend_attachment.srcAlphaBlendFactor = getBlendMode(state_.blend_state.getSrcBlendAlpha());
				vk_state_.pipeline.colour_blend_attachment.dstAlphaBlendFactor = getBlendMode(state_.blend_state.getDestBlendAlpha());
				vk_state_.pipeline.colour_blend_attachment.alphaBlendOp        = getBlendOp(state_.blend_state.getBlendOpAlpha());

				vk_state_.pipeline.colour_blend.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				vk_state_.pipeline.colour_blend.logicOpEnable     = VK_FALSE;
				vk_state_.pipeline.colour_blend.logicOp           = VK_LOGIC_OP_COPY;
				vk_state_.pipeline.colour_blend.attachmentCount   = 1;
				vk_state_.pipeline.colour_blend.pAttachments      = &vk_state_.pipeline.colour_blend_attachment;
				vk_state_.pipeline.colour_blend.blendConstants[0] = 0.0f;
				vk_state_.pipeline.colour_blend.blendConstants[1] = 0.0f;
				vk_state_.pipeline.colour_blend.blendConstants[2] = 0.0f;
				vk_state_.pipeline.colour_blend.blendConstants[3] = 0.0f;
			}

			if (isDirty(kDirtyStateTopology))
			{
				vk_state_.pipeline.input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				vk_state_.pipeline.input_assembly.primitiveRestartEnable = VK_FALSE;

				switch (state_.topology)
				{
				case asset::Topology::kLines:
					vk_state_.pipeline.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
					break;
				case asset::Topology::kTriangles:
					vk_state_.pipeline.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
					break;
				}
			}

			if (isDirty(kDirtyStateMultisample))
			{
				vk_state_.pipeline.multisample.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				vk_state_.pipeline.multisample.sampleShadingEnable   = VK_FALSE;
				vk_state_.pipeline.multisample.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
				vk_state_.pipeline.multisample.minSampleShading      = 1.0f;
				vk_state_.pipeline.multisample.pSampleMask           = nullptr;
				vk_state_.pipeline.multisample.alphaToCoverageEnable = VK_FALSE;
				vk_state_.pipeline.multisample .alphaToOneEnable     = VK_FALSE;
			}

			// TODO (Hilze): Add constant buffer support here.
			if (isDirty(kDirtyStateShader))
			{
				memory::PipelineLayout pipeline_layout{};
				vk_state_.pipeline.pipeline_layout = memory_.getPipelineLayout(pipeline_layout, device_manager_->getDevice(), device_manager_->getAllocator());


				VkPipelineVertexInputStateCreateInfo vertex_input_state{};
				vertex_input_state.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				vertex_input_state.vertexBindingDescriptionCount   = 0;
				vertex_input_state.pVertexBindingDescriptions      = nullptr;
				vertex_input_state.vertexAttributeDescriptionCount = 0;
				vertex_input_state.pVertexAttributeDescriptions    = nullptr;

				vk_state_.pipeline.shader_stages = state_.shader->getShaderStages();
			}

			if (vk_state_.dirty)
			{
				// Update pipeline.
				memory_.getPipeline(vk_state_.pipeline, device_manager_->getDevice(), device_manager_->getAllocator());

				// Update framebuffer.
				memory::Framebuffer framebuffer{};
				for (uint32_t i = 0; i < state_.render_targets.size(); ++i)
				{
					if (i == 0)
					{
						framebuffer.width  = state_.render_targets[i]->width;
						framebuffer.height = state_.render_targets[i]->height;
					}
					framebuffer.views.push_back(state_.render_targets[i]->view);
				}

				framebuffer.render_pass = vk_state_.pipeline.render_pass;
				vk_state_.bound_framebuffer = memory_.getFramebuffer(framebuffer, device_manager_->getDevice(), device_manager_->getAllocator());
			}
		}

		VkRenderPass VulkanPipelineStateManager::Memory::getRenderPass(const memory::RenderPass& pass, VkDevice device, VkAllocationCallbacks* allocator)
		{
			auto it = render_passes.find(pass);

			if (it == render_passes.end())
			{
				// Render targets.
				Vector<VkAttachmentDescription> colour_attachments(pass.render_targets.size());
				Vector<VkAttachmentReference> colour_attachment_references(pass.render_targets.size());
				
				for (uint32_t i = 0; i < pass.render_targets.size(); ++i)
				{
					VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					if (pass.render_targets[i]->format == VK_FORMAT_D16_UNORM ||
						pass.render_targets[i]->format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
						pass.render_targets[i]->format == VK_FORMAT_D32_SFLOAT ||
						pass.render_targets[i]->format == VK_FORMAT_S8_UINT ||
						pass.render_targets[i]->format == VK_FORMAT_D16_UNORM_S8_UINT ||
						pass.render_targets[i]->format == VK_FORMAT_D24_UNORM_S8_UINT ||
						pass.render_targets[i]->format == VK_FORMAT_D32_SFLOAT_S8_UINT)
					  layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					
					colour_attachments[i].format         = pass.render_targets[i]->format;
					colour_attachments[i].samples        = VK_SAMPLE_COUNT_1_BIT;
					colour_attachments[i].loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
					colour_attachments[i].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
					colour_attachments[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
					colour_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
					colour_attachments[i].initialLayout  = pass.render_targets[i]->layout;
					colour_attachments[i].finalLayout    = pass.render_targets[i]->layout;

					colour_attachment_references[i].attachment = i;
					colour_attachment_references[i].layout     = layout;
				}

				VkSubpassDescription subpass{};
				subpass.colorAttachmentCount = (uint32_t)colour_attachment_references.size();
				subpass.pColorAttachments    = colour_attachment_references.data();

				VkRenderPassCreateInfo render_pass_create_info{};
				render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				render_pass_create_info.attachmentCount = (uint32_t)colour_attachments.size();
				render_pass_create_info.pAttachments    = colour_attachments.data();
				render_pass_create_info.subpassCount    = 1;
				render_pass_create_info.pSubpasses      = &subpass;

				VkResult result;
				VkRenderPass render_pass;
				result = vkCreateRenderPass(device, &render_pass_create_info, allocator, &render_pass);
				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create render pass | %s", vkErrorCode(result));
				render_passes.insert({ pass, render_pass });
				return render_pass;
			}

			return it->second;
		}

		VkFramebuffer VulkanPipelineStateManager::Memory::getFramebuffer(const memory::Framebuffer& buffer, VkDevice device, VkAllocationCallbacks* allocator)
		{
			auto it = framebuffers.find(buffer);

			if (it == framebuffers.end())
			{
				VkFramebufferCreateInfo framebuffer_create_info{};
				framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebuffer_create_info.renderPass      = buffer.render_pass;
				framebuffer_create_info.attachmentCount = (uint32_t)buffer.views.size();
				framebuffer_create_info.pAttachments    = buffer.views.data();
				framebuffer_create_info.width           = buffer.width;
				framebuffer_create_info.height          = buffer.height;
				framebuffer_create_info.layers          = 1;

				VkResult result;
				VkFramebuffer framebuffer;
				result = vkCreateFramebuffer(device, &framebuffer_create_info, allocator, &framebuffer);
				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create framebuffer | %s", vkErrorCode(result));
				framebuffers.insert({ buffer, framebuffer });
				return framebuffer;
			}

			return it->second;
		}
		VkPipelineLayout VulkanPipelineStateManager::Memory::getPipelineLayout(const memory::PipelineLayout& layout, VkDevice device, VkAllocationCallbacks* allocator)
		{
			auto it = pipeline_layouts.find(layout);

			if (it == pipeline_layouts.end())
			{
				VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
				pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipeline_layout_create_info.setLayoutCount         = (uint32_t)layout.descriptor_set_layouts.size();
				pipeline_layout_create_info.pSetLayouts            = layout.descriptor_set_layouts.data();
				pipeline_layout_create_info.pushConstantRangeCount = (uint32_t)layout.push_constant_ranges.size();
				pipeline_layout_create_info.pPushConstantRanges    = layout.push_constant_ranges.data();

				VkResult result;
				VkPipelineLayout pipeline_layout;
				result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, allocator, &pipeline_layout);
				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create pipeline layout | %s", vkErrorCode(result));
				pipeline_layouts.insert({ layout, pipeline_layout });
				return pipeline_layout;
			}

			return it->second;
		}
		VkPipeline VulkanPipelineStateManager::Memory::getPipeline(const memory::Pipeline& pipeline, VkDevice device, VkAllocationCallbacks* allocator)
		{
			auto it = pipelines.find(pipeline);

			if (it == pipelines.end())
			{
				VkPipelineViewportStateCreateInfo viewport_state{};
				viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewport_state.viewportCount = 0;
				viewport_state.pViewports    = nullptr;
				viewport_state.scissorCount  = 0;
				viewport_state.pScissors     = nullptr;


				VkDynamicState dynamic_state_array[] = {
					VK_DYNAMIC_STATE_VIEWPORT,
					VK_DYNAMIC_STATE_SCISSOR
				};
				VkPipelineDynamicStateCreateInfo dynamic_states{};
				dynamic_states.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				dynamic_states.dynamicStateCount = sizeof(dynamic_state_array) / sizeof(dynamic_state_array[0]);
				dynamic_states.pDynamicStates = dynamic_state_array;

				VkGraphicsPipelineCreateInfo pipeline_create_info{};
				pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipeline_create_info.stageCount          = (uint32_t)pipeline.shader_stages.size(); // DONE
				pipeline_create_info.pStages             = pipeline.shader_stages.data();           // DONE
				pipeline_create_info.pVertexInputState   = &pipeline.vertex_input;                  // <-- TODO
				pipeline_create_info.pInputAssemblyState = &pipeline.input_assembly;                // DONE
				pipeline_create_info.pViewportState      = &viewport_state;                         // DONE
				pipeline_create_info.pRasterizationState = &pipeline.rasterizer;                    // DONE
				pipeline_create_info.pMultisampleState   = &pipeline.multisample;                   // DONE
				pipeline_create_info.pDepthStencilState  = nullptr;                                 // DONE
				pipeline_create_info.pColorBlendState    = &pipeline.colour_blend;                  // DONE
				pipeline_create_info.pDynamicState       = &dynamic_states;                         // DONE
				pipeline_create_info.layout              = pipeline.pipeline_layout;                // <-- TODO
				pipeline_create_info.renderPass          = pipeline.render_pass;                    // DONE
				pipeline_create_info.subpass             = 0;                                       // DONE
				pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;                          // DONE
				pipeline_create_info.basePipelineIndex   = -1;                                      // DONE

				VkResult result;
				VkPipeline vk_pipeline;
				result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, allocator, &vk_pipeline);
				LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create graphics pipeline | %s", vkErrorCode(result));
				pipelines.insert({ pipeline, vk_pipeline });
				return vk_pipeline;
			}

			return it->second;
		}
	}
}