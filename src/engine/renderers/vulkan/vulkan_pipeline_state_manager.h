#pragma once
#include "vulkan.h"
#include <containers/containers.h>
#include <platform/blend_state.h>
#include <platform/rasterizer_state.h>
#include <assets/mesh.h>
#include <glm/glm.hpp>

namespace lambda
{
	namespace linux
	{
		struct VulkanWrapperImage
		{
			VkImage       image;
			VkImageView   view;
			VkImageLayout layout;
			VkFormat      format;
			uint32_t      width;
			uint32_t      height;
		};
	}
}

namespace lambda
{
	namespace linux
	{
		namespace memory
		{
			struct Framebuffer
			{
				Vector<VkImageView> views;
				uint32_t            width;
				uint32_t            height;
				VkRenderPass        render_pass;
				size_t hash() const;
				bool operator==(const Framebuffer& other) const { return hash() == other.hash(); }
			};

			struct RenderPass
			{
				Vector<VulkanWrapperImage*> render_targets;
				size_t hash() const;
				bool operator==(const RenderPass& other) const { return hash() == other.hash(); }
			};

			struct PipelineLayout
			{
				Vector<VkDescriptorSetLayout> descriptor_set_layouts;
				Vector<VkPushConstantRange>   push_constant_ranges;
				size_t hash() const;
				bool operator==(const PipelineLayout& other) const { return hash() == other.hash(); }
			};

			struct Pipeline
			{
				Vector<VkPipelineShaderStageCreateInfo> shader_stages;
				VkPipelineVertexInputStateCreateInfo vertex_input;
				VkPipelineInputAssemblyStateCreateInfo input_assembly;
				VkPipelineRasterizationStateCreateInfo rasterizer;
				VkPipelineMultisampleStateCreateInfo multisample;
				VkPipelineColorBlendStateCreateInfo colour_blend;
				VkPipelineColorBlendAttachmentState colour_blend_attachment;
				VkPipelineLayout pipeline_layout;
				VkRenderPass render_pass;
				size_t hash() const;
				bool operator==(const Pipeline& other) const { return hash() == other.hash(); }
			};
		}
	}
}

namespace eastl
{
	/////////////////////////////////////////////////////////////////////////////
	template <>
	struct hash<lambda::linux::memory::Framebuffer>
	{
		std::size_t operator()(const lambda::linux::memory::Framebuffer& k) const
		{
			return k.hash();
		}
	};

	/////////////////////////////////////////////////////////////////////////////
	template <>
	struct hash<lambda::linux::memory::RenderPass>
	{
		std::size_t operator()(const lambda::linux::memory::RenderPass& k) const
		{
			return k.hash();
		}
	};

	/////////////////////////////////////////////////////////////////////////////
	template <>
	struct hash<lambda::linux::memory::PipelineLayout>
	{
		std::size_t operator()(const lambda::linux::memory::PipelineLayout& k) const
		{
			return k.hash();
		}
	};

	/////////////////////////////////////////////////////////////////////////////
	template <>
	struct hash<lambda::linux::memory::Pipeline>
	{
		std::size_t operator()(const lambda::linux::memory::Pipeline& k) const
		{
			return k.hash();
		}
	};
}

namespace lambda
{
	namespace linux
	{
		class VulkanDeviceManager;
		class VulkanShader;

		//////////////////////////////////////////////////////////////////////
		class VulkanPipelineStateManager
		{
			public:
			void initialize(VulkanDeviceManager* device_manager);
			void deinitialize();

			void setShader(VulkanShader* shader);
			void setRasterizer(platform::RasterizerState rasterizer);
			void setBlendState(platform::BlendState blend_state);
			void setRenderTargets(const Vector<VulkanWrapperImage*>& render_targets);
			void setTopology(const asset::Topology& topology);
			void bindPipeline();
			void endRenderPass();
			void beginRenderPass();

		private:
			VulkanDeviceManager* device_manager_;

			enum DirtyStates : uint32_t
			{
				kDirtyStateVertexInput    = 1ul  << 1ul,
				kDirtyStateTopology       = 2ul  << 1ul,
				kDirtyStateRasterizer     = 3ul  << 1ul,
				kDirtyStateMultisample    = 4ul  << 1ul,
				kDirtyStateBlend          = 5ul  << 1ul,
				kDirtyStateRenderTargets  = 6ul  << 1ul,
				kDirtyStateShader         = 7ul  << 1ul,
			};

			struct State
			{
				asset::Topology topology;
				platform::RasterizerState rasterizer;
				platform::BlendState blend_state;
				VulkanShader* shader;
				Vector<VulkanWrapperImage*> render_targets;
			} state_;

			VkDescriptorSetLayout descriptor_set_layout_;

			struct Memory
			{

				UnorderedMap<memory::RenderPass, VkRenderPass>         render_passes;
				UnorderedMap<memory::Framebuffer, VkFramebuffer>       framebuffers;
				UnorderedMap<memory::PipelineLayout, VkPipelineLayout> pipeline_layouts;
				UnorderedMap<memory::Pipeline, VkPipeline>             pipelines;

				VkRenderPass     getRenderPass(const memory::RenderPass& pass, VkDevice device, VkAllocationCallbacks* allocator);
				VkFramebuffer    getFramebuffer(const memory::Framebuffer& buffer, VkDevice device, VkAllocationCallbacks* allocator);
				VkPipelineLayout getPipelineLayout(const memory::PipelineLayout& layout, VkDevice device, VkAllocationCallbacks* allocator);
				VkPipeline       getPipeline(const memory::Pipeline& pipeline, VkDevice device, VkAllocationCallbacks* allocator);
			} memory_;

			struct VkState
			{
				memory::Pipeline pipeline;
				memory::Framebuffer framebuffer;
				memory::RenderPass render_pass;

				// Bound.
				VkPipeline bound_pipeline;
				VkRenderPass bound_render_pass;
				VkFramebuffer bound_framebuffer;
				bool in_render_pass;

				uint32_t dirty;
			} vk_state_;

			bool isDirty(uint32_t flag) { return (vk_state_.dirty & flag) ? true : false; }
			void makeDirty(uint32_t flag) { vk_state_.dirty |= flag; }
			void cleanDirty(uint32_t flag) { vk_state_.dirty &= ~flag; }
			void invalidateAll() { vk_state_.dirty = ~0ul; }
			void cleanAll() { vk_state_.dirty = 0ul; }
		};
  }
}