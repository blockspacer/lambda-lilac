#include "vulkan_renderer.h"
#include <interfaces/iworld.h>
#include "vulkan_texture.h"
#include "vulkan_shader.h"
#include "vulkan_mesh.h"
#include "vulkan_state_manager.h"
#include "memory/frame_heap.h"

namespace lambda
{
  namespace linux
  {
    const char* vkErrorCode(const VkResult& result)
	{
      switch(result)
	  {
	  case VK_SUCCESS:                        return "VK_SUCCESS";
	  case VK_NOT_READY:                      return "VK_NOT_READY";
	  case VK_TIMEOUT:                        return "VK_TIMEOUT";
	  case VK_EVENT_SET:                      return "VK_EVENT_SET";
	  case VK_EVENT_RESET:                    return "VK_EVENT_RESET";
	  case VK_INCOMPLETE:                     return "VK_INCOMPLETE";
	  case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
	  case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	  case VK_ERROR_INITIALIZATION_FAILED:    return "VK_ERROR_INITIALIZATION_FAILED";
	  case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
	  case VK_ERROR_MEMORY_MAP_FAILED:        return "VK_ERROR_MEMORY_MAP_FAILED";
	  case VK_ERROR_LAYER_NOT_PRESENT:        return "VK_ERROR_LAYER_NOT_PRESENT";
	  case VK_ERROR_EXTENSION_NOT_PRESENT:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
	  case VK_ERROR_FEATURE_NOT_PRESENT:      return "VK_ERROR_FEATURE_NOT_PRESENT";
	  case VK_ERROR_INCOMPATIBLE_DRIVER:      return "VK_ERROR_INCOMPATIBLE_DRIVER";
	  case VK_ERROR_TOO_MANY_OBJECTS:         return "VK_ERROR_TOO_MANY_OBJECTS";
	  case VK_ERROR_FORMAT_NOT_SUPPORTED:     return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	  case VK_ERROR_FRAGMENTED_POOL:          return "VK_ERROR_FRAGMENTED_POOL";
	  case VK_ERROR_OUT_OF_POOL_MEMORY:       return "VK_ERROR_OUT_OF_POOL_MEMORY";
	  case VK_ERROR_INVALID_EXTERNAL_HANDLE:  return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	  case VK_ERROR_SURFACE_LOST_KHR:         return "VK_ERROR_SURFACE_LOST_KHR";
	  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	  case VK_SUBOPTIMAL_KHR:                 return "VK_SUBOPTIMAL_KHR";
	  case VK_ERROR_OUT_OF_DATE_KHR:          return "VK_ERROR_OUT_OF_DATE_KHR";
	  case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	  case VK_ERROR_VALIDATION_FAILED_EXT:    return "VK_ERROR_VALIDATION_FAILED_EXT";
	  case VK_ERROR_INVALID_SHADER_NV:        return "VK_ERROR_INVALID_SHADER_NV";
	  case VK_ERROR_FRAGMENTATION_EXT:        return "VK_ERROR_FRAGMENTATION_EXT";
	  case VK_ERROR_NOT_PERMITTED_EXT:        return "VK_ERROR_NOT_PERMITTED_EXT";
	  default:                                return "UNKNOWN_ERROR_CODE";
	  }
	}

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
      );
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::freeRenderBuffer(platform::IRenderBuffer* buffer)
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
			memory_stats_.vertex -= buffer->getSize();
		if (is_index)
			memory_stats_.index -= buffer->getSize();
		if (is_constant)
			memory_stats_.constant -= buffer->getSize();

		vezDestroyBuffer(device_, static_cast<VulkanRenderBuffer*>(buffer)->getBuffer());
		foundation::Memory::destruct(buffer);
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

		uint32_t size = 0u;
		for (uint32_t i = 0u; i < texture->getLayerCount(); ++i)
		{
			uint32_t w = texture->getLayer(0u).getWidth();
			uint32_t h = texture->getLayer(0u).getHeight();
			uint32_t bpp, bpr, bpl;
			for (uint32_t j = 0u; j < texture->getLayer(0u).getMipCount(); ++j)
			{
				calculateImageMemory(
					texture->getLayer(0u).getFormat(),
					w,
					h,
					bpp,
					bpr,
					bpl
				);
				size += bpl;
				w /= 2u;
				h /= 2u;
			}
		}
		if ((texture->getLayer(0u).getFlags() & kTextureFlagIsRenderTarget)
			!= 0u)
			memory_stats_.render_target += size;
		else
			memory_stats_.texture += size;

		return vulkan_texture;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::freeRenderTexture(platform::IRenderTexture* texture)
	{
		uint32_t size = 0u;
		for (uint32_t i = 0u; i < texture->getDepth(); ++i)
		{
			uint32_t w = texture->getWidth();
			uint32_t h = texture->getHeight();
			uint32_t bpp, bpr, bpl;
			for (uint32_t j = 0u; j < texture->getMipCount(); ++j)
			{
				calculateImageMemory(texture->getFormat(), w, h, bpp, bpr, bpl);
				size += bpl;
				w /= 2u;
				h /= 2u;
			}
		}
		if ((texture->getFlags() & kTextureFlagIsRenderTarget) != 0u)
			memory_stats_.render_target -= size;
		else
			memory_stats_.texture -= size;

		foundation::Memory::destruct(
			((VulkanRenderTexture*)texture)->getTexture()
		);
		foundation::Memory::destruct(texture);
	}

	///////////////////////////////////////////////////////////////////////////
	VulkanRenderer::VulkanRenderer()
	  : world_(nullptr)
	  , vsync_(false)
	  , instance_(VK_NULL_HANDLE)
	  , physical_device_(VK_NULL_HANDLE)
	  , surface_(VK_NULL_HANDLE)
	  , device_(VK_NULL_HANDLE)
	  , swapchain_(VK_NULL_HANDLE)
	  , graphics_queue_(VK_NULL_HANDLE)
	  , command_buffer_()
	{
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
    void VulkanRenderer::initialize(world::IWorld* world)
    {
		world_ = world;
		
		full_screen_quad_.mesh =
			asset::AssetManager::getInstance().createAsset<asset::Mesh>(
				Name("__full_screen_quad_mesh__"),
				foundation::Memory::constructShared<asset::Mesh>(
					asset::Mesh::createScreenQuad()
					)
				);

		full_screen_quad_.shader = asset::ShaderManager::getInstance()->get(Name("resources/shaders/full_screen_quad.fx"));
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::deinitialize()
    {
		vezDestroySwapchain(device_, swapchain_);
		vezDestroyDevice(device_);
		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		vezDestroyInstance(instance_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setWindow(
      foundation::SharedPointer<platform::IWindow> window)
    {
		// Create instance.
		VezApplicationInfo application_info = {};
		application_info.pApplicationName   = "MyApplication";
		application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.pEngineName        = "Lambda Lilac";
		application_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

		Vector<const char*> enabled_layers = { "VK_LAYER_LUNARG_standard_validation" };
		Vector<const char*> enabled_extensions = { VK_KHR_SURFACE_EXTENSION_NAME };

#if VIOLET_WIN32
		enabled_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
#pragma error "Currently unsupported platform."
#endif

		VezInstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo        = &application_info;
		instance_create_info.enabledLayerCount       = static_cast<uint32_t>(enabled_layers.size());
		instance_create_info.ppEnabledLayerNames     = enabled_layers.data();
		instance_create_info.enabledExtensionCount   = static_cast<uint32_t>(enabled_extensions.size());
		instance_create_info.ppEnabledExtensionNames = enabled_extensions.data();
		VkResult result = vezCreateInstance(&instance_create_info, &instance_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to create instance | %s", vkErrorCode(result));

		// Get physical device.
		uint32_t physical_device_count;
		vezEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr);
		
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vezEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data());
		
		for (VkPhysicalDevice physical_device : physical_devices)
		{
		  VkPhysicalDeviceProperties properties;
		  vezGetPhysicalDeviceProperties(physical_device, &properties);
		
		  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		  {
			  physical_device_ = physical_device;
			  break;
		  }
		}

		LMB_ASSERT(physical_device_ != VK_NULL_HANDLE, "VULKAN: Could not find physical device");

		// Create surface.
#if VIOLET_WIN32
		VkWin32SurfaceCreateInfoKHR surface_create_info {};
		surface_create_info.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_create_info.hwnd      = (HWND)window->getWindow();
		surface_create_info.hinstance = GetModuleHandle(NULL);

		result = vkCreateWin32SurfaceKHR(instance_, &surface_create_info, nullptr, &surface_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create surface | %s", vkErrorCode(result));
#endif

		// Create device.
		enabled_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VezDeviceCreateInfo device_create_info{};
		device_create_info.enabledExtensionCount   = static_cast<uint32_t>(enabled_extensions.size());
		device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
		result = vezCreateDevice(physical_device_, &device_create_info, &device_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create device | %s", vkErrorCode(result));

	    // Create swapchain.
	    VezSwapchainCreateInfo swapchain_create_info{};
		swapchain_create_info.surface      = surface_;
		swapchain_create_info.format       = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		swapchain_create_info.tripleBuffer = VK_TRUE;
		result = vezCreateSwapchain(device_, &swapchain_create_info, &swapchain_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create swapchain | %s", vkErrorCode(result));

		createCommandBuffer();
		createTest();

		// Set VSync.
		setVSync(getVSync());

		state_manager_.initialize(this);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::resize()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::update(const double& delta_time)
    {
		delta_time_ = delta_time;
		total_time_ += delta_time_;
		setShaderVariable(
			platform::ShaderVariable(Name("delta_time"), (float)delta_time_)
		);
		setShaderVariable(
			platform::ShaderVariable(Name("total_time"), (float)total_time_)
		);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::startFrame()
    {
		command_buffer_.tryBegin();

		// Clear everything.
		beginTimer("Clear Everything");
		pushMarker("Clear Everything");
		glm::vec4 colour(0.0f);
		for (const auto& target : world_->getPostProcessManager().getAllTargets())
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
	}
   
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::endFrame(bool display)
    {
		setMesh(full_screen_quad_.mesh);
		setSubMesh(0u);
		setRasterizerState(platform::RasterizerState::SolidBack());
		setBlendState(platform::BlendState::Default());
		setDepthStencilState(platform::DepthStencilState::Default());

		beginTimer("Post Processing");
		pushMarker("Post Processing");
		for (auto& pass : world_->getPostProcessManager().getPasses())
		{
			if (pass.getEnabled())
			{
				pushMarker(pass.getName().getName());
				bindShaderPass(pass);
				draw();
				popMarker();
			}
			else
			{
				setMarker(pass.getName().getName() + " - Disabled");
			}
		}
		popMarker();
		endTimer("Post Processing");

		beginTimer("Debug Rendering");
		pushMarker("Debug Rendering");
		world_->getDebugRenderer().Render(world_);
		popMarker();
		endTimer("Debug Rendering");

		beginTimer("Copy To Screen");
		pushMarker("Copy To Screen");

		setBlendState(platform::BlendState::Alpha());

		copyToScreen(
			world_->getPostProcessManager().getTarget(
				world_->getPostProcessManager().getFinalTarget()
			).getTexture()
		);
		copyToScreen(
			world_->getPostProcessManager().getTarget(Name("gui")).getTexture()
		);
		popMarker();
		endTimer("Copy To Screen");
		
		pushMarker("Present");
		VkResult result;

		// End command buffer recording.
		command_buffer_.end();
		VkCommandBuffer command_buffer = command_buffer_.getCommandBuffer();

		// Submit the command buffer to the graphics queue.
		VezSubmitInfo submit_info{};
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers    = &command_buffer;

		// Request a wait semaphore to pass to present so it waits for rendering to complete.
		VkSemaphore semaphore = VK_NULL_HANDLE;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores    = &semaphore;
		result = vezQueueSubmit(graphics_queue_, 1, &submit_info, nullptr);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not submit queue | %s", vkErrorCode(result));

		vezDeviceWaitIdle(device_);

		// Present the swapchain framebuffer to the window.
		VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VezPresentInfo present_info{};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &semaphore;
		present_info.pWaitDstStageMask  = &wait_dst_stage_mask;
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = &swapchain_;
		//present_info.pImages            = &frame_buffer_.color_image;
		result = vezQueuePresent(graphics_queue_, &present_info);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not present KHR queue | %s", vkErrorCode(result));
		popMarker();

		memory_.removeUnusedEntries();
		foundation::GetFrameHeap()->update();
	}
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::draw()
    {
		if (!state_.shader)
			return;

		command_buffer_.tryBegin();

		// Set the viewport state and dimensions.
		uint32_t width  = world_->getWindow()->getSize().x;
		uint32_t height = world_->getWindow()->getSize().y;
		VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
		VkRect2D   scissor  = { { 0, 0 },{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
		vezCmdSetViewport(0, 1, &viewport);
		vezCmdSetScissor(0, 1, &scissor);
		vezCmdSetViewportState(1);

		if (state_.dirty_framebuffer)
			updateFramebuffer();


		// Define clear values for the swapchain's color and depth attachments.
		Vector<VezAttachmentReference> attachment_references = getAttachmentReferences();

		// Begin a render pass.
		VezRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.framebuffer     = state_.framebuffer;
		render_pass_begin_info.attachmentCount = (uint32_t)attachment_references.size();
		render_pass_begin_info.pAttachments    = attachment_references.data();
		vezCmdBeginRenderPass(&render_pass_begin_info);

		Vector<VulkanReflectionInfo> textures = state_.shader->getTextures();
		Vector<VulkanReflectionInfo> samplers = state_.shader->getSamplers();

		if (state_.dirty_textures)
		{
			for (uint16_t i = 0; i < 16 && state_.dirty_textures != 0; ++i)
			{
				if ((state_.dirty_textures & (1 << i)))
				{
					VkImageView view = state_.textures[i]->getTexture()->getMainView();
					for (const VulkanReflectionInfo& texture : textures)
					{
						if (texture.binding == i)
						{
							vezCmdBindImageView(view, test_.sampler, texture.set, texture.binding, 0);
						}
					}

					state_.dirty_textures &= ~(1 << i);
				}
			}
		}

		state_manager_.update(samplers);

		vezCmdEndRenderPass();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::drawInstanced(const Vector<glm::mat4>& matrices)
    {
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
		LMB_ASSERT(false, "VULKAN: copyToScreen not implemented");
    }

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::copyToTexture(const asset::VioletTextureHandle& src, const asset::VioletTextureHandle& dst)
	{
		LMB_ASSERT(false, "VULKAN: copyToTexture not implemented");
	}
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
		float dynamic_resolution_scale =
			world_->getShaderVariableManager().getShaderVariable(
				Name("dynamic_resolution_scale")
			).data.at(0);

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
				/*viewports.push_back(default_.viewport);
				rtvs.push_back(context_.backbuffer.Get());
				scissor_rects.push_back({
					0u,
					0u,
					(LONG)default_.viewport.z,
					(LONG)default_.viewport.w
				});*/
			}
			else if (output.getTexture()->getLayer(0u).getFormat() == TextureFormat::kR24G8 ||
				output.getTexture()->getLayer(0u).getFormat() == TextureFormat::kD32)
			{
				auto t = memory_.getTexture(output.getTexture())->getTexture();
				dsv = t->getSubView(t->pingPongIdx(), output.getLayer(), output.getMipMap());
			}
			else
			{
				for (const auto& input : shader_pass.getInputs())
					if (input.getName() == output.getName())
						memory_.getTexture(output.getTexture())->getTexture()->pingPong();

				auto t = memory_.getTexture(output.getTexture())->getTexture();
				rtvs.push_back(t->getSubView(t->pingPongIdx(), output.getLayer(), output.getMipMap()));

				viewports.push_back({
					0.0f,
					0.0f,
					(float)(output.getTexture()->getLayer(0u).getWidth()
					>> output.getMipMap()),
						(float)(output.getTexture()->getLayer(0u).getHeight()
							>> output.getMipMap())
				});

				if (output.getTexture()->getLayer(0u).getFlags() &
					kTextureFlagDynamicScale)
					viewports.back() *= dynamic_resolution_scale;

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
				viewports.back() *= dynamic_resolution_scale;

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
		setRenderTargets(rtvs, dsv, (uint32_t)viewports[0].z, (uint32_t)viewports[0].w);
	}
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::clearRenderTarget(
      asset::VioletTextureHandle texture, 
      const glm::vec4& colour)
    {
		LMB_ASSERT(false, "VULKAN: clearRenderTarget not implemented");
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setScissorRects(const Vector<glm::vec4>& rects)
    {
		LMB_ASSERT(false, "VULKAN: setScissorRect not implemented");
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setViewports(const Vector<glm::vec4>& rects)
    {
		LMB_ASSERT(false, "VULKAN: setViewports not implemented");
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setMesh(asset::MeshHandle mesh)
    {
		LMB_ASSERT(false, "VULKAN: setMesh not implemented");
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setSubMesh(const uint32_t& sub_mesh_idx)
    {
		LMB_ASSERT(false, "VULKAN: setSubMesh not implemented");
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setShader(asset::VioletShaderHandle shader)
    {
		if (!shader)
			return;

		VulkanShader* s = memory_.getShader(shader);

		if (s != state_.shader)
		{
			state_.dirty_shader = true;
			state_.shader = s;
		}
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setTexture(
      asset::VioletTextureHandle texture, 
      uint8_t slot)
    {
		if (!texture)
			return;

		VulkanRenderTexture* t = memory_.getTexture(texture);

		if (t != state_.textures[slot])
		{
			state_.dirty_textures |= (1 << slot);
			state_.textures[slot] = t;
		}
    }

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setRenderTargets(Vector<asset::VioletTextureHandle> render_targets, asset::VioletTextureHandle depth_buffer)
	{
		Vector<VkImageView> iv_rt;
		for (uint32_t i = 0; i < render_targets.size(); ++i)
			iv_rt.push_back(memory_.getTexture(render_targets[i])->getTexture()->getMainView());

		VulkanRenderTexture* dt = memory_.getTexture(depth_buffer);
		VkImageView iv_dt = dt ? dt->getTexture()->getMainView() : VK_NULL_HANDLE;

		uint32_t width, height;

		if (render_targets.size() > 0)
		{
			width  = render_targets[0]->getLayer(0u).getWidth();
			height = render_targets[0]->getLayer(0u).getHeight();
		}
		else if (depth_buffer)
		{
			width  = depth_buffer->getLayer(0u).getWidth();
			height = depth_buffer->getLayer(0u).getHeight();
		}

		setRenderTargets(iv_rt, iv_dt, width, height);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setRenderTargets(
		Vector<VkImageView> render_targets,
		VkImageView depth_buffer,
		uint32_t width,
		uint32_t height
	)
	{
		bool equal = true;
		for (uint32_t i = 0; i < render_targets.size(); ++i)
			if (render_targets[i] != state_.render_targets[i])
				equal = false;

		if (depth_buffer != state_.depth_target)
			equal = false;

		if (!equal)
		{
			for (uint32_t i = 0; i < render_targets.size(); ++i)
				state_.render_targets[i] = render_targets[i];

			state_.depth_target = depth_buffer;

			state_.rt_width = width;
			state_.rt_height = height;

			state_.dirty_framebuffer = true;
		}
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::pushMarker(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setMarker(const String& name)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::popMarker()
    {
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
    void VulkanRenderer::setShaderVariable(
      const platform::ShaderVariable& variable)
    {
		LMB_ASSERT(false, "VULKAN: setShaderVariable not implemented");
	}

	///////////////////////////////////////////////////////////////////////////
	VkDevice VulkanRenderer::getDevice() const
	{
		return device_;
	}

	///////////////////////////////////////////////////////////////////////////
	VkCommandBuffer VulkanRenderer::getCommandBuffer() const
	{
		return command_buffer_.getCommandBuffer();
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
    void VulkanRenderer::destroyAsset(
      foundation::SharedPointer<asset::IAsset> asset)
    {
    }

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::createCommandBuffer()
	{
		// Get the graphics queue handle.
		vezGetDeviceGraphicsQueue(device_, 0, &graphics_queue_);

		command_buffer_.create(device_, graphics_queue_);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::createTest()
	{
		VkResult result;
		VezSamplerCreateInfo sampler_create_info{};
		sampler_create_info.magFilter    = VK_FILTER_LINEAR;
		sampler_create_info.minFilter    = VK_FILTER_LINEAR;
		sampler_create_info.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		result = vezCreateSampler(device_, &sampler_create_info, &test_.sampler);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create sampler | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::updateFramebuffer()
	{
		Vector<VkImageView> attachments;

		for (uint32_t i = 0; i < state_.shader->getNumRenderTargets(); ++i)
			attachments.push_back(state_.render_targets[i]);

		if (state_.depth_target)
			attachments.push_back(state_.depth_target);

		VezFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.attachmentCount = (uint32_t)attachments.size();
		framebuffer_create_info.pAttachments = attachments.data();
		framebuffer_create_info.width  = state_.rt_width;
		framebuffer_create_info.height = state_.rt_height;
		framebuffer_create_info.layers = 1;
			
		VkResult result;
		result = vezCreateFramebuffer(device_, &framebuffer_create_info, &state_.framebuffer);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create frame buffer | %s", vkErrorCode(result));
	}

	Vector<VezAttachmentReference> VulkanRenderer::getAttachmentReferences()
	{
		Vector<VezAttachmentReference> attachment_references;

		for (uint32_t i = 0; i < state_.shader->getNumRenderTargets(); ++i)
		{
			VezAttachmentReference attachment_reference{};
			attachment_reference.loadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment_reference.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment_references.push_back(attachment_reference);
		}

		if (state_.depth_target)
		{
			VezAttachmentReference attachment_reference{};
			attachment_reference.loadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment_reference.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment_references.push_back(attachment_reference);
		}
		
		return attachment_references;
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setVSync(bool vsync)
    {
		vsync_ = vsync;
		vezSwapchainSetVSync(swapchain_, vsync);
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
	}

	///////////////////////////////////////////////////////////////////////////
	void* VulkanRenderBuffer::lock()
	{
		LMB_ASSERT(data_, "VULKAN: Buffer was already locked");
		data_ = foundation::Memory::allocate(size_);
		return data_;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderBuffer::unlock()
	{
		LMB_ASSERT(!data_, "VULKAN: Buffer was not locked");
		VkResult result;
		result = vezBufferSubData(renderer_->getDevice(), buffer_, 0, size_, data_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not update buffer data | %s", vkErrorCode(result));
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
		LMB_ASSERT(data_, "VULKAN: Texty was already locked");
		data_ = foundation::Memory::allocate(bpl);
		return data_;*/
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderTexture::unlock(uint32_t level)
	{
		LMB_ASSERT(false, "VULKAN: Not yet implemented");
		/*LMB_ASSERT(!data_, "VULKAN: Buffer was not locked");
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
		: command_buffer(VK_NULL_HANDLE)
		, recording(false)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::create(VkDevice device, VkQueue graphics_queue)
	{
		VkResult result;
		VezCommandBufferAllocateInfo cb_alloc_info{};
		cb_alloc_info.queue              = graphics_queue;
		cb_alloc_info.commandBufferCount = 1;
		result = vezAllocateCommandBuffers(device, &cb_alloc_info, &command_buffer);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not allocate command buffer | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::destroy(VkDevice device)
	{
		LMB_ASSERT(!recording, "VULKAN: Command buffer was already in recording state");
		vezFreeCommandBuffers(device, 1, &command_buffer);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::begin()
	{
		LMB_ASSERT(!recording, "VULKAN: Command buffer was already in recording state");
		recording = true;
	
		VkResult result;
		result = vezBeginCommandBuffer(command_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not begin command buffer | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::end()
	{
		LMB_ASSERT(recording, "VULKAN: Command buffer was not in recording state");
		recording = false;

		VkResult result;
		result = vezEndCommandBuffer();
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not end command buffer | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::tryBegin()
	{
		if (!recording)
			begin();
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanCommandBuffer::tryEnd()
	{
		if (recording)
			end();
	}

	///////////////////////////////////////////////////////////////////////////
	VkCommandBuffer VulkanCommandBuffer::getCommandBuffer() const
	{
		return command_buffer;
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
			Entry<VulkanShader> entry;
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
			VulkanRenderTexture* texture = (VulkanRenderTexture*)renderer->allocRenderTexture(handle);
			Entry<VulkanRenderTexture> entry;
			entry.ptr = texture;
			entry.keep_in_memory = handle->getKeepInMemory();
			textures.insert(eastl::make_pair(handle.getHash(), entry));
			it = textures.find(handle.getHash());
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
			renderer->freeRenderTexture(it->second.ptr);
			textures.erase(it);
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
	}
	
	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::Memory::removeAllEntries()
	{
		for (auto& texture : textures)
			removeTexture(texture.first);
		for (auto& shader : shaders)
			removeShader(shader.first);
	}
  }
}