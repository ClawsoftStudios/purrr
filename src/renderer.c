#include "purrr/purrr.h"

#include "vulkan/renderer.h"

Purrr_Result purrr_create_renderer(Purrr_Context context, Purrr_Renderer_Create_Info createInfo, Purrr_Renderer *renderer) {
  switch (_purrr_get_header(context).backend) {
  case PURRR_VULKAN: return _purrr_create_renderer_vulkan((_Purrr_Context_Vulkan*)context, createInfo, (_Purrr_Renderer_Vulkan**)renderer);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_renderer(Purrr_Renderer renderer) {
  Purrr_Result result = PURRR_SUCCESS;

  switch (_purrr_get_header(renderer).backend) {
  case PURRR_VULKAN: {
    if ((result = _purrr_destroy_renderer_vulkan((_Purrr_Renderer_Vulkan*)renderer)) < PURRR_SUCCESS) return result;
  } break;
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  _purrr_free_with_header(renderer);

  return PURRR_SUCCESS;
}

Purrr_Result purrr_wait_renderer(Purrr_Renderer renderer) {
  switch (_purrr_get_header(renderer).backend) {
  case PURRR_VULKAN: return _purrr_wait_renderer_vulkan((_Purrr_Renderer_Vulkan*)renderer);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_begin_renderer(Purrr_Renderer renderer) {
  switch (_purrr_get_header(renderer).backend) {
  case PURRR_VULKAN: return _purrr_begin_renderer_vulkan((_Purrr_Renderer_Vulkan*)renderer);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_renderer_begin(Purrr_Renderer renderer, void *renderTarget, Purrr_Color color) {
  switch (_purrr_get_header(renderer).backend) {
  case PURRR_VULKAN: return _purrr_renderer_begin_vulkan((_Purrr_Renderer_Vulkan*)renderer, renderTarget, color);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_renderer_end(Purrr_Renderer renderer) {
  switch (_purrr_get_header(renderer).backend) {
  case PURRR_VULKAN: return _purrr_renderer_end_vulkan((_Purrr_Renderer_Vulkan*)renderer);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_render_renderer(Purrr_Renderer renderer) {
  switch (_purrr_get_header(renderer).backend) {
  case PURRR_VULKAN: return _purrr_render_renderer_vulkan((_Purrr_Renderer_Vulkan*)renderer);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}