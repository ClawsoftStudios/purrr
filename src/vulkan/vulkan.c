#include "vulkan.h"

uint32_t _purrr_vulkan_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDeviceMemoryProperties memProperties) {
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;
  return UINT32_MAX;
}