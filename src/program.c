#include "purrr/purrr.h"
#include "./internal.h"

#include "./vulkan/program.h"

Purrr_Result purrr_create_program(Purrr_Handle renderTarget, Purrr_Program_Create_Info createInfo, Purrr_Program *program) {
  switch (_purrr_get_header(renderTarget).backend) {
  case PURRR_VULKAN: return _purrr_create_program_vulkan(renderTarget, createInfo, (_Purrr_Program_Vulkan**)program);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_program(Purrr_Program program) {
  switch (_purrr_get_header(program).backend) {
  case PURRR_VULKAN: return _purrr_destroy_program_vulkan((_Purrr_Program_Vulkan*)program);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}