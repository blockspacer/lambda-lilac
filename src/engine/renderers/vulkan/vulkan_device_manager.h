#pragma once
#include "vulkan.h"
#include <containers/containers.h>

#define VIOLET_USE_GPU_MARKERS 0

namespace lambda
{
  namespace platform
  {
    class IWindow;
  }
  namespace linux
  {
    //////////////////////////////////////////////////////////////////////
    class VulkanDeviceManager
    {
    public:
	  void initialize(platform::IWindow* window);
	  void deinitialize();
	  VkAllocationCallbacks* getAllocator()                  const { return allocator_; }
	  VkInstance       getInstance()                         const { return instance_; }
	  VkPhysicalDevice getPhysicalDevice()                   const { return physical_device_; }
	  VkSurfaceKHR     getSurface()                          const { return surface_; }
	  VkDevice         getDevice()                           const { return device_; }
	  VkSwapchainKHR   getSwapchain()                        const { return swapchain_; }
	  VkImage          getSwapchainImage(uint32_t index)     const { return swapchain_images_[index]; }
	  VkImageView      getSwapchainImageView(uint32_t index) const { return swapchain_image_views_[index]; }
	  VkFormat         getSwapchainFormat()                  const { return swapchain_format_; }
	  VkExtent2D       getSwapchainExtent()                  const { return swapchain_extent_; }
	  uint32_t         getGraphicsQueueFamilyIndex()         const { return graphics_queue_family_; }
	  uint32_t         getPresentQueueFamilyIndex()          const { return present_queue_family_; }
	  uint32_t         getComputeQueueFamilyIndex()          const { return compute_queue_family_; }
	  VkQueue          getGraphicsQueue()                    const { return graphics_queue_; }
	  VkQueue          getPresentQueue()                     const { return present_queue_; }
	  VkQueue          getComputeQueue()                     const { return compute_queue_; }

	private:
	  void initializeLayers();
	  void validateInstance();
	  void createInstance();
	  void createDebug();
	  void getPhysicalDevice();
	  void createSurface(platform::IWindow* window);
	  void findQueues();
	  void validateDevice();
	  void createDevice();
	  void getQueues();
	  void createSwapchain(platform::IWindow* window);

	private:
	  Vector<const char*> device_extensions_;
	  Vector<const char*> device_layers_;
	  Vector<const char*> instance_extensions_;
	  Vector<const char*> instance_layers_;
	  VkAllocationCallbacks* allocator_ = nullptr;
	  VkInstance instance_;
	  VkPhysicalDevice physical_device_;
	  VkSurfaceKHR surface_;
	  VkDevice device_;
	  
	  VkSwapchainKHR swapchain_;
	  Vector<VkImage> swapchain_images_;
	  Vector<VkImageView> swapchain_image_views_;
	  VkFormat swapchain_format_;
	  VkExtent2D swapchain_extent_;

	  uint32_t graphics_queue_family_;
	  VkQueue  graphics_queue_;
	  uint32_t present_queue_family_;
	  VkQueue  present_queue_;
	  uint32_t compute_queue_family_;
	  VkQueue  compute_queue_;

#if VIOLET_DEBUG
	  VkDebugUtilsMessengerEXT debug_messenger_;
#endif
	};
  }
}