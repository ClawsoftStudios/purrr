#ifndef   __PURRR_VULKAN_CONTEXT_H_
#define   __PURRR_VULKAN_CONTEXT_H_

#include "vulkan.h"

#include <vulkan/vulkan.h>

typedef struct _Purrr_Context_Vulkan {
  VkInstance instance;
  VkPhysicalDevice gpu;

  uint32_t graphicsQueueFamily;
  VkDevice device;
  VkQueue graphicsQueue;
} _Purrr_Context_Vulkan;

Purrr_Result _purrr_create_context_vulkan(Purrr_Context_Create_Info createInfo, _Purrr_Context_Vulkan **context);
Purrr_Result _purrr_destroy_context_vulkan(_Purrr_Context_Vulkan *context);

#endif // __PURRR_VULKAN_CONTEXT_H_