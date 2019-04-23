#pragma once
#include "vulkan.h"
#include <containers/containers.h>

namespace lambda
{
  namespace linux
  {
    class VulkanDeviceManager;
		class VulkanShader;

		struct VulkanWrapperImage
		{
			VkImage       image;
			VkImageView   view;
			VkImageLayout layout;
			VkFormat      format;
		};

		//////////////////////////////////////////////////////////////////////
		class VulkanPipelineStateManager
		{
			public:
			void initialize(VulkanDeviceManager* device_manager);
			void deinitialize();

			void setShader(VulkanShader* shader);
			void setVertexInput(VkPipelineVertexInputStateCreateInfo vertex_input);
			void bindPipeline();
			void setRenderTargets(const Vector<VulkanWrapperImage*>& render_targets);
			void setViewports(const Vector<glm::vec4>& viewports);
			void setScissorRects(const Vector<glm::vec4>& scissor_rects);

		private:
			VulkanDeviceManager* device_manager_;

			enum DirtyStates : uint32_t
			{
				kDirtyStateVertexInput    = 1ul  << 1ul,
				kDirtyStateInputAssembly  = 2ul  << 1ul,
				kDirtyStateViewport       = 3ul  << 1ul,
				kDirtyScissorRects        = 4ul  << 1ul,
				kDirtyStateRasterizer     = 5ul  << 1ul,
				kDirtyStateMultisample    = 6ul  << 1ul,
				kDirtyStateColourBlend    = 7ul  << 1ul,
				kDirtyStateRenderTargets  = 8ul  << 1ul,
				kDirtyStateShader         = 9ul  << 1ul,
			};

			struct State
			{
				VulkanShader* shader;
				Vector<glm::vec4> viewports;
				Vector<glm::vec4> scissor_rects;
				Vector<VulkanWrapperImage*> render_targets;
			} state_;

			struct VkState
			{
				VkPipelineVertexInputStateCreateInfo vertex_input;
				VkPipelineInputAssemblyStateCreateInfo input_assembly;
				VkPipelineRasterizationStateCreateInfo rasterizer;
				VkPipelineMultisampleStateCreateInfo multisample;
				VkPipelineColorBlendStateCreateInfo colour_blend;
				VkPipelineLayout pipeline_layout;

				// ViewportState.
				Vector<VkViewport> viewports;
				Vector<VkRect2D> scissor_rects;
				VkPipelineViewportStateCreateInfo viewport_state;

				// RenderPass.
				Vector<VkAttachmentDescription> colour_attachments;
				Vector<VkAttachmentReference> colour_attachment_references;
				VkRenderPass render_pass;
				
				uint32_t dirty;
			} vk_state_;

			VkPipeline pipeline_;

			bool isDirty(uint32_t flag) { return (vk_state_.dirty & flag) ? true : false; }
			void makeDirty(uint32_t flag) { vk_state_.dirty |= flag; }
			void cleanDirty(uint32_t flag) { vk_state_.dirty &= ~flag; }
			void invalidateAll() { vk_state_.dirty = ~0ul; }
			void cleanAll() { vk_state_.dirty = 0ul; }
		};
  }
}