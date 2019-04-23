#include "vulkan_renderer.h"
#include <interfaces/iworld.h>
#include "vulkan_texture.h"
#include "vulkan_shader.h"
#include "vulkan_mesh.h"
#include "vulkan_state_manager.h"
#include "memory/frame_heap.h"
#include <fstream>
#include <stb_image.h>
#include <vulkan/vulkan.hpp>

namespace lambda
{
  namespace linux
  {
	///////////////////////////////////////////////////////////////////////////
	platform::IRenderBuffer* VulkanRenderer::allocRenderBuffer(uint32_t size, uint32_t flags, void* data)
	{
		const bool is_dynamic   = 
        (flags & platform::IRenderBuffer::kFlagDynamic)   ? true : false;
      const bool is_staging   = 
        (flags & platform::IRenderBuffer::kFlagStaging)   ? true : false;
      const bool is_immutable = 
        (flags & platform::IRenderBuffer::kFlagImmutable) ? true : false;
      const bool is_vertex    = 
        (flags & platform::IRenderBuffer::kFlagVertex)    ? true : false;
      const bool is_index     = 
        (flags & platform::IRenderBuffer::kFlagIndex)     ? true : false;
      const bool is_constant  = 
        (flags & platform::IRenderBuffer::kFlagConstant)  ? true : false;
/*
	  VezMemoryFlags mem_flags;
	  VezBufferCreateInfo buffer_create_info{};

	  buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		  (is_vertex ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT :
		  (is_index ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT :
		  (is_constant ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0u)));
	  mem_flags = is_dynamic ? VEZ_MEMORY_CPU_TO_GPU :
		  (is_staging ? VEZ_MEMORY_CPU_TO_GPU : VEZ_MEMORY_GPU_ONLY);
	  buffer_create_info.pQueueFamilyIndices;
	  buffer_create_info.queueFamilyIndexCount;
	  buffer_create_info.size = size;

	  VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	  VkBuffer buffer = VK_NULL_HANDLE;
	  VkResult result;
	  result = vezCreateBuffer(device_, mem_flags, &buffer_create_info, &buffer);
	  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create buffer | %s", vkErrorCode(result));
	  
	  if (data)
		vezBufferSubData(device_, buffer, 0, size, data);

      if (is_vertex)
        memory_stats_.vertex   += size;
      if (is_index)
        memory_stats_.index    += size;
      if (is_constant)
        memory_stats_.constant += size;

      return foundation::Memory::construct<VulkanRenderBuffer>(
        size, 
        flags, 
        buffer, 
		this
      );*/
	  return nullptr;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::freeRenderBuffer(platform::IRenderBuffer*& buffer)
	{
		const bool is_vertex =
			(buffer->getFlags() & platform::IRenderBuffer::kFlagVertex)
			? true : false;
		const bool is_index =
			(buffer->getFlags() & platform::IRenderBuffer::kFlagIndex)
			? true : false;
		const bool is_constant =
			(buffer->getFlags() & platform::IRenderBuffer::kFlagConstant)
			? true : false;

		if (is_vertex)
			memory_stats_.vertex -= ((VulkanRenderBuffer*)buffer)->getGPUSize();
		if (is_index)
			memory_stats_.index -= ((VulkanRenderBuffer*)buffer)->getGPUSize();
		if (is_constant)
			memory_stats_.constant -= ((VulkanRenderBuffer*)buffer)->getGPUSize();

		//vezDestroyBuffer(device_, ((VulkanRenderBuffer*)buffer)->getBuffer());
		//foundation::Memory::destruct(buffer);
		buffer = nullptr;
	}

	///////////////////////////////////////////////////////////////////////////
	platform::IRenderTexture* VulkanRenderer::allocRenderTexture(asset::VioletTextureHandle texture)
	{
		VulkanRenderTexture* vulkan_texture =
			foundation::Memory::construct<VulkanRenderTexture>(
				texture->getLayer(0u).getWidth(),
				texture->getLayer(0u).getHeight(),
				texture->getLayerCount(),
				texture->getLayer(0u).getMipCount(),
				texture->getLayer(0u).getFormat(),
				texture->getLayer(0u).getFlags(),
				foundation::Memory::construct<VulkanTexture>(texture, this),
				this
				);

		if ((texture->getLayer(0u).getFlags() & kTextureFlagIsRenderTarget) != 0u)
			memory_stats_.render_target += vulkan_texture->getTexture()->getGPUSize();
		else
			memory_stats_.texture += vulkan_texture->getTexture()->getGPUSize();

		auto total = memory_stats_.vertex + memory_stats_.index + memory_stats_.constant + memory_stats_.texture + memory_stats_.render_target;
		foundation::Debug(toString(TO_MB(total)) + "\n");

		return vulkan_texture;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::freeRenderTexture(platform::IRenderTexture*& texture)
	{
		if ((texture->getFlags() & kTextureFlagIsRenderTarget) != 0u)
			memory_stats_.render_target -= ((VulkanRenderTexture*)texture)->getTexture()->getGPUSize();
		else
			memory_stats_.texture -= ((VulkanRenderTexture*)texture)->getTexture()->getGPUSize();

		foundation::Memory::destruct(((VulkanRenderTexture*)texture)->getTexture());
		foundation::Memory::destruct(texture);
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanRenderer::VulkanRenderer()
	  : scene_(nullptr)
	  , override_scene_(nullptr)
	  , vsync_(false)
	  , command_buffer_()
	  , render_scale_(1.0f)
#if VIOLET_USE_GPU_MARKERS
	  , has_debug_markers_(false)
	  , marker_depth_(0u)
#endif
	{
		memset(&memory_stats_, 0, sizeof(memory_stats_));
		memset(&state_, 0, sizeof(state_));
		memory_.renderer = this;
	}

	///////////////////////////////////////////////////////////////////////////
    VulkanRenderer::~VulkanRenderer()
    {
		memory_.removeAllEntries();
		memory_.renderer = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::initialize(scene::Scene& scene)
    {
		scene_ = &scene;
		
		full_screen_quad_.mesh = asset::MeshManager::getInstance()->create(Name("__full_screen_quad__"), asset::Mesh::createScreenQuad());
		full_screen_quad_.shader = asset::ShaderManager::getInstance()->get(Name("resources/shaders/full_screen_quad.fx"));

		default_texture_ = asset::TextureManager::getInstance()->create(
			Name("__default_render_texture__"),
			1u, 1u, 1u, TextureFormat::kR8G8B8A8,
			kTextureFlagIsRenderTarget, // TODO (Hilze): Remove!
			Vector<char>(4, 255)
		);
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::deinitialize()
    {
		device_manager_.deinitialize();
    }

    ///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setWindow(platform::IWindow* window)
	{
		device_manager_.initialize(window);

		memory_.getShader(full_screen_quad_.shader);
		
		createCommandBuffer();

		// Set VSync.
		setVSync(getVSync());

		state_manager_.initialize(this);
		resize();

		setSamplerState(platform::SamplerState::PointClamp(), 6u);
		setSamplerState(platform::SamplerState::LinearClamp(), 7u);
		setSamplerState(platform::SamplerState::AnisotrophicClamp(), 8u);
		setSamplerState(platform::SamplerState::PointBorder(), 9u);
		setSamplerState(platform::SamplerState::LinearBorder(), 10u);
		setSamplerState(platform::SamplerState::AnisotrophicBorder(), 11u);
		setSamplerState(platform::SamplerState::PointWrap(), 12u);
		setSamplerState(platform::SamplerState::LinearWrap(), 13u);
		setSamplerState(platform::SamplerState::AnisotrophicWrap(), 14u);

		for (uint32_t i = 0; i < 16; ++i)
			setTexture(default_texture_, i);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setOverrideScene(scene::Scene* scene)
	{
		override_scene_ = scene;
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::resize()
    {
		glm::uvec2 render_size =
			(glm::uvec2)((glm::vec2)getScene()->window->getSize() *
				getScene()->window->getDPIMultiplier());

		getScene()->post_process_manager->resize(glm::vec2(render_size.x, render_size.y) * render_scale_);

		screen_size_.x = (float)render_size.x;
		screen_size_.y = (float)render_size.y;
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::update(const double& delta_time)
    {
		delta_time_ = (float)delta_time;
		total_time_ += delta_time_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::startFrame()
    {
		command_buffer_.tryBegin();

		// Clear everything.
		beginTimer("Clear Everything");
		pushMarker("Clear Everything");
		glm::vec4 colour(0.0f);
		for (const auto& target : getScene()->post_process_manager->getAllTargets())
		{
			if (target.second.getTexture()->getLayer(0u).getFlags()
				& kTextureFlagClear)
			{
				clearRenderTarget(target.second.getTexture(), colour);
			}
		}
		popMarker();
		endTimer("Clear Everything");

		// Reset everything back to normal.

		memset(&state_, 0, sizeof(state_));
		state_manager_.bindTopology(asset::Topology::kTriangles);
		setSamplerState(platform::SamplerState::LinearWrap(), 0u);
		setRasterizerState(platform::RasterizerState::SolidFront());

		memset(&state_, 0, sizeof(state_));
		state_.sub_mesh = UINT32_MAX;
		memset(&vk_state_, 0, sizeof(vk_state_));
		invalidateAll();

		if (!cbs_.drs) cbs_.drs = (VulkanRenderBuffer*)allocRenderBuffer(sizeof(float), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic, nullptr);
		float drs_data[] = { dynamic_resolution_scale_ };
		memcpy(cbs_.drs->lock(), drs_data, sizeof(drs_data));
		cbs_.drs->unlock();
		setConstantBuffer(cbs_.drs, cbDynamicResolutionIdx);

		if (!cbs_.per_frame) cbs_.per_frame = (VulkanRenderBuffer*)allocRenderBuffer(sizeof(float) * 4, platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic, nullptr);
		float per_frame_data[] = { screen_size_.x, screen_size_.y, delta_time_, total_time_ };
		memcpy(cbs_.per_frame->lock(), per_frame_data, sizeof(per_frame_data));
		cbs_.per_frame->unlock();
		setConstantBuffer(cbs_.per_frame, cbPerFrameIdx);
	}
   
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::endFrame(bool display)
    {
		//VkResult result;

		//// End command buffer recording.
		//command_buffer_.tryEnd();
		//Vector<VkCommandBuffer> command_buffer = command_buffer_.getCommandBuffers();

		//// Submit the command buffer to the graphics queue.
		//VkSubmitInfo submit_info{};
		//submit_info.commandBufferCount = (uint32_t)command_buffer.size();
		//submit_info.pCommandBuffers    = command_buffer.data();

		//// Request a wait semaphore to pass to present so it waits for rendering to complete.
		//VkSemaphore semaphore = VK_NULL_HANDLE;
		//submit_info.signalSemaphoreCount = 1;
		//submit_info.pSignalSemaphores    = &semaphore;
		//result = vkQueueSubmit(graphics_queue_, 1, &submit_info, nullptr);
		//LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not submit queue | %s", vkErrorCode(result));

		//vkDeviceWaitIdle(device_);

		//// Present the swapchain framebuffer to the window.
		//VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		//VkPresentInfoKHR present_info{};
		//present_info.waitSemaphoreCount = 1;
		//present_info.pWaitSemaphores    = &semaphore;
		////present_info.pWaitDstStageMask  = &wait_dst_stage_mask;
		//present_info.swapchainCount     = 1;
		////present_info.pSwapchains        = &swapchain_;
		////present_info.pImages            = &backbuffer_;
		//result = vkQueuePresentKHR(graphics_queue_, &present_info);
		//LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not present KHR queue | %s", vkErrorCode(result));
		//popMarker();

		//memory_.removeUnusedEntries();
		//foundation::GetFrameHeap()->update();
		//command_buffer_.reset();
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::draw(uint32_t instance_count)
    {
		//static int kFirst = 0;
		//if (kFirst++ == 0)
		//	return;

		//	command_buffer_.tryBegin();

		//	VulkanShader* shader = memory_.getShader(state_.shader);

		//	// Update framebuffer.
		//	///if (isDirty(DirtyState::kRenderTargets))
		//	{
		//		Framebuffer framebuffer{};
		//		framebuffer.num_framebuffers = 0;
		//		framebuffer.rt_width  = state_.rt_width;
		//		framebuffer.rt_height = state_.rt_height;

		//		for (uint32_t i = 0; i < shader->getNumRenderTargets(); ++i)
		//			framebuffer.render_targets[framebuffer.num_framebuffers++] = vk_state_.render_targets[i];

		//		if (vk_state_.depth_target)
		//			framebuffer.render_targets[framebuffer.num_framebuffers++] = vk_state_.depth_target;

		//		vk_state_.framebuffer = memory_.getFramebuffer(framebuffer);
		//		cleanDirty(DirtyState::kRenderTargets);
		//	}

		//	// Begin a render pass.
		//	VkRenderPassBeginInfo render_pass_begin_info{};
		//	render_pass_begin_info.framebuffer     = vk_state_.framebuffer->framebuffer;
		//	render_pass_begin_info.attachmentCount = (uint32_t)vk_state_.framebuffer->attachment_references.size();
		//	render_pass_begin_info.pAttachments    = vk_state_.framebuffer->attachment_references.data();
		//	vkCmdBeginRenderPass(&render_pass_begin_info);

		//	// Update viewports.
		//	///if (isDirty(DirtyState::kViewports))
		//	{
		//		vezCmdSetViewport(0, state_.num_viewports, &vk_state_.viewports[0]);
		//		vezCmdSetViewportState(state_.num_viewports);
		//		cleanDirty(DirtyState::kViewports);
		//	}

		//	// Update scissors.
		//	///if (isDirty(DirtyState::kScissorRects))
		//	{
		//		vezCmdSetScissor(0, state_.num_scissor_rects, &vk_state_.scissor_rects[0]);
		//		cleanDirty(DirtyState::kScissorRects);
		//	}

		//	// Update shader.
		//	///if (isDirty(DirtyState::kShader))
		//	{
		//		shader->bind();
		//		cleanDirty(DirtyState::kShader);
		//	}

		//	// Update textures.
		//	Vector<VulkanReflectionInfo> textures = shader->getTextures();

		//	///if (isDirty(DirtyState::kTextures))
		//	{
		//		for (const VulkanReflectionInfo& texture : textures)
		//			vezCmdBindImageView(vk_state_.textures[texture.slot], VK_NULL_HANDLE, texture.set, texture.binding, 0);
		//		cleanDirty(DirtyState::kTextures);
		//	}

		//	// Update state manager.
		//	Vector<VulkanReflectionInfo> samplers = shader->getSamplers();
		//	state_manager_.update(samplers, shader->getNumRenderTargets());

		//	// Update constant buffers.
		//	for (auto& buffer : shader->getBuffers())
		//		vezCmdBindBuffer(vk_state_.constant_buffers[buffer.slot], 0, VK_WHOLE_SIZE, buffer.set, buffer.binding, 0);

		//	VulkanMesh* mesh = memory_.getMesh(state_.mesh);

		//	// Update mesh.
		//	///if (isDirty(DirtyState::kMesh))
		//	{
		//		mesh->bind(shader->getStages(), state_.sub_mesh);
		//		cleanDirty(DirtyState::kMesh);
		//	}

		//	// Draw.
		//	mesh->draw(state_.sub_mesh);
	
		//	// Rend renderpass.
		//	vezCmdEndRenderPass();

		//	//command_buffer_.tryEnd();
		//	//command_buffer_.tryBegin();
	}
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
		state_manager_.bindRasterizerState(rasterizer_state);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setBlendState(const platform::BlendState& blend_state)
    {
		state_manager_.bindBlendState(blend_state);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
		state_manager_.bindDepthStencilState(depth_stencil_state);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
		state_manager_.bindSamplerState(sampler_state, slot);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::generateMipMaps(const asset::VioletTextureHandle& texture)
    {
		if (!texture)
			return;

		VulkanRenderTexture* t = memory_.getTexture(texture);
		if (t)
			t->getTexture()->generateMips();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::copyToScreen(const asset::VioletTextureHandle& texture)
    {
		VulkanRenderTexture* src_texture = memory_.getTexture(texture);

		copy(
			src_texture->getTexture()->getTexture(),
			glm::uvec2(src_texture->getWidth(), src_texture->getHeight()),
			backbuffer_,
			glm::uvec2(backbuffer_width_, backbuffer_height_)
		);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::copyToTexture(const asset::VioletTextureHandle& src, const asset::VioletTextureHandle& dst)
	{
		VulkanRenderTexture* src_texture = memory_.getTexture(src);
		VulkanRenderTexture* dst_texture = memory_.getTexture(dst);

		copy(
			src_texture->getTexture()->getTexture(),
			glm::uvec2(src_texture->getWidth(), src_texture->getHeight()),
			dst_texture->getTexture()->getTexture(),
			glm::uvec2(dst_texture->getWidth(), dst_texture->getHeight())
		);
	}
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
		setShader(shader_pass.getShader());

		for (uint32_t i = 0; i < shader_pass.getInputs().size(); ++i)
		{
			LMB_ASSERT(false == shader_pass.getInputs()[i].isBackBuffer(), "VULKAN: Cannot bind back buffer as input");
			setTexture(shader_pass.getInputs()[i].getTexture(), i);
		}


		// Outputs.
		Vector<VkImageView> rtvs;
		VkImageView dsv = VK_NULL_HANDLE;
		Vector<glm::vec4> viewports;
		Vector<glm::vec4> scissor_rects;

		for (auto& output : shader_pass.getOutputs())
		{
			if (output.isBackBuffer())
			{
				// TODO (Hilze): Support.
				viewports.push_back({
					0.0f,
					0.0f,
					(float)backbuffer_width_,
					(float)backbuffer_height_
				});
				rtvs.push_back(backbuffer_view_);
				scissor_rects.push_back({
					0.0f,
					0.0f,
					(float)backbuffer_width_,
					(float)backbuffer_height_
				});
			}
			else if (output.getTexture()->getLayer(0u).getFormat() == TextureFormat::kR24G8 ||
				output.getTexture()->getLayer(0u).getFormat() == TextureFormat::kD32)
			{
				auto t = memory_.getTexture(output.getTexture())->getTexture();
				dsv = t->getSubView(output.getLayer(), output.getMipMap());
			}
			else
			{
				for (const auto& input : shader_pass.getInputs())
					LMB_ASSERT(input.getName() != output.getName(), "ERR");

				auto t = memory_.getTexture(output.getTexture())->getTexture();
				rtvs.push_back(t->getSubView(output.getLayer(), output.getMipMap()));

				viewports.push_back({
					0.0f,
					0.0f,
					(float)std::max(1u, output.getTexture()->getLayer(0u).getWidth() >> output.getMipMap()),
					(float)std::max(1u, output.getTexture()->getLayer(0u).getHeight() >> output.getMipMap())
				});

				if (output.getTexture()->getLayer(0u).getFlags() &
					kTextureFlagDynamicScale)
					viewports.back() *= dynamic_resolution_scale_;

				scissor_rects.push_back({
					0u,
					0u,
					(LONG)viewports.back().z,
					(LONG)viewports.back().w
				});
			}
		}

		if (viewports.empty() && dsv)
		{
			viewports.push_back({
				0.0f,
				0.0f,
				(float)(
					shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getWidth()
					>> shader_pass.getOutputs()[0u].getMipMap()),
					(float)(
						shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getHeight()
						>> shader_pass.getOutputs()[0u].getMipMap())
			});

			if (shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getFlags()
				& kTextureFlagDynamicScale)
				viewports.back() *= dynamic_resolution_scale_;

			scissor_rects.push_back({
				0u,
				0u,
				(LONG)viewports.back().z,
				(LONG)viewports.back().w
			});
		}

		// Shader.
		setScissorRects(scissor_rects);
		setViewports(viewports);


		state_.rt_width  = (uint32_t)viewports[0].z;
		state_.rt_height = (uint32_t)viewports[0].w;

		memcpy(vk_state_.render_targets, rtvs.data(), rtvs.size() * sizeof(VkImageView));
		vk_state_.depth_target = dsv;
		
		makeDirty(DirtyState::kRenderTargets);
	}
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::clearRenderTarget(
      asset::VioletTextureHandle handle,
      const glm::vec4& colour)
    {
		/*if (!handle)
			return;

		command_buffer_.tryBegin();
		
		VulkanRenderTexture* texture = memory_.getTexture(handle);

		VkViewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = (float)texture->getWidth();
		viewport.height   = (float)texture->getHeight();

		vezCmdSetViewport(0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset.x = 0ul;
		scissor.offset.y = 0ul;
		scissor.extent.width  = texture->getWidth();
		scissor.extent.height = texture->getHeight();

		vezCmdSetScissor (0, 1, &scissor);

		Framebuffer framebuffer{};
		framebuffer.num_framebuffers  = 1;
		framebuffer.rt_width          = texture->getWidth();
		framebuffer.rt_height         = texture->getHeight();
		framebuffer.render_targets[0] = texture->getTexture()->getMainView();

		VulkanFramebuffer* fb = memory_.getFramebuffer(framebuffer);
		VezRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.framebuffer     = fb->framebuffer;
		render_pass_begin_info.attachmentCount = (uint32_t)fb->attachment_references.size();
		render_pass_begin_info.pAttachments    = fb->attachment_references.data();
		vezCmdBeginRenderPass(&render_pass_begin_info);

		VezImageSubresourceRange range{};
		range.baseArrayLayer = 0;
		range.baseMipLevel   = 0;
		range.layerCount     = 1;
		range.levelCount     = 1;
		VkClearColorValue color{};
		memcpy(color.float32, &colour.x, sizeof(colour));
		vezCmdClearColorImage(texture->getTexture()->getTexture(), &color, 1, &range);
		vezCmdEndRenderPass();

		command_buffer_.tryEnd();
		command_buffer_.tryBegin();

		makeDirty(DirtyState::kViewports);
		makeDirty(DirtyState::kScissorRects);
		makeDirty(DirtyState::kRenderTargets);*/
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setScissorRects(const Vector<glm::vec4>& rects)
    {
		bool dirty = false;

		if (state_.num_scissor_rects != rects.size())
			dirty = true;

		for (uint32_t i = 0; i < rects.size() && !dirty; ++i)
			if (rects[i] != state_.scissor_rects[i])
				dirty = true;

		if (dirty)
		{
			state_.num_scissor_rects = (uint32_t)rects.size();
			for (uint32_t i = 0; i < state_.num_scissor_rects; ++i)
			{
				state_.scissor_rects[i] = rects[i];
				vk_state_.scissor_rects[i].offset.x      = (uint32_t)rects[i].x;
				vk_state_.scissor_rects[i].offset.y      = (uint32_t)rects[i].y;
				vk_state_.scissor_rects[i].extent.width  = (uint32_t)rects[i].z;
				vk_state_.scissor_rects[i].extent.height = (uint32_t)rects[i].w;
			}
			makeDirty(DirtyState::kScissorRects);
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setViewports(const Vector<glm::vec4>& rects)
    {
		bool dirty = false;

		if (state_.num_viewports != rects.size())
			dirty = true;

		for (uint32_t i = 0; i < rects.size() && !dirty; ++i)
			if (rects[i] != state_.viewports[i])
				dirty = true;

		if (dirty)
		{
			state_.num_viewports = (uint32_t)rects.size();
			for (uint32_t i = 0; i < state_.num_viewports; ++i)
			{
				state_.viewports[i] = rects[i];
				vk_state_.viewports[i].x        = rects[i].x;
				vk_state_.viewports[i].y        = rects[i].y;
				vk_state_.viewports[i].width    = rects[i].z;
				vk_state_.viewports[i].height   = rects[i].w;
				vk_state_.viewports[i].minDepth = 0.0f;
				vk_state_.viewports[i].maxDepth = 1.0f;
			}
			makeDirty(DirtyState::kViewports);
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setMesh(asset::VioletMeshHandle mesh)
    {
		if (!mesh)
			return;

		if (mesh != state_.mesh)
		{
			state_manager_.bindTopology(mesh->getTopology());
			state_.mesh = mesh;
			makeDirty(DirtyState::kMesh);
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setSubMesh(const uint32_t& sub_mesh_idx)
    {
		if (sub_mesh_idx != state_.sub_mesh)
		{
			state_.sub_mesh = sub_mesh_idx;
			makeDirty(DirtyState::kMesh);
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setShader(asset::VioletShaderHandle shader)
    {
      if (!shader)
        return;

	  if (shader != state_.shader)
	  {
        makeDirty(DirtyState::kShader);
		state_.shader    = shader;
		vk_state_.shader = memory_.getShader(shader);
	  }
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setTexture(
      asset::VioletTextureHandle texture, 
      uint8_t slot)
    {
	  if (texture != state_.textures[slot])
	  {
		if (slot == 0)
		{
			glm::vec2 val(1.0f);
			if (texture)
				val = 1.0f / glm::vec2((float)texture->getLayer(0u).getWidth(), (float)texture->getLayer(0u).getHeight());

			setUserData(glm::vec4(val.x, val.y, 0.0f, 0.0f), 15);
		}

        makeDirty(DirtyState::kTextures);
        state_.textures[slot]    = texture;
		vk_state_.textures[slot] = memory_.getTexture(texture ? texture : default_texture_)->getTexture()->getMainView();
	  }
    }

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setConstantBuffer(platform::IRenderBuffer* constant_buffer, uint8_t slot)
	{
		if (state_.constant_buffers[slot] == constant_buffer)
			return;

		state_.constant_buffers[slot] = constant_buffer;
		vk_state_.constant_buffers[slot] = constant_buffer ? ((VulkanRenderBuffer*)constant_buffer)->getBuffer() : nullptr;
		makeDirty(DirtyState::kConstantBuffers);

		for (int i = 0; i < (int)ShaderStages::kCount; ++i)
			state_.dirty_constant_buffers[i] |= 1ull << slot;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setUserData(glm::vec4 user_data, uint8_t slot)
	{
		if (cbs_.user_data[slot] != user_data)
		{
			cbs_.user_data[slot] = user_data;

			if (!cbs_.cb_user_data)
				cbs_.cb_user_data = (VulkanRenderBuffer*)allocRenderBuffer(sizeof(cbs_.user_data), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic, nullptr);

			memcpy(cbs_.cb_user_data->lock(), cbs_.user_data, sizeof(cbs_.user_data));
			cbs_.cb_user_data->unlock();

			setConstantBuffer(cbs_.cb_user_data, cbUserDataIdx);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setRenderTargets(Vector<asset::VioletTextureHandle> render_targets, asset::VioletTextureHandle depth_buffer)
	{
		bool equal = false;
		for (uint32_t i = 0; i < render_targets.size(); ++i)
			if (render_targets[i] != state_.render_targets[i])
				equal = false;

		if (depth_buffer)
			if (depth_buffer != state_.depth_target)
				equal = false;

		if (!equal)
		{
			for (uint32_t i = 0; i < render_targets.size(); ++i)
			{
				state_.render_targets[i]    = render_targets[i];
				vk_state_.render_targets[i] = memory_.getTexture(render_targets[i])->getTexture()->getMainView();
			}

			state_.depth_target = depth_buffer;
			vk_state_.depth_target = depth_buffer ? memory_.getTexture(depth_buffer)->getTexture()->getMainView() : VK_NULL_HANDLE;


			if (render_targets.size() > 0)
			{
				state_.rt_width  = render_targets[0]->getLayer(0u).getWidth();
				state_.rt_height = render_targets[0]->getLayer(0u).getHeight();
			}
			else if (depth_buffer)
			{
				state_.rt_width  = depth_buffer->getLayer(0u).getWidth();
				state_.rt_height = depth_buffer->getLayer(0u).getHeight();
			}
			
			makeDirty(DirtyState::kRenderTargets);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	static const uint32_t kColorCount = 12u;
	static float kColors[kColorCount][4u] = {
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.5f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.5f, 1.0f, 0.0f, 1.0f },

		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.5f, 1.0f },
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.5f, 1.0f, 1.0f },

		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.5f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.5f, 1.0f }
	};

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::pushMarker(const String& name)
    {
#if VIOLET_USE_GPU_MARKERS
		if (has_debug_markers_)
		{
			VkDebugMarkerMarkerInfoEXT marker_info{};
			marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(marker_info.color, kColors[marker_depth_++ % kColorCount], sizeof(float) * 4u);
			debug_marker_begin_(command_buffer_.getActiveCommandBuffer(), &marker_info);
		}
#endif
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setMarker(const String& name)
    {
#if VIOLET_USE_GPU_MARKERS
		if (has_debug_markers_)
		{
			VkDebugMarkerMarkerInfoEXT marker_info{};
			marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(marker_info.color, kColors[marker_depth_ % kColorCount], sizeof(float) * 4u);
			debug_marker_insert_(command_buffer_.getActiveCommandBuffer(), &marker_info);
		}
#endif
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::popMarker()
    {
#if VIOLET_USE_GPU_MARKERS
		if (has_debug_markers_)
		{
			debug_marker_end_(command_buffer_.getActiveCommandBuffer());
			marker_depth_--;
		}
#endif
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::beginTimer(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::endTimer(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    uint64_t VulkanRenderer::getTimerMicroSeconds(const String& name)
    {
      return 0ul;
    }
    
	///////////////////////////////////////////////////////////////////////////
	VkDevice VulkanRenderer::getDevice() const
	{
		return device_manager_.getDevice();
	}

	///////////////////////////////////////////////////////////////////////////
	VkCommandBuffer VulkanRenderer::getCommandBuffer() const
	{
		return command_buffer_.getActiveCommandBuffer();
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setRenderScale(const float& render_scale)
    {
		if (render_scale != render_scale_)
		{
			render_scale_ = render_scale;
			resize();
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::destroyTexture(const size_t& hash)
	{
		memory_.removeTexture(hash);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::destroyShader(const size_t& hash)
	{
		memory_.removeShader(hash);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::destroyMesh(const size_t& hash)
	{
		memory_.removeMesh(hash);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::createCommandBuffer()
	{
		// Get the graphics queue handle.
		//vezGetDeviceGraphicsQueue(device_, 0, &graphics_queue_);

		//command_buffer_.create(device_, graphics_queue_);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::copy(VkImage src, glm::uvec2 src_size, VkImage dst, glm::uvec2 dst_size)
	{
		/*command_buffer_.tryBegin();
		
		VezImageBlit region{};
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = 1;
		region.srcSubresource.mipLevel = 0;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = 1;
		region.dstSubresource.mipLevel = 0;
		region.srcOffsets[0].x = 0;
		region.srcOffsets[0].y = 0;
		region.srcOffsets[0].z = 0;
		region.srcOffsets[1].x = src_size.x;
		region.srcOffsets[1].y = src_size.y;
		region.srcOffsets[1].z = 1;
		region.dstOffsets[0].x = 0;
		region.dstOffsets[0].y = 0;
		region.dstOffsets[0].z = 0;
		region.dstOffsets[1].x = dst_size.x;
		region.dstOffsets[1].y = dst_size.y;
		region.dstOffsets[1].z = 1;

		vezCmdBlitImage(
			src,
			dst,
			1,
			&region,
			VK_FILTER_NEAREST
		);

		command_buffer_.tryEnd();
		command_buffer_.tryBegin();*/
	}

	///////////////////////////////////////////////////////////////////////////
	scene::Scene* VulkanRenderer::getScene() const
	{
		return override_scene_ ? override_scene_ : scene_;
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setVSync(bool vsync)
    {
		vsync_ = vsync;
		//vezSwapchainSetVSync(swapchain_, vsync);
    }

    ///////////////////////////////////////////////////////////////////////////
    float VulkanRenderer::getRenderScale()
    {
		return render_scale_;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool VulkanRenderer::getVSync() const
    {
      return vsync_;
    }

	///////////////////////////////////////////////////////////////////////////
	VulkanRenderBuffer::VulkanRenderBuffer(uint32_t size, uint32_t flags, VkBuffer buffer, VulkanRenderer* renderer)
		: data_(nullptr)
		, size_(size)
		, flags_(flags)
		, buffer_(buffer)
		, renderer_(renderer)
	{
		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(renderer_->getDevice(), buffer_, &memory_requirements);
		gpu_size_ = (size_t)memory_requirements.size;
	}

	///////////////////////////////////////////////////////////////////////////
	void* VulkanRenderBuffer::lock()
	{
		LMB_ASSERT(!data_, "VULKAN: Buffer was already locked");
		data_ = foundation::Memory::allocate(size_);
		return data_;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderBuffer::unlock()
	{
		LMB_ASSERT(data_, "VULKAN: Buffer was not locked");
		//VkResult result;
		//result = vezBufferSubData(renderer_->getDevice(), buffer_, 0, size_, data_);
		//LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not update buffer data | %s", vkErrorCode(result));
		foundation::Memory::deallocate(data_);
		data_ = nullptr;
	}

	///////////////////////////////////////////////////////////////////////////
	uint32_t VulkanRenderBuffer::getFlags() const
	{
		return flags_;
	}

	///////////////////////////////////////////////////////////////////////////
	uint32_t VulkanRenderBuffer::getSize() const
	{
		return size_;
	}

	///////////////////////////////////////////////////////////////////////////
	size_t VulkanRenderBuffer::getGPUSize() const
	{
		return gpu_size_;
	}

	///////////////////////////////////////////////////////////////////////////
	VkBuffer VulkanRenderBuffer::getBuffer() const
	{
		return buffer_;
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanRenderTexture::VulkanRenderTexture(uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_count, TextureFormat format, uint32_t flags, VulkanTexture * texture, VulkanRenderer * renderer)
		: data_(nullptr)
		, width_(width)
		, height_(height)
		, depth_(depth)
		, mip_count_(mip_count)
		, format_(format)
		, flags_(flags)
		, texture_(texture)
		, renderer_(renderer)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	void* VulkanRenderTexture::lock(uint32_t level)
	{
		LMB_ASSERT(false, "VULKAN: Not yet implemented");
		return nullptr;

		/*uint32_t bpp, bpr, bpl;
		calculateImageMemory(w, h, l, bpp, bpr, bpl);
		LMB_ASSERT(!data_, "VULKAN: Texty was already locked");
		data_ = foundation::Memory::allocate(bpl);
		return data_;*/
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderTexture::unlock(uint32_t level)
	{
		LMB_ASSERT(false, "VULKAN: Not yet implemented");
		/*LMB_ASSERT(data_, "VULKAN: Buffer was not locked");
		VezImageSubDataInfo sub_data_info{};
		sub_data_info;
		vezImageSubData(renderer_->getDevice(), texture_->getImage(), &sub_data_info, data_);
		foundation::Memory::deallocate(data_);
		data_ = nullptr;*/
	}

	///////////////////////////////////////////////////////////////////////////
	uint32_t VulkanRenderTexture::getWidth() const
	{
		return width_;
	}

	///////////////////////////////////////////////////////////////////////////
	uint32_t VulkanRenderTexture::getHeight() const
	{
		return height_;
	}

	///////////////////////////////////////////////////////////////////////////
	uint32_t VulkanRenderTexture::getDepth() const
	{
		return depth_;
	}

	///////////////////////////////////////////////////////////////////////////
	uint32_t VulkanRenderTexture::getMipCount() const
	{
		return mip_count_;
	}

	///////////////////////////////////////////////////////////////////////////
	uint32_t VulkanRenderTexture::getFlags() const
	{
		return flags_;
	}
	
	///////////////////////////////////////////////////////////////////////////
	TextureFormat VulkanRenderTexture::getFormat() const
	{
		return format_;
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanTexture* VulkanRenderTexture::getTexture() const
	{
		return texture_;
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanCommandBuffer::VulkanCommandBuffer()
		: recording(false)
		, current_command_buffer(-1)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::create(VkDevice device, VkQueue graphics_queue)
	{
		this->device = device;
		this->graphics_queue = graphics_queue;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::destroy(VkDevice device)
	{
		LMB_ASSERT(!recording, "VULKAN: Command buffer was already in recording state");
		//vezFreeCommandBuffers(device, (uint32_t)command_buffers.size(), command_buffers.data());
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::begin()
	{
		LMB_ASSERT(!recording, "VULKAN: Command buffer was already in recording state");
		recording = true;
	
		/*if (++current_command_buffer >= command_buffers.size())
		{
			VkResult result;
			VezCommandBufferAllocateInfo cb_alloc_info{};
			cb_alloc_info.queue              = graphics_queue;
			cb_alloc_info.commandBufferCount = 1;
			command_buffers.push_back(VK_NULL_HANDLE);
			result = vezAllocateCommandBuffers(device, &cb_alloc_info, &command_buffers.back());
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not allocate command buffer | %s", vkErrorCode(result));
		}

		VkResult result;
		result = vezBeginCommandBuffer(command_buffers[current_command_buffer], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not begin command buffer | %s", vkErrorCode(result));*/
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::end()
	{
		LMB_ASSERT(recording, "VULKAN: Command buffer was not in recording state");
		recording = false;

		//VkResult result;
		//result = vezEndCommandBuffer();
		//LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not end command buffer | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	bool VulkanCommandBuffer::tryBegin()
	{
		if (!recording)
		{
			begin();
			return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	bool VulkanCommandBuffer::tryEnd()
	{
		if (recording)
		{
			end();
			return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	bool VulkanCommandBuffer::isRecording() const
	{
		return recording;
	}

	///////////////////////////////////////////////////////////////////////////
	VkCommandBuffer VulkanCommandBuffer::getActiveCommandBuffer() const
	{
		return command_buffers[current_command_buffer];
	}

	///////////////////////////////////////////////////////////////////////////
	Vector<VkCommandBuffer> VulkanCommandBuffer::getCommandBuffers() const
	{
		return command_buffers;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::reset()
	{
		current_command_buffer = -1;
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanShader* VulkanRenderer::Memory::getShader(asset::VioletShaderHandle handle)
	{
		if (!handle)
			return nullptr;

		auto it = shaders.find(handle.getHash());
		if (it == shaders.end())
		{
			VulkanShader* shader = foundation::Memory::construct<VulkanShader>(handle, renderer);
			Entry<VulkanShader*> entry;
			entry.ptr = shader;
			entry.keep_in_memory = handle->getKeepInMemory();
			shaders.insert(eastl::make_pair(handle.getHash(), entry));
			it = shaders.find(handle.getHash());
		}

		it->second.hit_count++;
		return it->second.ptr;
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanRenderTexture* VulkanRenderer::Memory::getTexture(asset::VioletTextureHandle handle)
	{
		if (!handle)
			return nullptr;

		auto it = textures.find(handle.getHash());
		if (it == textures.end())
		{
			size_t hash = handle.getHash();
			VulkanRenderTexture* texture = (VulkanRenderTexture*)renderer->allocRenderTexture(handle);
			Entry<VulkanRenderTexture*> entry;
			entry.ptr = texture;
			entry.keep_in_memory = handle->getKeepInMemory();
			textures.insert(eastl::make_pair(handle.getHash(), entry));
			it = textures.find(handle.getHash());
		}

		it->second.hit_count++;
		return it->second.ptr;
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanMesh* VulkanRenderer::Memory::getMesh(asset::VioletMeshHandle handle)
	{
		if (!handle)
			return nullptr;

		auto it = meshes.find(handle.getHash());
		if (it == meshes.end())
		{
			Entry<VulkanMesh*> entry;
			entry.ptr = foundation::Memory::construct<VulkanMesh>(handle, renderer);
			entry.keep_in_memory = false;
			meshes.insert(eastl::make_pair(handle.getHash(), entry));
			it = meshes.find(handle.getHash());
		}

		it->second.hit_count++;
		return it->second.ptr;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeShader(asset::VioletShaderHandle handle)
	{
		removeShader(handle.getHash());
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeTexture(asset::VioletTextureHandle handle)
	{
		removeTexture(handle.getHash());
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeMesh(asset::VioletMeshHandle handle)
	{
		removeMesh(handle.getHash());
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeShader(size_t handle)
	{
		auto it = shaders.find(handle);
		if (it != shaders.end())
		{
			foundation::Memory::destruct(it->second.ptr);
			shaders.erase(it);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeTexture(size_t handle)
	{
		auto it = textures.find(handle);
		if (it != textures.end())
		{
			renderer->freeRenderTexture((platform::IRenderTexture*&)it->second.ptr);
			textures.erase(it);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeMesh(size_t handle)
	{
		auto it = meshes.find(handle);
		if (it != meshes.end())
		{
			foundation::Memory::destruct(it->second.ptr);
			meshes.erase(it);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeUnusedEntries()
	{
		for (auto& texture : textures)
			if (texture.second.hit_count == 0 && !texture.second.keep_in_memory)
				removeTexture(texture.first);
			else
				texture.second.hit_count = 0;
		for (auto& shader : shaders)
			if (shader.second.hit_count == 0 && !shader.second.keep_in_memory)
				removeShader(shader.first);
			else
				shader.second.hit_count = 0;
		for (auto& mesh : meshes)
			if (mesh.second.hit_count == 0 && !mesh.second.keep_in_memory)
				removeMesh(mesh.first);
			else
				mesh.second.hit_count = 0;
	}
	
	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeAllEntries()
	{
		for (auto& texture : textures)
			removeTexture(texture.first);
		for (auto& shader : shaders)
			removeShader(shader.first);
		for (auto& mesh : meshes)
			removeMesh(mesh.first);
	}
  }
}