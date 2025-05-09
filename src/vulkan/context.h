#ifndef   __PURRR_VULKAN_CONTEXT_H_
#define   __PURRR_VULKAN_CONTEXT_H_

#include "./vulkan.h"
#include "../internal.h"

typedef struct _Purrr_Context_Vulkan {
  VkInstance instance;
  VkPhysicalDevice gpu;

  uint32_t queueFamily;
  VkDevice device;
  VkQueue queue;

  VkCommandPool commandPool;
} _Purrr_Context_Vulkan;

Purrr_Result _purrr_create_context_vulkan(Purrr_Context_Create_Info createInfo, _Purrr_Context_Vulkan **context);
Purrr_Result _purrr_destroy_context_vulkan(_Purrr_Context_Vulkan *context);



Purrr_Result _purrr_context_begin_one_time_command_buffer_vulkan(_Purrr_Context_Vulkan *context, VkCommandBuffer *cmdBuf);
Purrr_Result _purrr_context_submit_one_time_command_buffer_vulkan(_Purrr_Context_Vulkan *context, VkCommandBuffer cmdBuf);

#endif // __PURRR_VULKAN_CONTEXT_H_