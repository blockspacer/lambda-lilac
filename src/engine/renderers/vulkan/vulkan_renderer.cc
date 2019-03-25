#include "vulkan_renderer.h"
#include <interfaces/iworld.h>

namespace lambda
{
  namespace linux
  {
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

		VezInstanceCreateInfo instance_create_info{};
		instance_create_info.pApplicationInfo    = &application_info;
		instance_create_info.enabledLayerCount   = static_cast<uint32_t>(enabled_layers.size());
		instance_create_info.ppEnabledLayerNames = enabled_layers.data();
		VkResult result = vezCreateInstance(&instance_create_info, &instance_);
		LMB_ASSERT(result != VK_SUCCESS, "VULKAN: Failed to create instance");

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

		LMB_ASSERT(physical_device_ == VK_NULL_HANDLE, "VULKAN: Could not find physical device");

		// Create surface.
#if VIOLET_WIN32
		VkWin32SurfaceCreateInfoKHR surface_create_info {};
		surface_create_info.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_create_info.hwnd      = (HWND)window->getWindow();
		surface_create_info.hinstance = GetModuleHandle(nullptr);

		result = vkCreateWin32SurfaceKHR(instance_, &surface_create_info, nullptr, &surface_);
		LMB_ASSERT(result != VK_SUCCESS, "VULKAN: Could not create surface");
#endif

		// Create device.
		VezDeviceCreateInfo device_create_info{};
		device_create_info.enabledExtensionCount = 0;
		device_create_info.ppEnabledExtensionNames = nullptr;
		result = vezCreateDevice(physical_device_, &device_create_info, &device_);
		LMB_ASSERT(result != VK_SUCCESS, "VULKAN: Could not create device");

	    // Create swapchain.
	    VezSwapchainCreateInfo swapchain_create_info{};
		swapchain_create_info.surface = surface_;
		swapchain_create_info.format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		swapchain_create_info.tripleBuffer = VK_TRUE;
		result = vezCreateSwapchain(device_, &swapchain_create_info, &swapchain_);
		LMB_ASSERT(result != VK_SUCCESS, "VULKAN: Could not create swapchain");
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
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::endFrame(bool display)
    {
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
    void VulkanRenderer::setRenderScale(const float & render_scale)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::destroyAsset(
      foundation::SharedPointer<asset::IAsset> asset)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::setVSync(bool vsync)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    float VulkanRenderer::getRenderScale()
    {
      return 1.0f;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool VulkanRenderer::getVSync() const
    {
      return false;
    }
  }
}