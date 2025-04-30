#include "purrr/purrr.h"

#include "./vulkan/context.h"

Purrr_Backend _purrr_get_context_backend(Purrr_Context context) {
  if (!context) return COUNT_PURRR_BACKENDS;
  return ((Purrr_Backend*)context)[-1];
}

bool _purrr_is_context_valid(Purrr_Context context) {
  return _purrr_get_context_backend(context) < COUNT_PURRR_BACKENDS;
}

Purrr_Result purrr_create_context(Purrr_Context_Create_Info createInfo, Purrr_Context *context) {
  switch (createInfo.backend) {
  case PURRR_VULKAN: return _purrr_create_context_vulkan(createInfo, (_Purrr_Context_Vulkan**)context);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_context(Purrr_Context context) {
  switch (_purrr_get_context_backend(context)) {
  case PURRR_VULKAN: return _purrr_destroy_context_vulkan((_Purrr_Context_Vulkan*)context);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}