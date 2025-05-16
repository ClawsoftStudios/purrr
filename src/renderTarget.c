#include "purrr/purrr.h"
#include "./internal.h"

#include "./vulkan/renderTarget.h"

Purrr_Result purrr_create_render_target(Purrr_Context context, Purrr_Render_Target_Create_Info createInfo, Purrr_Render_Target *renderTarget) {
  switch (_purrr_get_header(context).backend) {
  case PURRR_VULKAN: return _purrr_create_render_target_vulkan((_Purrr_Context_Vulkan*)context, createInfo, (_Purrr_Render_Target_Vulkan**)renderTarget);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_render_target(Purrr_Render_Target renderTarget) {
  Purrr_Result result = PURRR_SUCCESS;

  switch (_purrr_get_header(renderTarget).backend) {
  case PURRR_VULKAN: {
    if ((result = _purrr_destroy_render_target_vulkan((_Purrr_Render_Target_Vulkan*)renderTarget)) < PURRR_SUCCESS) return result;
  } break;
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  _purrr_free_with_header(renderTarget);

  return PURRR_SUCCESS;
}

Purrr_Result purrr_get_render_target_image(Purrr_Render_Target renderTarget, uint32_t imageIndex, Purrr_Image *image) {
  switch (_purrr_get_header(renderTarget).backend) {
  case PURRR_VULKAN: return _purrr_get_render_target_image_vulkan((_Purrr_Render_Target_Vulkan*)renderTarget, imageIndex, image);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_resize_render_target(Purrr_Render_Target renderTarget, uint32_t newWidth, uint32_t newHeight) {
  switch (_purrr_get_header(renderTarget).backend) {
  case PURRR_VULKAN: return _purrr_resize_render_target_vulkan((_Purrr_Render_Target_Vulkan*)renderTarget, newWidth, newHeight);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}