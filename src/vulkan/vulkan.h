#ifndef   __PURRR_VULKAN_H_
#define   __PURRR_VULKAN_H_

#include "purrr/purrr.h"

#include <vulkan/vulkan.h>

uint32_t _purrr_vulkan_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDeviceMemoryProperties memProperties);

#endif // __PURRR_VULKAN_H_