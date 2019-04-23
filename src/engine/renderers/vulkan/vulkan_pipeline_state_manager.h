#pragma once
#include "vulkan.h"
#include <containers/containers.h>

namespace lambda
{
  namespace linux
  {
    class VulkanDeviceManager;

    //////////////////////////////////////////////////////////////////////
    class VulkanPipelineStateManager
	{
    public:
	  void initialize(VulkanDeviceManager* device_manager);
	  void deinitialize();

	private:
		VkRenderPass render_pass_;
		VkPipeline pipeline_;
		VulkanDeviceManager* device_manager_;

		enum DirtyStates : uint32_t
		{
			kVertexInput    = 1ul << 1ul,
			kInputAssembly  = 2ul << 1ul,
			kViewport       = 3ul << 1ul,
			kRasterizer     = 4ul << 1ul,
			kMultisample    = 5ul << 1ul,
			kColourBlend    = 6ul << 1ul,
			kPipelineLayout = 7ul << 1ul,
			kRenderPass     = 8ul << 1ul,
		};

		struct State
		{
			VkPipelineVertexInputStateCreateInfo vertex_input;
			VkPipelineInputAssemblyStateCreateInfo input_assembly;
			VkPipelineViewportStateCreateInfo viewport;
			VkPipelineRasterizationStateCreateInfo rasterizer;
			VkPipelineMultisampleStateCreateInfo multisample;
			VkPipelineColorBlendStateCreateInfo colour_blend;
			VkPipelineLayout pipeline_layout;
			VkRenderPass render_pass;
			uint32_t dirty;
		} state_;

		VkPipeline pipeline_;

		bool isDirty(uint32_t flag) { return (state_.dirty & flag); }
		void makeDirty(uint32_t flag) { state_.dirty |= flag; }
		void cleanDirty(uint32_t flag) { state_.dirty &= ~flag; }
		void invalidateAll() { state_.dirty = ~0ul; }
		void cleanAll() { state_.dirty = 0ul; }
	};
  }
}