#include "purrr/purrr.h"
#include "./internal.h"

#include "./vulkan/image.h"

Purrr_Result purrr_create_image(Purrr_Context context, Purrr_Image_Create_Info createInfo, Purrr_Image *image) {
  switch (_purrr_get_header(context).backend) {
  case PURRR_VULKAN: return _purrr_create_image_vulkan((_Purrr_Context_Vulkan*)context, createInfo, (_Purrr_Image_Vulkan**)image);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}

Purrr_Result purrr_destroy_image(Purrr_Image image) {
  switch (_purrr_get_header(image).backend) {
  case PURRR_VULKAN: return _purrr_destroy_image_vulkan((_Purrr_Image_Vulkan*)image);
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INVALID_ARGS_ERROR;
  }
}