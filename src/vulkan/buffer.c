#include "./buffer.h"
#include "./window.h"

Purrr_Result _purrr_vulkan_create_buffer(_Purrr_Context_Vulkan *context, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *memory) {
  if (!size || !usage || !buffer || !memory) return PURRR_INVALID_ARGS_ERROR;

  VkBufferCreateInfo bufferCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = VK_NULL_HANDLE
  };

  if (vkCreateBuffer(context->device, &bufferCreateInfo, VK_NULL_HANDLE, buffer) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkMemoryRequirements memRequirements = {0};
  vkGetBufferMemoryRequirements(context->device, *buffer, &memRequirements);

  VkPhysicalDeviceMemoryProperties memProperties = {0};
  vkGetPhysicalDeviceMemoryProperties(context->gpu, &memProperties);

  VkMemoryAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .allocationSize = memRequirements.size,
    .memoryTypeIndex = _purrr_vulkan_find_memory_type(memRequirements.memoryTypeBits, properties, memProperties)
  };

  if (vkAllocateMemory(context->device, &allocInfo, VK_NULL_HANDLE, memory) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  if (vkBindBufferMemory(context->device, *buffer, *memory, 0) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_create_buffer_vulkan(_Purrr_Context_Vulkan *context, Purrr_Buffer_Create_Info createInfo, _Purrr_Buffer_Vulkan **buffer) {
  if (!context || !buffer || createInfo.type >= COUNT_PURRR_BUFFER_TYPES) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Buffer_Vulkan *buf = _purrr_malloc_with_header((_Purrr_Object_Header){
    .backend = PURRR_VULKAN,
    .type = _PURRR_OBJECT_BUFFER,
    .flags = 0
  }, sizeof(*buf));
  if (!buf) return PURRR_BUY_MORE_RAM;
  memset(buf, 0, sizeof(*buf));

  buf->context = context;
  buf->type = createInfo.type;
  buf->size = createInfo.size;

  Purrr_Result result = PURRR_SUCCESS;
  switch (createInfo.type) {
  case PURRR_BUFFER_VERTEX: {
    result = _purrr_vulkan_create_buffer(context, createInfo.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buf->buffer, &buf->memory);
  } break;
  case PURRR_BUFFER_INDEX: {
    result = _purrr_vulkan_create_buffer(context, createInfo.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buf->buffer, &buf->memory);
  } break;
  case PURRR_BUFFER_UNIFORM: {
    result = _purrr_vulkan_create_buffer(context, createInfo.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buf->buffer, &buf->memory);
  } break;
  case COUNT_PURRR_BUFFER_TYPES:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  if (result < PURRR_SUCCESS) return result;

  if (createInfo.type == PURRR_BUFFER_UNIFORM) {
    VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .descriptorPool = context->descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &context->uniformBufferLayout
    };

    if (vkAllocateDescriptorSets(context->device, &allocInfo, &buf->descriptorSet) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

    VkDescriptorBufferInfo bufferInfo = {
      .buffer = buf->buffer,
      .offset = 0,
      .range = createInfo.size
    };

    VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = VK_NULL_HANDLE,
      .dstSet = buf->descriptorSet,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pImageInfo = VK_NULL_HANDLE,
      .pBufferInfo = &bufferInfo,
      .pTexelBufferView = VK_NULL_HANDLE
    };

    vkUpdateDescriptorSets(context->device, 1, &write, 0, VK_NULL_HANDLE);
  }

  *buffer = buf;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_buffer_vulkan(_Purrr_Buffer_Vulkan *buffer) {
  if (!buffer) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Context_Vulkan *context = buffer->context;
  if (buffer->buffer) vkDestroyBuffer(context->device, buffer->buffer, VK_NULL_HANDLE);
  if (buffer->memory) vkFreeMemory(context->device, buffer->memory, VK_NULL_HANDLE);
  if (buffer->descriptorSet) vkFreeDescriptorSets(context->device, context->descriptorPool, 1, &buffer->descriptorSet);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_copy_buffer_data_vulkan(_Purrr_Buffer_Vulkan *dst, void *src, uint32_t size, uint32_t offset) {
  if (!dst || !src || !size || size+offset > dst->size) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Context_Vulkan *context = dst->context;

  if (dst->type == PURRR_BUFFER_UNIFORM) {
    void *data = NULL;
    if (vkMapMemory(context->device, dst->memory, 0, size, 0, &data) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
    memcpy(data, src, size);
    vkUnmapMemory(context->device, dst->memory);
  } else {
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    Purrr_Result result = PURRR_SUCCESS;
    if ((result = _purrr_vulkan_create_buffer(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingMemory)) < PURRR_SUCCESS) return result;

    {
      void *data = NULL;
      if (vkMapMemory(context->device, stagingMemory, 0, size, 0, &data) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
      memcpy(data, src, size);
      vkUnmapMemory(context->device, stagingMemory);
    }

    VkCommandBuffer cmdBuf = VK_NULL_HANDLE;
    if ((result = _purrr_context_begin_one_time_command_buffer_vulkan(context, &cmdBuf)) < PURRR_SUCCESS) return result;

    VkBufferCopy copyRegion = {
      .srcOffset = 0,
      .dstOffset = offset,
      .size = size
    };

    vkCmdCopyBuffer(cmdBuf, stagingBuffer, dst->buffer, 1, &copyRegion);

    if ((result = _purrr_context_submit_one_time_command_buffer_vulkan(context, cmdBuf)) < PURRR_SUCCESS) return result;

    vkDestroyBuffer(context->device, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(context->device, stagingMemory, VK_NULL_HANDLE);
  }

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_copy_buffer_vulkan(_Purrr_Buffer_Vulkan *dst, _Purrr_Buffer_Vulkan *src, uint32_t offset) {
  if (!dst || !src || src->size+offset > dst->size) return PURRR_INVALID_ARGS_ERROR;

  return PURRR_SUCCESS;
}