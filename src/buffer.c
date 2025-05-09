#include "purrr/purrr.h"
#include "./internal.h"

#include "./vulkan/buffer.h"

Purrr_Result purrr_create_buffer(Purrr_Context context, Purrr_Buffer_Create_Info createInfo, Purrr_Buffer *buffer) {
  switch (_purrr_get_header(context).backend) {
  case PURRR_VULKAN: return _purrr_create_buffer_vulkan((_Purrr_Context_Vulkan*)context, createInfo, (_Purrr_Buffer_Vulkan**)buffer);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_buffer(Purrr_Buffer buffer) {
  Purrr_Result result = PURRR_SUCCESS;

  switch (_purrr_get_header(buffer).backend) {
  case PURRR_VULKAN: {
    if ((result = _purrr_destroy_buffer_vulkan((_Purrr_Buffer_Vulkan*)buffer)) < PURRR_SUCCESS) return result;
  } break;
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  _purrr_free_with_header(buffer);

  return PURRR_SUCCESS;
}

Purrr_Result purrr_copy_buffer_data(Purrr_Buffer dst, void *src, uint32_t size, uint32_t offset) {
  switch (_purrr_get_header(dst).backend) {
  case PURRR_VULKAN: return _purrr_copy_buffer_data_vulkan((_Purrr_Buffer_Vulkan*)dst, src, size, offset);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_copy_buffer(Purrr_Buffer dst, Purrr_Buffer src, uint32_t offset) {
  switch (_purrr_get_header(dst).backend) {
  case PURRR_VULKAN: return _purrr_copy_buffer_vulkan((_Purrr_Buffer_Vulkan*)dst, (_Purrr_Buffer_Vulkan*)src, offset);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}