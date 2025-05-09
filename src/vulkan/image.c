#include "./image.h"

#include "./format.h"

static VkImageUsageFlags image_type_to_usage(Purrr_Image_Type type) {
  switch (type) {
  case PURRR_IMAGE_TEXTURE: return VK_IMAGE_USAGE_SAMPLED_BIT;
  case COUNT_PURRR_IMAGE_TYPES:
  default: return 0;
  }
}

Purrr_Result _purrr_create_image_vulkan(_Purrr_Context_Vulkan *context, Purrr_Image_Create_Info createInfo, _Purrr_Image_Vulkan **image) {
  if (!context || !image) return PURRR_INVALID_ARGS_ERROR;
  if (createInfo.format >= COUNT_PURRR_FORMATS || !createInfo.width || !createInfo.height) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Image_Vulkan *img = _purrr_malloc_with_header((_Purrr_Object_Header){
    .backend = PURRR_VULKAN,
    .type = _PURRR_OBJECT_IMAGE,
    .flags = 0
  }, sizeof(*img));
  if (!img) return PURRR_BUY_MORE_RAM;
  memset(img, 0, sizeof(*img));

  img->context = context;

  img->stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

  img->format = createInfo.format;
  img->width = createInfo.width;
  img->height = createInfo.height;

  VkImageCreateInfo imageCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = _purrr_format_to_vk_format(createInfo.format),
    .extent = {
      .width = createInfo.width,
      .height = createInfo.height,
      .depth = 1
    },
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | image_type_to_usage(createInfo.type),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = VK_NULL_HANDLE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
  };

  if (vkCreateImage(context->device, &imageCreateInfo, VK_NULL_HANDLE, &img->image) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkMemoryRequirements memRequirements = {0};
  vkGetImageMemoryRequirements(context->device, img->image, &memRequirements);

  VkPhysicalDeviceMemoryProperties memProperties = {0};
  vkGetPhysicalDeviceMemoryProperties(context->gpu, &memProperties);

  VkMemoryAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .allocationSize = memRequirements.size,
    .memoryTypeIndex = _purrr_vulkan_find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memProperties)
  };

  if (vkAllocateMemory(context->device, &allocInfo, VK_NULL_HANDLE, &img->memory) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  if (vkBindImageMemory(context->device, img->image, img->memory, 0) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  Purrr_Result result = PURRR_SUCCESS;
  if (createInfo.pixels) {
    if ((result = _purrr_transition_image_layout_vulkan(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT)) < PURRR_SUCCESS) return result;
    if ((result = _purrr_copy_image_vulkan(img, (VkExtent2D){ createInfo.width, createInfo.height }, (VkOffset2D){ 0 }, createInfo.pixels)) < PURRR_SUCCESS) return result;
  }

  if ((result = _purrr_transition_image_layout_vulkan(img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)) < PURRR_SUCCESS) return result;

  VkImageViewCreateInfo viewCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .image = img->image,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = _purrr_format_to_vk_format(createInfo.format),
    .components = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY
    },
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1
    }
  };

  if (vkCreateImageView(context->device, &viewCreateInfo, VK_NULL_HANDLE, &img->view) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  *image = img;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_image_vulkan(_Purrr_Image_Vulkan *image) {
  if (!image) return PURRR_INVALID_ARGS_ERROR;

  if (image->image) vkDestroyImage(image->context->device, image->image, VK_NULL_HANDLE);
  if (image->memory) vkFreeMemory(image->context->device, image->memory, VK_NULL_HANDLE);
  if (image->view) vkDestroyImageView(image->context->device, image->view, VK_NULL_HANDLE);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_transition_image_layout_vulkan(_Purrr_Image_Vulkan *image, VkImageLayout newLayout, VkAccessFlags newAccessMask, VkPipelineStageFlags newStageMask) {
  if (!image) return PURRR_INVALID_ARGS_ERROR;

  Purrr_Result result = PURRR_SUCCESS;

  VkCommandBuffer cmdBuf = VK_NULL_HANDLE;
  if ((result = _purrr_context_begin_one_time_command_buffer_vulkan(image->context, &cmdBuf)) < PURRR_SUCCESS) return result;

  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .pNext = VK_NULL_HANDLE,
    .srcAccessMask = image->accessMask,
    .dstAccessMask = newAccessMask,
    .oldLayout = image->layout,
    .newLayout = newLayout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image->image,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1
    }
  };

  vkCmdPipelineBarrier(
    cmdBuf,
    image->stageMask, newStageMask,
    0,
    0, VK_NULL_HANDLE,
    0, VK_NULL_HANDLE,
    1, &barrier
  );

  if ((result = _purrr_context_submit_one_time_command_buffer_vulkan(image->context, cmdBuf)) < PURRR_SUCCESS) return result;

  image->layout = newLayout;
  image->accessMask = newAccessMask;
  image->stageMask = newStageMask;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_vulkan_create_buffer(_Purrr_Context_Vulkan *context, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *memory);

Purrr_Result _purrr_copy_image_vulkan(_Purrr_Image_Vulkan *image, VkExtent2D size, VkOffset2D offset, void *pixels) {
  if (!image || !size.width || !size.height || !pixels) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Context_Vulkan *context = image->context;

  Purrr_Result result = PURRR_SUCCESS;

  VkBuffer stagingBuffer = VK_NULL_HANDLE;
  VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
  uint32_t bufSize = size.width * size.height * _purrr_formats_channel_count[image->format] * _purrr_formats_size[image->format];
  if ((result = _purrr_vulkan_create_buffer(context, bufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingMemory)) < PURRR_SUCCESS) return result;

  {
    void *data = NULL;
    if (vkMapMemory(context->device, stagingMemory, 0, bufSize, 0, &data) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
    memcpy(data, pixels, bufSize);
    vkUnmapMemory(context->device, stagingMemory);
  }

  VkCommandBuffer cmdBuf = VK_NULL_HANDLE;
  if ((result = _purrr_context_begin_one_time_command_buffer_vulkan(context, &cmdBuf)) < PURRR_SUCCESS) return result;

  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = 1
    },
    .imageOffset = { offset.x, offset.y, 0 },
    .imageExtent = { size.width, size.height, 1 },
  };

  vkCmdCopyBufferToImage(
    cmdBuf,
    stagingBuffer,
    image->image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
  );

  if ((result = _purrr_context_submit_one_time_command_buffer_vulkan(context, cmdBuf)) < PURRR_SUCCESS) return result;

  vkDestroyBuffer(context->device, stagingBuffer, VK_NULL_HANDLE);
  vkFreeMemory(context->device, stagingMemory, VK_NULL_HANDLE);

  return PURRR_SUCCESS;
}