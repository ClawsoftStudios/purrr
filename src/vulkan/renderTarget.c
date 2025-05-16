#include "./renderTarget.h"

#include "format.h"

#include <assert.h>

static Purrr_Result create_render_target(_Purrr_Render_Target_Vulkan *renderTarget);
static void destroy_render_target(_Purrr_Render_Target_Vulkan *renderTarget);



Purrr_Result _purrr_create_render_target_vulkan(_Purrr_Context_Vulkan *context, Purrr_Render_Target_Create_Info createInfo, _Purrr_Render_Target_Vulkan **renderTarget) {
  if (!context || !renderTarget || !createInfo.imageCount) return PURRR_INVALID_ARGS_ERROR;

  if ((createInfo.depthIndex != PURRR_NO_DEPTH) && createInfo.depthIndex >= createInfo.imageCount) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Render_Target_Vulkan *target = _purrr_malloc_with_header((_Purrr_Object_Header){
    .backend = PURRR_VULKAN,
    .type = _PURRR_OBJECT_RENDER_TARGET,
    .flags = _PURRR_OBJECT_FLAG_RENDER_TARGET
  }, sizeof(*target));
  if (!target) return PURRR_BUY_MORE_RAM;
  memset(target, 0, sizeof(*target));

  target->context = context;
  target->imageCount = createInfo.imageCount;
  target->depthIndex = createInfo.depthIndex;

  target->width = createInfo.width;
  target->height = createInfo.height;

  target->images = malloc(sizeof(*target->images) * target->imageCount);
  if (!target->images) return PURRR_BUY_MORE_RAM;

  for (uint32_t i = 0; i < target->imageCount; ++i) {
    Purrr_Image image = createInfo.images[i];
    _Purrr_Object_Header imageHeader = _purrr_get_header(image);
    if (imageHeader.backend != PURRR_VULKAN || imageHeader.type != _PURRR_OBJECT_IMAGE) return PURRR_INVALID_ARGS_ERROR;
    _Purrr_Image_Vulkan *vulkanImage = (_Purrr_Image_Vulkan*)image;
    if (vulkanImage->context != target->context) return PURRR_INVALID_ARGS_ERROR;

    if (!(vulkanImage->usage & PURRR_IMAGE_USAGE_FLAG_ATTACHMENT) && vulkanImage->width != createInfo.width && vulkanImage->height != createInfo.height) return PURRR_INVALID_ARGS_ERROR;

    target->images[i] = vulkanImage;
  }

  Purrr_Result result = PURRR_SUCCESS;
  if ((result = create_render_target(target)) < PURRR_SUCCESS) return result;

  *renderTarget = target;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_render_target_vulkan(_Purrr_Render_Target_Vulkan *renderTarget) {
  if (!renderTarget) return PURRR_INVALID_ARGS_ERROR;

  free(renderTarget->images);
  destroy_render_target(renderTarget);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_get_render_target_image_vulkan(_Purrr_Render_Target_Vulkan *renderTarget, uint32_t imageIndex, Purrr_Image *image) {
  if (!renderTarget || imageIndex > renderTarget->imageCount || !image) return PURRR_INVALID_ARGS_ERROR;
  *image = (Purrr_Image)renderTarget->images[imageIndex];
  return PURRR_SUCCESS;
}

Purrr_Result _purrr_resize_render_target_vulkan(_Purrr_Render_Target_Vulkan *renderTarget, uint32_t newWidth, uint32_t newHeight) {
  if (!renderTarget || !newWidth || !newHeight) return PURRR_INVALID_ARGS_ERROR;

  destroy_render_target(renderTarget);

  Purrr_Result result = PURRR_SUCCESS;
  for (uint32_t i = 0; i < renderTarget->imageCount; ++i) {
    _Purrr_Image_Vulkan *image = renderTarget->images[i];

    if ((result = purrr_create_image((Purrr_Context)renderTarget->context, (Purrr_Image_Create_Info){
      .usage = image->usage,
      .format = image->format,
      .width = newWidth,
      .height = newHeight,
      .pixels = NULL,
      .sampler = (Purrr_Sampler)image->sampler
    }, (Purrr_Image*)&renderTarget->images[i])) < PURRR_SUCCESS) return result;

    VkImageLayout layout = renderTarget->images[i]->layout;
    VkAccessFlags accessMask = renderTarget->images[i]->accessMask;
    VkPipelineStageFlags stageMask = renderTarget->images[i]->stageMask;

    if ((result = _purrr_transition_image_layout_vulkan(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT)) < PURRR_SUCCESS) return result;
    if ((result = _purrr_transition_image_layout_vulkan(renderTarget->images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT)) < PURRR_SUCCESS) return result;

    if ((result = _purrr_copy_image_from_image_vulkan(renderTarget->images[i], image)) < PURRR_SUCCESS) return result;

    if ((result = _purrr_transition_image_layout_vulkan(renderTarget->images[i], layout, accessMask, stageMask)) < PURRR_SUCCESS) return result;

    (void)purrr_destroy_image((Purrr_Image)image);
  }

  return create_render_target(renderTarget);
}



static Purrr_Result create_render_target(_Purrr_Render_Target_Vulkan *renderTarget) {
  if (!renderTarget) return PURRR_INVALID_ARGS_ERROR;

  VkImageView *imageViews = malloc(sizeof(*imageViews) * renderTarget->imageCount);
  if (!imageViews) return PURRR_BUY_MORE_RAM;
  VkAttachmentDescription *attachments = malloc(sizeof(*attachments) * renderTarget->imageCount);
  if (!attachments) return PURRR_BUY_MORE_RAM;

  bool doDepth = renderTarget->depthIndex!=PURRR_NO_DEPTH;
  VkAttachmentReference *references = malloc(sizeof(*references) * (renderTarget->imageCount - doDepth));
  if (!references) return PURRR_BUY_MORE_RAM;

  VkAttachmentReference *depthReference = (doDepth?malloc(sizeof(*depthReference)):VK_NULL_HANDLE);
  if (doDepth && !depthReference) return PURRR_BUY_MORE_RAM;

  uint32_t colorAttachmentCount = 0;
  for (uint32_t i = 0; i < renderTarget->imageCount; ++i) {
    imageViews[i] = renderTarget->images[i]->view;

    attachments[i] = (VkAttachmentDescription){
      .flags = 0,
      .format = _purrr_format_to_vk_format(renderTarget->images[i]->format),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = renderTarget->images[i]->layout,
      .finalLayout = renderTarget->images[i]->layout
    };

    VkAttachmentReference *referencePtr = ((i != renderTarget->depthIndex)?&references[colorAttachmentCount++]:depthReference);
    assert(referencePtr);
    *referencePtr = (VkAttachmentReference){
      .attachment = i,
      .layout = ((renderTarget->images[i]->aspectFlags==VK_IMAGE_ASPECT_COLOR_BIT)?VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    };
  }

  {
    VkSubpassDescription subpass = {
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = VK_NULL_HANDLE,
      .colorAttachmentCount = colorAttachmentCount,
      .pColorAttachments = references,
      .pResolveAttachments = VK_NULL_HANDLE,
      .pDepthStencilAttachment = depthReference,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = VK_NULL_HANDLE
    };

    VkSubpassDependency dependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0
    };

    VkRenderPassCreateInfo renderPassCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .attachmentCount = renderTarget->imageCount,
      .pAttachments = attachments,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency
    };

    if (vkCreateRenderPass(renderTarget->context->device, &renderPassCreateInfo, VK_NULL_HANDLE, &renderTarget->renderPass) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  }

  {
    VkFramebufferCreateInfo frameBufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .renderPass = renderTarget->renderPass,
      .attachmentCount = renderTarget->imageCount,
      .pAttachments = imageViews,
      .width = renderTarget->width,
      .height = renderTarget->height,
      .layers = 1
    };

    if (vkCreateFramebuffer(renderTarget->context->device, &frameBufferCreateInfo, VK_NULL_HANDLE, &renderTarget->framebuffer) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  }

  free(attachments);
  free(references);
  free(depthReference);
  free(imageViews);

  return PURRR_SUCCESS;
}

static void destroy_render_target(_Purrr_Render_Target_Vulkan *renderTarget) {
  vkDestroyRenderPass(renderTarget->context->device, renderTarget->renderPass, VK_NULL_HANDLE);
  vkDestroyFramebuffer(renderTarget->context->device, renderTarget->framebuffer, VK_NULL_HANDLE);
}