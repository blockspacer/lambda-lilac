#include "vulkan_device_manager.h"
#include <utils/console.h>
#include <interfaces/iwindow.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace lambda
{
  namespace linux
  {
    ///////////////////////////////////////////////////////////////////////////
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

#if VIOLET_DEBUG
	///////////////////////////////////////////////////////////////////////////
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		String prefix;

		if ((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0)
			prefix = "[GEN]";
		if ((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0)
			prefix = "[VAL]";
		if ((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0)
			prefix = "[PER]";

		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0)
			foundation::Debug(prefix + " " + String(pCallbackData->pMessage) + "\n");
		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0)
			foundation::Info(prefix + " " + String(pCallbackData->pMessage) + "\n");
		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
			foundation::Warning(prefix + " " + String(pCallbackData->pMessage) + "\n");
		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
			foundation::Error(prefix + " " + String(pCallbackData->pMessage) + "\n");

		return VK_FALSE;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback2(
		VkDebugReportFlagsEXT                       flags,
		VkDebugReportObjectTypeEXT                  objectType,
		uint64_t                                    object,
		size_t                                      location,
		int32_t                                     messageCode,
		const char*                                 pLayerPrefix,
		const char*                                 pMessage,
		void*                                       pUserData)
	{
		VkDebugUtilsMessageTypeFlagsEXT message_type;
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity;

		switch (flags)
		{
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_DEBUG_BIT_EXT:       message_severity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; break;
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_ERROR_BIT_EXT:       message_severity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; break;
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_INFORMATION_BIT_EXT: message_severity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; break;
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_WARNING_BIT_EXT:     message_severity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; break;
		}

		message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;

		VkDebugUtilsMessengerCallbackDataEXT callback_data{};
		callback_data.pMessage = pMessage;
		return debugCallback(message_severity, message_type, &callback_data, pUserData);
	}
#endif

    ///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::initialize(platform::IWindow* window)
	{
		initializeLayers();
		validateInstance();
		createInstance();
		createDebug();
		getPhysicalDevice();
		createSurface(window);
		findQueues();
		validateDevice();
		createDevice();
		getQueues();
		createSwapchain(window);
		createCommandPool();
		createCommandBuffers();
		createSemaphores();
		createVmaAllocator();
		current_frame_ = 1;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::deinitialize()
	{
		for (const auto& view : swapchain_image_views_)
			vkDestroyImageView(device_, view, allocator_);
		for (const auto& image : swapchain_images_)
			vkDestroyImage(device_, image, allocator_);
		vkDestroySwapchainKHR(device_, swapchain_, allocator_);
		vkDestroyDevice(device_, allocator_);
		vkDestroySurfaceKHR(instance_, surface_, allocator_);
#if VIOLET_DEBUG
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
		destroyDebugUtilsMessengerEXT(instance_, debug_messenger_, allocator_);
#endif
		vkDestroyInstance(instance_, allocator_);
	}
	
	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::beginFrame()
	{
		vkWaitForFences(device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);
		vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);

		VkCommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VkResult result;
		result = vkBeginCommandBuffer(command_buffers_[current_frame_], &command_buffer_begin_info);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not begin command buffer | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
#pragma optimize ("", off)
	void VulkanDeviceManager::endFrame()
	{
		VkResult result;
		result = vkEndCommandBuffer(command_buffers_[current_frame_]);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not end command buffer | %s", vkErrorCode(result));

		uint32_t image_index;
		result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, image_available_semaphore_[current_frame_], VK_NULL_HANDLE, &image_index);		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not acquire nex image KHR | %s", vkErrorCode(result));

		VkSemaphore wait_semaphores[]      = { image_available_semaphore_[current_frame_] };
		VkSemaphore signal_semaphores[]    = { render_finished_semaphore_[current_frame_] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submit_info{};
		submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount   = 1;
		submit_info.pWaitSemaphores      = wait_semaphores;
		submit_info.pWaitDstStageMask    = wait_stages;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores    = signal_semaphores;		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &command_buffers_[current_frame_];

		result = vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fences_[current_frame_]);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not submit queue | %s", vkErrorCode(result));
		VkSwapchainKHR swapchains[] = { swapchain_ };
		VkPresentInfoKHR present_info{};
		present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = signal_semaphores;		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = swapchains;
		present_info.pImageIndices      = &image_index;
		present_info.pResults           = nullptr;

		result = vkQueuePresentKHR(present_queue_, &present_info);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not present queue KHR | %s", vkErrorCode(result));

		result = vkQueueWaitIdle(present_queue_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not wait queue idle | %s", vkErrorCode(result));
	
		current_frame_ = (current_frame_ + 1) % command_buffers_.size();
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::initializeLayers()
	{
		instance_layers_ = { "VK_LAYER_LUNARG_standard_validation" };
		instance_extensions_ = { VK_KHR_SURFACE_EXTENSION_NAME };

#if VIOLET_DEBUG
		instance_extensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instance_extensions_.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		//instance_layers_.push_back("VK_LAYER_RENDERDOC_Capture");
#endif

#if VIOLET_WIN32
		instance_extensions_.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
#pragma error "Currently unsupported platform."
#endif

		device_extensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::validateInstance()
	{
		VkResult result;
		uint32_t extension_count;
		result = vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count, nullptr);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to enumerate instance extension properties | %s", vkErrorCode(result));

		Vector<VkExtensionProperties> extensions(extension_count);
		result = vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count, extensions.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to enumerate instance extension properties | %s", vkErrorCode(result));

		for (const auto& extension : instance_extensions_)
		{
			bool found = false;
			for (const auto& found_extension : extensions)
			{
				if (String(found_extension.extensionName) == String(extension))
				{
					found = true;
					break;
				}
			}
			LMB_ASSERT(found, "VULKAN: Could not find instance extension %s", extension);
		}

		uint32_t layer_count;
		result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to enumerate instance layer properties | %s", vkErrorCode(result));

		Vector<VkLayerProperties> layers(layer_count);
		result = vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to enumerate instance layer properties | %s", vkErrorCode(result));

		for (const auto& layer : instance_layers_)
		{
			bool found = false;
			for (const auto& found_layer : layers)
			{
				if (String(found_layer.layerName) == String(layer))
				{
					found = true;
					break;
				}
			}
			LMB_ASSERT(found, "VULKAN: Could not find instance layer %s", layer);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createInstance()
	{
		VkApplicationInfo application_info {};
		application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pApplicationName   = "MyApp";
		application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.pEngineName        = "Lambda Lilac";
		application_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		application_info.apiVersion         = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo        = &application_info;
		instance_create_info.enabledLayerCount       = static_cast<uint32_t>(instance_layers_.size());
		instance_create_info.ppEnabledLayerNames     = instance_layers_.data();
		instance_create_info.enabledExtensionCount   = static_cast<uint32_t>(instance_extensions_.size());
		instance_create_info.ppEnabledExtensionNames = instance_extensions_.data();
		
		VkResult result;
		result = vkCreateInstance(&instance_create_info, allocator_, &instance_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to create instance | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createDebug()
	{
#if VIOLET_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{};
		debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_utils_messenger_create_info.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			0;
		debug_utils_messenger_create_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			0;
		debug_utils_messenger_create_info.pfnUserCallback = debugCallback;
		debug_utils_messenger_create_info.pUserData = nullptr;

		PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
		PFN_vkCreateDebugReportCallbackEXT createDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");
		
		VkDebugReportCallbackCreateInfoEXT debug_report_callback_create_info{};
		debug_report_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_report_callback_create_info.flags =
			VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			0;
		debug_report_callback_create_info.pfnCallback = debugCallback2;
		
		VkResult result;
		result = createDebugReportCallbackEXT(instance_, &debug_report_callback_create_info, allocator_, &debug_report_callback_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to create debug report callback | %s", vkErrorCode(result));

		result = createDebugUtilsMessengerEXT(instance_, &debug_utils_messenger_create_info, VK_NULL_HANDLE, &debug_messenger_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Failed to create debug utils messenger | %s", vkErrorCode(result));
#endif
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::getPhysicalDevice()
	{
		uint32_t physical_device_count;
		vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr);

		Vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data());

		for (VkPhysicalDevice physical_device : physical_devices)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physical_device, &properties);

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				foundation::Debug(properties.deviceName + String("\n"));
				physical_device_ = physical_device;
				break;
			}
		}

		LMB_ASSERT(physical_device_ != VK_NULL_HANDLE, "VULKAN: Could not find physical device");
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createSurface(platform::IWindow* window)
	{
#if VIOLET_WIN32
		VkWin32SurfaceCreateInfoKHR surface_create_info{};
		surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_create_info.hwnd = (HWND)window->getWindow();
		surface_create_info.hinstance = GetModuleHandle(NULL);

		VkResult result;
		result = vkCreateWin32SurfaceKHR(instance_, &surface_create_info, allocator_, &surface_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create surface | %s", vkErrorCode(result));
#else
#pragma error "Unknown platform"
#endif
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::findQueues()
	{
		uint32_t queue_family_count;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, nullptr);

		Vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, queue_families.data());

		for (uint32_t i = 0; i < queue_families.size(); ++i)
		{
			const auto& queue_family = queue_families[i];
			VkBool32 can_present = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &can_present);
			if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphics_queue_family_ = i;
			if (queue_family.queueCount > 0 && can_present)
				present_queue_family_ = i;
			if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)
				compute_queue_family_ = i;

			if (graphics_queue_family_ != UINT32_MAX && present_queue_family_ != UINT32_MAX && compute_queue_family_ != UINT32_MAX)
				break;
		}

		LMB_ASSERT(graphics_queue_family_ != UINT32_MAX, "VULKAN: Could not find a valid graphics queue family");
		LMB_ASSERT(present_queue_family_ != UINT32_MAX, "VULKAN: Could not find a valid present queue family");
		LMB_ASSERT(compute_queue_family_ != UINT32_MAX, "VULKAN: Could not find a valid compute queue family");
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::validateDevice()
	{
		VkResult result;

		// Validate extensions.
		uint32_t extension_count;
		result = vkEnumerateDeviceExtensionProperties(physical_device_, VK_NULL_HANDLE, &extension_count, VK_NULL_HANDLE);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not enumerate device extensions | %s", vkErrorCode(result));

		Vector<VkExtensionProperties> extensions(extension_count);
		result = vkEnumerateDeviceExtensionProperties(physical_device_, VK_NULL_HANDLE, &extension_count, extensions.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not enumerate device extensions | %s", vkErrorCode(result));

		for (const auto& extension : device_extensions_)
		{
			bool found = false;
			for (const auto& found_extension : extensions)
			{
				if (String(found_extension.extensionName) == String(extension))
				{
					found = true;
					break;
				}
			}
			LMB_ASSERT(found, "VULKAN: Could not find device extension %s", extension);
		}

		// Validate layers.
		uint32_t layer_count;
		result = vkEnumerateDeviceLayerProperties(physical_device_, &layer_count, VK_NULL_HANDLE);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not enumerate device layers | %s", vkErrorCode(result));

		Vector<VkLayerProperties> layers(layer_count);
		result = vkEnumerateDeviceLayerProperties(physical_device_, &layer_count, layers.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not enumerate device layers | %s", vkErrorCode(result));

		for (const auto& layer : device_layers_)
		{
			bool found = false;
			for (const auto& found_layer : layers)
			{
				if (String(found_layer.layerName) == String(layer))
				{
					found = true;
					break;
				}
			}
			LMB_ASSERT(found, "VULKAN: Could not find device layer %s", layer);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createDevice()
	{
		float queue_priority = 1.0f;
		VkDeviceQueueCreateInfo queue_create_infos[3];
		queue_create_infos[0] = {};
		queue_create_infos[1] = {};
		queue_create_infos[2] = {};
		queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[0].queueCount = 1;
		queue_create_infos[1].queueCount = 1;
		queue_create_infos[2].queueCount = 1;
		queue_create_infos[0].pQueuePriorities = &queue_priority;
		queue_create_infos[1].pQueuePriorities = &queue_priority;
		queue_create_infos[2].pQueuePriorities = &queue_priority;
		queue_create_infos[0].queueFamilyIndex = graphics_queue_family_;
		queue_create_infos[1].queueFamilyIndex = present_queue_family_;
		queue_create_infos[2].queueFamilyIndex = compute_queue_family_;
		
		VkPhysicalDeviceFeatures device_features{};

		VkDeviceCreateInfo device_create_info{};
		device_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.enabledExtensionCount   = static_cast<uint32_t>(device_extensions_.size());
		device_create_info.ppEnabledExtensionNames = device_extensions_.data();
		device_create_info.enabledLayerCount       = static_cast<uint32_t>(device_layers_.size());
		device_create_info.ppEnabledLayerNames     = device_layers_.data();
		device_create_info.pQueueCreateInfos       = queue_create_infos;
		device_create_info.queueCreateInfoCount    = 3;
		device_create_info.pEnabledFeatures        = &device_features;
		
		VkResult result;
		result = vkCreateDevice(physical_device_, &device_create_info, allocator_, &device_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create device | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::getQueues()
	{
		LMB_ASSERT(graphics_queue_family_ != UINT32_MAX, "VULKAN: Could not find a valid graphics queue family");
		LMB_ASSERT(present_queue_family_  != UINT32_MAX, "VULKAN: Could not find a valid present queue family");
		LMB_ASSERT(compute_queue_family_  != UINT32_MAX, "VULKAN: Could not find a valid compute queue family");
		vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);
		vkGetDeviceQueue(device_, present_queue_family_,  0, &present_queue_);
		vkGetDeviceQueue(device_, compute_queue_family_,  0, &compute_queue_);
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createSwapchain(platform::IWindow* window)
	{
		VkResult result;
		VkSurfaceCapabilitiesKHR surface_capabilities;
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_capabilities);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not get physical device surface capabilities | %s", vkErrorCode(result));

		uint32_t format_count;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not get physical device surface formats | %s", vkErrorCode(result));

		Vector<VkSurfaceFormatKHR> surface_formats(format_count);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, surface_formats.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not get physical device surface formats | %s", vkErrorCode(result));

		uint32_t present_mode_count;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_mode_count, nullptr);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not get physical device surface present modes KHR | %s", vkErrorCode(result));

		Vector<VkPresentModeKHR> present_modes(present_mode_count);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_mode_count, present_modes.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not get physical device surface present modes KHR | %s", vkErrorCode(result));
			
		VkSurfaceFormatKHR desired_format = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

		if (surface_formats.size() == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
			desired_format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

		for (uint32_t i = 0; i < surface_formats.size() && desired_format.format == VK_FORMAT_UNDEFINED; ++i)
			if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				desired_format = surface_formats[i];

		if (desired_format.format == VK_FORMAT_UNDEFINED && surface_formats.size() > 0)
			desired_format = surface_formats[0];

		VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;

		for (uint32_t i = 0; i < present_modes.size() && desired_present_mode == VK_PRESENT_MODE_MAX_ENUM_KHR; ++i)
			if (present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
				desired_present_mode = present_modes[i];
		for (uint32_t i = 0; i < present_modes.size() && desired_present_mode == VK_PRESENT_MODE_MAX_ENUM_KHR; ++i)
			if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				desired_present_mode = present_modes[i];
		for (uint32_t i = 0; i < present_modes.size() && desired_present_mode == VK_PRESENT_MODE_MAX_ENUM_KHR; ++i)
			if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				desired_present_mode = present_modes[i];

		VkExtent2D desired_extent;

		if (surface_capabilities.currentExtent.width != UINT32_MAX)
			desired_extent = surface_capabilities.currentExtent;
		else 
		{
			desired_extent = { window->getSize().x, window->getSize().y };

			desired_extent.width  = std::max(surface_capabilities.minImageExtent.width,  std::min(surface_capabilities.maxImageExtent.width,  desired_extent.width));
			desired_extent.height = std::max(surface_capabilities.minImageExtent.height, std::min(surface_capabilities.maxImageExtent.height, desired_extent.height));
		}

		uint32_t image_count = surface_capabilities.minImageCount + 1;
		if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
			image_count = surface_capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface          = surface_;
		swapchain_create_info.minImageCount    = image_count;
		swapchain_create_info.imageFormat      = desired_format.format;
		swapchain_create_info.imageColorSpace  = desired_format.colorSpace;
		swapchain_create_info.imageExtent      = desired_extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queue_family_indices[] = { graphics_queue_family_, present_queue_family_ };

		if (graphics_queue_family_ != present_queue_family_) 
		{
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.queueFamilyIndexCount = 2;
			swapchain_create_info.pQueueFamilyIndices   = queue_family_indices;
		}
		else 
		{
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_create_info.queueFamilyIndexCount = 0;
			swapchain_create_info.pQueueFamilyIndices   = nullptr;
		}

		swapchain_create_info.preTransform   = surface_capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode    = desired_present_mode;
		swapchain_create_info.clipped        = VK_TRUE;
		swapchain_create_info.oldSwapchain   = VK_NULL_HANDLE;

		result = vkCreateSwapchainKHR(device_, &swapchain_create_info, allocator_, &swapchain_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create swapchain KHR | %s", vkErrorCode(result));

		swapchain_format_ = desired_format.format;
		swapchain_extent_ = desired_extent;

		result = vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not get swapchain images KHR | %s", vkErrorCode(result));

		swapchain_images_.resize(image_count);
		result = vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not get swapchain images KHR | %s", vkErrorCode(result));

		swapchain_image_views_.resize(image_count);

		for (uint32_t i = 0; i < swapchain_images_.size(); i++)
		{
			VkImageViewCreateInfo image_view_create_info{};
			image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image                           = swapchain_images_[i];
			image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format                          = swapchain_format_;
			image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel   = 0;
			image_view_create_info.subresourceRange.levelCount     = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount     = 1;

			result = vkCreateImageView(device_, &image_view_create_info, allocator_, &swapchain_image_views_[i]);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create image view | %s", vkErrorCode(result));
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createCommandPool()
	{
		VkCommandPoolCreateInfo command_pool_create_info{};
		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.queueFamilyIndex = graphics_queue_family_;

		VkResult result;
		result = vkCreateCommandPool(device_, &command_pool_create_info, allocator_, &command_pool_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create command pool | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createCommandBuffers()
	{
		command_buffers_.resize(swapchain_images_.size());
		VkCommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandBufferCount = (uint32_t)command_buffers_.size();
		command_buffer_allocate_info.commandPool        = command_pool_;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkResult result;
		result = vkAllocateCommandBuffers(device_, &command_buffer_allocate_info, command_buffers_.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not allocate command buffers | %s", vkErrorCode(result));
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createSemaphores()
	{
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;		VkResult result;

		image_available_semaphore_.resize(command_buffers_.size());
		render_finished_semaphore_.resize(command_buffers_.size());
		in_flight_fences_.resize(command_buffers_.size());
		for (uint32_t i = 0; i < command_buffers_.size(); ++i)
		{
			result = vkCreateSemaphore(device_, &semaphore_create_info, allocator_, &image_available_semaphore_[i]);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create semaphore | %s", vkErrorCode(result));
			result = vkCreateSemaphore(device_, &semaphore_create_info, allocator_, &render_finished_semaphore_[i]);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create semaphore | %s", vkErrorCode(result));
			result = vkCreateFence(device_, &fence_create_info, allocator_, &in_flight_fences_[i]);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: could not create fence | %s", vkErrorCode(result));
		}
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanDeviceManager::createVmaAllocator()
	{
		VmaAllocatorCreateInfo allocator_create_info{};
		allocator_create_info.physicalDevice = physical_device_;
		allocator_create_info.device         = device_;

		vmaCreateAllocator(&allocator_create_info, &vma_allocator_);
	}
  }
}