#include "purrr/purrr.h"
#include "./internal.h"

#include "./vulkan/context.h"

Purrr_Result purrr_create_context(Purrr_Backend backend, Purrr_Context_Create_Info createInfo, Purrr_Context *context) {
  switch (backend) {
  case PURRR_VULKAN: return _purrr_create_context_vulkan(createInfo, (_Purrr_Context_Vulkan**)context);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_context(Purrr_Context context) {
  Purrr_Result result = PURRR_SUCCESS;

  switch (_purrr_get_header(context).backend) {
  case PURRR_VULKAN: {
    if ((result = _purrr_destroy_context_vulkan((_Purrr_Context_Vulkan*)context)) < PURRR_SUCCESS) return result;
  } break;
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  _purrr_free_with_header(context);

  return PURRR_SUCCESS;
}