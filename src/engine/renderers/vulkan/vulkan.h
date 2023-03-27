#pragma once

#if VIOLET_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.hpp>
#include <utils/name.h>

namespace lambda
{
	namespace linux
	{
		extern const char* vkErrorCode(const VkResult& result);

		struct VulkanReflectionInfo
		{
			Name     name;
			uint32_t set = 0u;
			uint32_t binding = 0u;
			uint32_t slot = 0u;

			bool operator==(const VulkanReflectionInfo& other) const
			{
				return set == other.set && binding == other.binding && name.getHash() == other.name.getHash();
			}
			bool operator<(const VulkanReflectionInfo& other) const
			{
				return set < other.set && binding < other.binding && name.getHash() < other.name.getHash();
			}
			bool operator>(const VulkanReflectionInfo& other) const
			{
				return set > other.set && binding > other.binding && name.getHash() > other.name.getHash();
			}
		};
	}
}