#include "./sampler.h"

static VkFilter filter_to_vk(Purrr_Filter filter);
static VkSamplerAddressMode sampler_address_mode_to_vk(Purrr_Sampler_Address_Mode addressMode);
static VkBorderColor sampler_border_color_to_vk(Purrr_Sampler_Border_Color borderColor);

Purrr_Result _purrr_create_sampler_vulkan(_Purrr_Context_Vulkan *context, Purrr_Sampler_Create_Info createInfo, _Purrr_Sampler_Vulkan **sampler) {
  if (!context || !sampler) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Sampler_Vulkan *smplr = _purrr_malloc_with_header((_Purrr_Object_Header){
    .backend = PURRR_VULKAN,
    .type = _PURRR_OBJECT_SAMPLER,
    .flags = 0
  }, sizeof(*smplr));
  if (!smplr) return PURRR_BUY_MORE_RAM;
  memset(smplr, 0, sizeof(*smplr));

  smplr->context = context;

  VkSamplerCreateInfo samplerCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .magFilter = filter_to_vk(createInfo.magFilter),
    .minFilter = filter_to_vk(createInfo.minFilter),
    .mipmapMode = (VkSamplerMipmapMode)filter_to_vk(createInfo.mipFilter),
    .addressModeU = sampler_address_mode_to_vk(createInfo.addressModeU),
    .addressModeV = sampler_address_mode_to_vk(createInfo.addressModeV),
    .addressModeW = sampler_address_mode_to_vk(createInfo.addressModeW),
    .mipLodBias = createInfo.mipLodBias,
    .anisotropyEnable = VK_FALSE,
    .maxAnisotropy = createInfo.maxAnisotropy,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_NEVER,
    .minLod = createInfo.minLod,
    .maxLod = createInfo.maxLod,
    .borderColor = sampler_border_color_to_vk(createInfo.borderColor),
    .unnormalizedCoordinates = VK_FALSE
  };

  if (vkCreateSampler(context->device, &samplerCreateInfo, VK_NULL_HANDLE, &smplr->sampler) != VK_SUCCESS) {
    _purrr_destroy_sampler_vulkan(smplr);
    return PURRR_INTERNAL_ERROR;
  }

  *sampler = smplr;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_sampler_vulkan(_Purrr_Sampler_Vulkan *sampler) {
  if (!sampler) return PURRR_INVALID_ARGS_ERROR;

  if (sampler->sampler) vkDestroySampler(sampler->context->device, sampler->sampler, VK_NULL_HANDLE);

  _purrr_free_with_header(sampler);

  return PURRR_SUCCESS;
}



static VkFilter filter_to_vk(Purrr_Filter filter) {
  switch (filter) {
  case PURRR_NEAREST: return VK_FILTER_NEAREST;
  case PURRR_LINEAR: return VK_FILTER_LINEAR;
  case COUNT_PURRR_FILTERS:
  default: return VK_FILTER_MAX_ENUM;
  }
}

static VkSamplerAddressMode sampler_address_mode_to_vk(Purrr_Sampler_Address_Mode addressMode) {
  switch (addressMode) {
  case PURRR_SAMPLER_ADDRESS_REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  case PURRR_SAMPLER_ADDRESS_MIRROR: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  case PURRR_SAMPLER_ADDRESS_CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  case PURRR_SAMPLER_ADDRESS_CLAMP_TO_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  case COUNT_PURRR_SAMPLER_ADRESS_MODES:
  default: return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
  }
}

static VkBorderColor sampler_border_color_to_vk(Purrr_Sampler_Border_Color borderColor) {
  switch (borderColor) {
  case PURRR_SAMPLER_BORDER_FLOAT_TRANSPARENT_BLACK: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  case PURRR_SAMPLER_BORDER_INT_TRANSPARENT_BLACK: return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
  case PURRR_SAMPLER_BORDER_FLOAT_OPAQUE_BLACK: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
  case PURRR_SAMPLER_BORDER_INT_OPAQUE_BLACK: return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  case PURRR_SAMPLER_BORDER_FLOAT_OPAQUE_WHITE: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  case PURRR_SAMPLER_BORDER_INT_OPAQUE_WHITE: return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  case COUNT_PURRR_SAMPLER_BORDER_COLORS:
  default: return VK_BORDER_COLOR_MAX_ENUM;
  }
}