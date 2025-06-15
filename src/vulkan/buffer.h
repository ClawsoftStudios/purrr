#ifndef   __PURRR_VULKAN_BUFFER_H_
#define   __PURRR_VULKAN_BUFFER_H_

#include "./vulkan.h"
#include "./context.h"

#include "../internal.h"

typedef struct _Purrr_Buffer_Vulkan {
  _Purrr_Context_Vulkan *context;

  Purrr_Buffer_Type type;
  uint32_t size;
  VkBuffer buffer;
  VkDeviceMemory memory;
  bool hostVisible;

  VkDescriptorSet descriptorSet;
} _Purrr_Buffer_Vulkan;

Purrr_Result _purrr_create_buffer_vulkan(_Purrr_Context_Vulkan *context, Purrr_Buffer_Create_Info createInfo, _Purrr_Buffer_Vulkan **buffer);
Purrr_Result _purrr_destroy_buffer_vulkan(_Purrr_Buffer_Vulkan *buffer);

Purrr_Result _purrr_copy_buffer_data_vulkan(_Purrr_Buffer_Vulkan *dst, void *src, uint32_t size, uint32_t offset);
Purrr_Result _purrr_copy_buffer_vulkan(_Purrr_Buffer_Vulkan *dst, _Purrr_Buffer_Vulkan *src, uint32_t offset);

#endif // __PURRR_VULKAN_BUFFER_H_