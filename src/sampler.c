#include "purrr/purrr.h"
#include "./internal.h"

#include "./vulkan/sampler.h"

Purrr_Result purrr_create_sampler(Purrr_Context context, Purrr_Sampler_Create_Info createInfo, Purrr_Sampler *sampler) {
  switch (_purrr_get_header(context).backend) {
  case PURRR_VULKAN: return _purrr_create_sampler_vulkan((_Purrr_Context_Vulkan*)context, createInfo, (_Purrr_Sampler_Vulkan**)sampler);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_sampler(Purrr_Sampler sampler) {
  switch (_purrr_get_header(sampler).backend) {
  case PURRR_VULKAN: return _purrr_destroy_sampler_vulkan((_Purrr_Sampler_Vulkan*)sampler);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}