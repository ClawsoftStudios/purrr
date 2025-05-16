#ifndef   __PURRR_VULKAN_RENDER_TARGET_H_
#define   __PURRR_VULKAN_RENDER_TARGET_H_

#include "./vulkan.h"
#include "./context.h"
#include "./image.h"

typedef struct _Purrr_Render_Target_Vulkan {
  _Purrr_Context_Vulkan *context;

  uint32_t imageCount;
  _Purrr_Image_Vulkan **images;
  uint32_t depthIndex;

  uint32_t width, height;

  VkRenderPass renderPass;
  VkFramebuffer framebuffer;
} _Purrr_Render_Target_Vulkan;

Purrr_Result _purrr_create_render_target_vulkan(_Purrr_Context_Vulkan *context, Purrr_Render_Target_Create_Info createInfo, _Purrr_Render_Target_Vulkan **renderTarget);
Purrr_Result _purrr_destroy_render_target_vulkan(_Purrr_Render_Target_Vulkan *renderTarget);

Purrr_Result _purrr_get_render_target_image_vulkan(_Purrr_Render_Target_Vulkan *renderTarget, uint32_t imageIndex, Purrr_Image *image);

Purrr_Result _purrr_resize_render_target_vulkan(_Purrr_Render_Target_Vulkan *renderTarget, uint32_t newWidth, uint32_t newHeight);

#endif // __PURRR_VULKAN_RENDER_TARGET_H_