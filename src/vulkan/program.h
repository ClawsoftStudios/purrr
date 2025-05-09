#ifndef   __PURRR_VULKAN_PROGRAM_H_
#define   __PURRR_VULKAN_PROGRAM_H_

#include "./vulkan.h"
#include "./context.h"

#include "../internal.h"

typedef struct _Purrr_Program_Vulkan {
  Purrr_Handle renderTarget;
  _Purrr_Context_Vulkan *context;

  VkPipelineLayout layout;
  VkPipeline pipeline;
} _Purrr_Program_Vulkan;

Purrr_Result _purrr_create_program_vulkan(Purrr_Handle renderTarget, Purrr_Program_Create_Info createInfo, _Purrr_Program_Vulkan **program);
Purrr_Result _purrr_destroy_program_vulkan(_Purrr_Program_Vulkan *program);

#endif // __PURRR_VULKAN_PROGRAM_H_