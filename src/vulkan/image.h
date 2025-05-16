#ifndef   __PURRR_VULKAN_IMAGE_H_
#define   __PURRR_VULKAN_IMAGE_H_

#include "./vulkan.h"
#include "./context.h"
#include "./sampler.h"

#include "../internal.h"

typedef struct _Purrr_Image_Vulkan {
  _Purrr_Context_Vulkan *context;
  Purrr_Image_Usage_Flags usage;

  _Purrr_Sampler_Vulkan *sampler;

  Purrr_Format format;
  uint32_t width, height;

  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  VkImageAspectFlags aspectFlags;

  VkDescriptorSet descriptorSet;

  VkImageLayout layout;
  VkAccessFlags accessMask;
  VkPipelineStageFlags stageMask;
} _Purrr_Image_Vulkan;

Purrr_Result _purrr_create_image_vulkan(_Purrr_Context_Vulkan *context, Purrr_Image_Create_Info createInfo, _Purrr_Image_Vulkan **image);
Purrr_Result _purrr_destroy_image_vulkan(_Purrr_Image_Vulkan *image);



Purrr_Result _purrr_transition_image_layout_vulkan(_Purrr_Image_Vulkan *image, VkImageLayout newLayout, VkAccessFlags newAccessMask, VkPipelineStageFlags newStageMask);
Purrr_Result _purrr_copy_image_vulkan(_Purrr_Image_Vulkan *image, VkExtent2D size, VkOffset2D offset, void *pixels);
Purrr_Result _purrr_copy_image_from_image_vulkan(_Purrr_Image_Vulkan *image, _Purrr_Image_Vulkan *source);

#endif //  __PURRR_VULKAN_IMAGE_H_