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

  bool ownsImage;
  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  VkImageAspectFlags aspectFlags;

  VkDescriptorSet descriptorSet;

  VkImageLayout layout;
  VkAccessFlags accessMask;
  VkPipelineStageFlags stageMask;
} _Purrr_Image_Vulkan;

typedef struct _Purrr_Image_Create_Info_Vulkan_Ex {
  Purrr_Image_Usage_Flags usage;
  Purrr_Format format;
  uint32_t width, height;
  void *pixels;
  Purrr_Sampler sampler;

  VkImage image;
  bool createMemory;
} _Purrr_Image_Create_Info_Vulkan_Ex;

Purrr_Result _purrr_create_image_vulkan(_Purrr_Context_Vulkan *context, Purrr_Image_Create_Info createInfo, _Purrr_Image_Vulkan **image);
Purrr_Result _purrr_create_image_vulkan_ex(_Purrr_Context_Vulkan *context, _Purrr_Image_Create_Info_Vulkan_Ex createInfo, _Purrr_Image_Vulkan **image);
Purrr_Result _purrr_destroy_image_vulkan(_Purrr_Image_Vulkan *image);

Purrr_Result _purrr_transition_image_layout_vulkan(_Purrr_Image_Vulkan *image, VkImageLayout newLayout, VkAccessFlags newAccessMask, VkPipelineStageFlags newStageMask);
Purrr_Result _purrr_copy_image_vulkan(_Purrr_Image_Vulkan *image, VkExtent2D size, VkOffset2D offset, void *pixels);
Purrr_Result _purrr_copy_image_from_image_vulkan(_Purrr_Image_Vulkan *image, _Purrr_Image_Vulkan *source);



VkImageUsageFlags _purrr_image_usage_flags_to_vk_usage(Purrr_Image_Usage_Flags usage, bool color);

#endif //  __PURRR_VULKAN_IMAGE_H_