#include "vulkan_renderer.h"
#include <interfaces/iworld.h>
#include "vulkan_texture.h"
#include "vulkan_shader.h"
#include "vulkan_mesh.h"
#include "vulkan_state_manager.h"

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
	void VulkanRenderer::freeRenderTexture(platform::IRenderTexture*& texture)
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
	  , command_buffer_(VK_NULL_HANDLE)
	{
	}

	///////////////////////////////////////////////////////////////////////////
    VulkanRenderer::~VulkanRenderer()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::initialize(world::IWorld* world)
    {
		world_ = world;
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

		createFrameBuffer(window.get());
		createCommandBuffer();

		// Set VSync.
		setVSync(getVSync());
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::resize()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::update(const double& delta_time)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::startFrame()
    {
		VkResult result;
	
		result = vezBeginCommandBuffer(command_buffer_, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not begin command buffer | %s", vkErrorCode(result));
	}
   
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::endFrame(bool display)
    {
		VkResult result;

		// End command buffer recording.
		result = vezEndCommandBuffer();
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not end command buffer | %s", vkErrorCode(result));


		// Submit the command buffer to the graphics queue.
		VezSubmitInfo submit_info{};
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers    = &command_buffer_;

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
		present_info.pImages            = &frame_buffer_.color_image;
		result = vezQueuePresent(graphics_queue_, &present_info);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not present KHR queue | %s", vkErrorCode(result));
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::draw()
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::drawInstanced(const Vector<glm::mat4>& matrices)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setBlendState(const platform::BlendState& blend_state)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::generateMipMaps(const asset::VioletTextureHandle& texture)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::copyToScreen(const asset::VioletTextureHandle& texture)
    {
    }

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::copyToTexture(const asset::VioletTextureHandle& src, const asset::VioletTextureHandle& dst)
	{
	}
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::clearRenderTarget(
      asset::VioletTextureHandle texture, 
      const glm::vec4& colour)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setScissorRect(const glm::vec4& rect)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setViewports(const Vector<glm::vec4>& rects)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setMesh(asset::MeshHandle mesh)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setSubMesh(const uint32_t& sub_mesh_idx)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setShader(asset::VioletShaderHandle shader)
    {
		VulkanShader sh(shader, this);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setTexture(
      asset::VioletTextureHandle texture, 
      uint8_t slot)
    {
    }

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::setRenderTargets(Vector<asset::VioletTextureHandle> render_targets, asset::VioletTextureHandle depth_buffer)
	{
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
    }

	///////////////////////////////////////////////////////////////////////////
	VkDevice VulkanRenderer::getDevice() const
	{
		return device_;
	}

	///////////////////////////////////////////////////////////////////////////
	VkCommandBuffer VulkanRenderer::getCommandBuffer() const
	{
		return command_buffer_;
	}

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setRenderScale(const float & render_scale)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::destroyAsset(
      foundation::SharedPointer<asset::IAsset> asset)
    {
    }

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::createFrameBuffer(platform::IWindow* window)
	{
		VkResult result;

		// Get swapchain format.
		VkSurfaceFormatKHR swap_chain_format{};
		vezGetSwapchainSurfaceFormat(swapchain_, &swap_chain_format);

		// Create color buffer.
		VezImageCreateInfo image_create_info{};
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = swap_chain_format.format;
		image_create_info.extent = { window->getSize().x, window->getSize().y, 1 };
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		result = vezCreateImage(device_, VEZ_MEMORY_GPU_ONLY, &image_create_info, &frame_buffer_.color_image);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create color buffer | %s", vkErrorCode(result));

		// Create the image view for binding the texture as a resource.
		VezImageViewCreateInfo image_view_create_info{};
		image_view_create_info.image = frame_buffer_.color_image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = image_create_info.format;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.subresourceRange.levelCount = 1;
		result = vezCreateImageView(device_, &image_view_create_info, &frame_buffer_.color_image_view);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create color buffer view | %s", vkErrorCode(result));

		// Create the depth image for the m_framebuffer.
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = VK_FORMAT_D32_SFLOAT;
		image_create_info.extent = { window->getSize().x, window->getSize().y, 1 };
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		result = vezCreateImage(device_, VEZ_MEMORY_GPU_ONLY, &image_create_info, &frame_buffer_.depth_stencil_image);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create depth stencil buffer | %s", vkErrorCode(result));

		// Create the image view for binding the texture as a resource.
		image_view_create_info.image = frame_buffer_.depth_stencil_image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = image_create_info.format;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.subresourceRange.levelCount = 1;
		result = vezCreateImageView(device_, &image_view_create_info, &frame_buffer_.depth_stencil_image_view);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create depth stencil buffer view | %s", vkErrorCode(result));

		// Create the m_framebuffer.
		Array<VkImageView, 2> attachments = { frame_buffer_.color_image_view, frame_buffer_.depth_stencil_image_view };
		VezFramebufferCreateInfo frame_buffer_create_info{};
		frame_buffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		frame_buffer_create_info.pAttachments = attachments.data();
		frame_buffer_create_info.width = window->getSize().x;
		frame_buffer_create_info.height = window->getSize().y;
		frame_buffer_create_info.layers = 1;
		result = vezCreateFramebuffer(device_, &frame_buffer_create_info, &frame_buffer_.handle);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create frame buffer | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanRenderer::createCommandBuffer()
	{
		VkResult result;

		// Get the graphics queue handle.
		vezGetDeviceGraphicsQueue(device_, 0, &graphics_queue_);

		// Create a command buffer handle.
		VezCommandBufferAllocateInfo cb_alloc_info{};
		cb_alloc_info.queue              = graphics_queue_;
		cb_alloc_info.commandBufferCount = 1;
		result = vezAllocateCommandBuffers(device_, &cb_alloc_info, &command_buffer_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not allocate command buffer | %s", vkErrorCode(result));

		//// Set the viewport state and dimensions.
		//uint32_t width  = world_->getWindow()->getSize().x;
		//uint32_t height = world_->getWindow()->getSize().y;
		//VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
		//VkRect2D   scissor  = { { 0, 0 },{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
		//vezCmdSetViewport(0, 1, &viewport);
		//vezCmdSetScissor(0, 1, &scissor);
		//vezCmdSetViewportState(1);

		//// Define clear values for the swapchain's color and depth attachments.
		//Array<VezAttachmentReference, 2> attachment_references = {};
		//attachment_references[0].clearValue.color = { 0.3f, 0.3f, 0.3f, 0.0f };
		//attachment_references[0].loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//attachment_references[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//attachment_references[1].clearValue.depthStencil.depth = 1.0f;
		//attachment_references[1].loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//attachment_references[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//// Begin a render pass.
		//VezRenderPassBeginInfo render_pass_begin_info{};
		//render_pass_begin_info.framebuffer     = frame_buffer_.handle;
		//render_pass_begin_info.attachmentCount = static_cast<uint32_t>(attachment_references.size());
		//render_pass_begin_info.pAttachments    = attachment_references.data();
		//vezCmdBeginRenderPass(&render_pass_begin_info);

		//// Bind the pipeline and associated resources.
		//vezCmdBindPipeline(test_.pipeline.pipeline);
		//vezCmdBindBuffer(test_.uniform_buffer, 0, VK_WHOLE_SIZE, 0, 0, 0);
		//vezCmdBindImageView(test_.image_view, test_.sampler, 0, 1, 0);

		//// Set depth stencil state.
		//VezPipelineDepthStencilState depth_stencil_state = {};
		//depth_stencil_state.depthTestEnable  = VK_TRUE;
		//depth_stencil_state.depthWriteEnable = VK_TRUE;
		//depth_stencil_state.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
		//vezCmdSetDepthStencilState(&depth_stencil_state);

		//// Bind the vertex buffer and index buffers.
		//VkDeviceSize offset = 0;
		//vezCmdBindVertexBuffers(0, 1, &test_.vertex_buffer, &offset);
		//vezCmdBindIndexBuffer(test_.index_buffer, 0, VK_INDEX_TYPE_UINT32);

		//// Draw the quad.
		//vezCmdDrawIndexed(6, 1, 0, 0, 0);

		//// End the render pass.
		//vezCmdEndRenderPass();
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
      return 1.0f;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool VulkanRenderer::getVSync() const
    {
      return vsync_;
    }
	VulkanRenderBuffer::VulkanRenderBuffer(uint32_t size, uint32_t flags, VkBuffer buffer, VulkanRenderer* renderer)
		: data_(nullptr)
		, size_(size)
		, flags_(flags)
		, buffer_(buffer)
		, renderer_(renderer)
	{
	}
	void* VulkanRenderBuffer::lock()
	{
		LMB_ASSERT(data_, "VULKAN: Buffer was already locked");
		data_ = foundation::Memory::allocate(size_);
		return data_;
	}
	void VulkanRenderBuffer::unlock()
	{
		LMB_ASSERT(!data_, "VULKAN: Buffer was not locked");
		VkResult result;
		result = vezBufferSubData(renderer_->getDevice(), buffer_, 0, size_, data_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not update buffer data | %s", vkErrorCode(result));
		foundation::Memory::deallocate(data_);
		data_ = nullptr;
	}
	uint32_t VulkanRenderBuffer::getFlags() const
	{
		return flags_;
	}
	uint32_t VulkanRenderBuffer::getSize() const
	{
		return size_;
	}
	VkBuffer VulkanRenderBuffer::getBuffer() const
	{
		return buffer_;
	}
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
	uint32_t VulkanRenderTexture::getWidth() const
	{
		return width_;
	}
	uint32_t VulkanRenderTexture::getHeight() const
	{
		return height_;
	}
	uint32_t VulkanRenderTexture::getDepth() const
	{
		return depth_;
	}
	uint32_t VulkanRenderTexture::getMipCount() const
	{
		return mip_count_;
	}
	uint32_t VulkanRenderTexture::getFlags() const
	{
		return flags_;
	}
	TextureFormat VulkanRenderTexture::getFormat() const
	{
		return format_;
	}
	VulkanTexture* VulkanRenderTexture::getTexture() const
	{
		return texture_;
	}
}
}