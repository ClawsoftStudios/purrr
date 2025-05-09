#ifndef   __PURRR_VULKAN_SAMPLER_H_
#define   __PURRR_VULKAN_SAMPLER_H_

#include "./vulkan.h"
#include "./context.h"

typedef struct _Purrr_Sampler_Vulkan {
  _Purrr_Context_Vulkan *context;

  VkSampler sampler;
} _Purrr_Sampler_Vulkan;

Purrr_Result _purrr_create_sampler_vulkan(_Purrr_Context_Vulkan *context, Purrr_Sampler_Create_Info createInfo, _Purrr_Sampler_Vulkan **sampler);
Purrr_Result _purrr_destroy_sampler_vulkan(_Purrr_Sampler_Vulkan *sampler);

#endif // __PURRR_VULKAN_SAMPLER_H_