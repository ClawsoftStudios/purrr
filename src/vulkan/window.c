#include "./window.h"

#include <stdio.h>

#ifndef min
#define min(a, b) ((a<b)?(a):(b))
#endif // min

#ifndef max
#define max(a, b) ((a>b)?(a):(b))
#endif // max

#ifndef clamp
#define clamp(x, a, b) max(min(x, a), b)
#endif // clamp

static Purrr_Result create_swapchain(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, Purrr_Image_Usage_Flags imageUsage, uint32_t *width, uint32_t *height, VkFormat *format, VkSwapchainKHR *swapchain);
static Purrr_Result create_render_pass(VkDevice device, VkFormat format, VkFormat depthFormat, bool depth, VkRenderPass *renderPass);
static Purrr_Result create_framebuffers(VkDevice device, VkRenderPass renderPass, uint32_t width, uint32_t height, bool depth, uint32_t imageCount, _Purrr_Image_Vulkan **depthImages, _Purrr_Image_Vulkan **images, VkFramebuffer *framebuffers);

static Purrr_Result create_full_swapchain(_Purrr_Window_Vulkan *window, Purrr_Window realWindow);
static void destroy_swapchain(_Purrr_Window_Vulkan *window);

Purrr_Result _purrr_create_window_vulkan(_Purrr_Renderer_Vulkan *renderer, Purrr_Window_Create_Info createInfo, void *platform, _Purrr_Window_Vulkan *window, Purrr_Window realWindow) {
  if (!renderer || !platform || !window) return PURRR_INVALID_ARGS_ERROR;

  window->renderer = renderer;

  Purrr_Result result = PURRR_SUCCESS;
  _Purrr_Context_Vulkan *context = renderer->context;
  if ((result = _purrr_create_window_surface(context, platform, window)) < PURRR_SUCCESS) return result;

  window->depth = createInfo.depth;
  window->imageUsage = PURRR_IMAGE_USAGE_FLAG_ATTACHMENT | createInfo.imageUsage;
  window->sampler = createInfo.sampler;

  if ((result = create_full_swapchain(window, realWindow)) < PURRR_SUCCESS) return result;

  if ((result = _purrr_add_renderer_window(renderer, realWindow, window, &window->index)) < PURRR_SUCCESS) return result;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_window_vulkan(_Purrr_Window_Vulkan *window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Renderer_Vulkan *renderer = window->renderer;
  _Purrr_Context_Vulkan *context = renderer->context;

  vkDeviceWaitIdle(context->device);

  destroy_swapchain(window);

  if (window->surface) vkDestroySurfaceKHR(context->instance, window->surface, VK_NULL_HANDLE);

  Purrr_Result result = PURRR_SUCCESS;
  if ((result = _purrr_remove_renderer_window(renderer, window->index)) < PURRR_SUCCESS) return result;

  free(window);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_get_window_image_vulkan(_Purrr_Window_Vulkan *window, _Purrr_Image_Vulkan **image) {
  if (!window || !image) return PURRR_INVALID_ARGS_ERROR;
  *image = window->images[window->renderer->windows.imageIndices[window->index]];
  return PURRR_SUCCESS;
}



Purrr_Result _purrr_recreate_window_swapchain(Purrr_Window realWindow, _Purrr_Window_Vulkan *vulkanWindow) {
  if (!realWindow || !vulkanWindow) return PURRR_INVALID_ARGS_ERROR;

  purrr_poll_windows();

  if (!realWindow->width || !realWindow->height) {
    vulkanWindow->fullWidth = vulkanWindow->fullHeight = 0;
    return PURRR_MINIMIZED;
  }

  if (realWindow->width == vulkanWindow->fullWidth && realWindow->height == vulkanWindow->fullHeight) return PURRR_SUCCESS;

  vkDeviceWaitIdle(vulkanWindow->renderer->context->device);

  destroy_swapchain(vulkanWindow);

  Purrr_Result result = PURRR_SUCCESS;
  if ((result = create_full_swapchain(vulkanWindow, realWindow)) < PURRR_SUCCESS) return result;

  vulkanWindow->renderer->windows.swapchains[vulkanWindow->index] = vulkanWindow->swapchain;

  return PURRR_TRUE;
}

static Purrr_Result create_swapchain(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, Purrr_Image_Usage_Flags imageUsage, uint32_t *width, uint32_t *height, VkFormat *format, VkSwapchainKHR *swapchain) {
  if (!gpu || !device || !surface || !width || !height || !format || !swapchain) return PURRR_INVALID_ARGS_ERROR;

  VkSurfaceCapabilitiesKHR capabilities = {0};
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkSurfaceFormatKHR surfaceFormat = {0};
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  VkExtent2D extent = {0};

  {
    uint32_t formatCount = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, VK_NULL_HANDLE) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
    if (!formatCount) return PURRR_INTERNAL_ERROR;

    VkSurfaceFormatKHR *formats = malloc(sizeof(*formats) * formatCount);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, formats) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

    surfaceFormat = formats[0];

    for (uint32_t i = 0; i < formatCount; ++i) {
      VkSurfaceFormatKHR thisFormat = formats[i];
      if (thisFormat.format == VK_FORMAT_B8G8R8A8_UNORM && thisFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        surfaceFormat = thisFormat;
        break;
      }
    }

    free(formats);
  }

  {
    uint32_t presentModeCount = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, VK_NULL_HANDLE) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
    if (!presentModeCount) return PURRR_INTERNAL_ERROR;

    VkPresentModeKHR *presentModes = malloc(sizeof(*presentModes) * presentModeCount);
    if (!presentModes) return PURRR_BUY_MORE_RAM;

    if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

    for (uint32_t i = 0; i < presentModeCount; ++i) {
      if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        presentMode = presentModes[i];
    }

    free(presentModes);
  }

  if (capabilities.currentExtent.width != UINT32_MAX) extent = capabilities.currentExtent;
  else {
    extent = (VkExtent2D){
      clamp(*width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
      clamp(*height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
  }

  *width = extent.width;
  *height = extent.height;

  VkSwapchainCreateInfoKHR createInfo = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .surface = surface,
    .minImageCount = capabilities.minImageCount,
    .imageFormat = surfaceFormat.format,
    .imageColorSpace = surfaceFormat.colorSpace,
    .imageExtent = extent,
    .imageArrayLayers = 1,
    .imageUsage = _purrr_image_usage_flags_to_vk_usage(imageUsage, true),
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = VK_NULL_HANDLE,
    .preTransform = capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentMode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  *format = surfaceFormat.format;

  return (vkCreateSwapchainKHR(device, &createInfo, VK_NULL_HANDLE, swapchain) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}

static Purrr_Result create_render_pass(VkDevice device, VkFormat format, VkFormat depthFormat, bool depth, VkRenderPass *renderPass) {
  if (!device || !format || !renderPass) return PURRR_INVALID_ARGS_ERROR;

  VkAttachmentDescription *attachmentDescriptions = malloc(sizeof(*attachmentDescriptions) * (1+depth));
  VkAttachmentReference *attachmentReferences = malloc(sizeof(*attachmentReferences) * (1+depth));
  attachmentDescriptions[0] = (VkAttachmentDescription){
    .format = format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };

  attachmentReferences[0] = (VkAttachmentReference){
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  if (depth) {
    attachmentDescriptions[1] = (VkAttachmentDescription){
      .format = depthFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    attachmentReferences[1] = (VkAttachmentReference){
      .attachment = 1,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
  }

  VkSubpassDescription subpass = {
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = VK_NULL_HANDLE,
    .colorAttachmentCount = 1,
    .pColorAttachments = attachmentReferences,
    .pResolveAttachments = VK_NULL_HANDLE,
    .pDepthStencilAttachment = (depth?&attachmentReferences[1]:VK_NULL_HANDLE),
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

  VkRenderPassCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .attachmentCount = 1+depth,
    .pAttachments = attachmentDescriptions,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency
  };

  return (vkCreateRenderPass(device, &createInfo, VK_NULL_HANDLE, renderPass) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}

static Purrr_Result create_framebuffers(VkDevice device, VkRenderPass renderPass, uint32_t width, uint32_t height, bool depth, uint32_t imageCount, _Purrr_Image_Vulkan **depthImages, _Purrr_Image_Vulkan **images, VkFramebuffer *framebuffers) {
  if (!device || !renderPass || !imageCount || !images || !framebuffers) return PURRR_INVALID_ARGS_ERROR;

  VkFramebufferCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .renderPass = renderPass,
    .attachmentCount = 1+depth,
    .pAttachments = VK_NULL_HANDLE,
    .width = width,
    .height = height,
    .layers = 1
  };

  VkImageView *attachments = malloc(sizeof(*createInfo.pAttachments) * createInfo.attachmentCount);
  if (!attachments) return PURRR_BUY_MORE_RAM;
  createInfo.pAttachments = attachments;

  for (uint32_t i = 0; i < imageCount; ++i) {
    attachments[0] = images[i]->view;
    if (depth) attachments[1] = depthImages[i]->view;

    if (vkCreateFramebuffer(device, &createInfo, VK_NULL_HANDLE, &framebuffers[i]) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  }

  return PURRR_SUCCESS;
}

static Purrr_Result create_full_swapchain(_Purrr_Window_Vulkan *window, Purrr_Window realWindow) {
  _Purrr_Context_Vulkan *context = window->renderer->context;

  Purrr_Result result = PURRR_SUCCESS;

  window->width = window->fullWidth = realWindow->width;
  window->height = window->fullHeight = realWindow->height;
  if ((result = create_swapchain(context->gpu, context->device, window->surface, window->imageUsage, &window->width, &window->height, &window->format, &window->swapchain)) < PURRR_SUCCESS) return result;

  {
    if (vkGetSwapchainImagesKHR(context->device, window->swapchain, &window->imageCount, VK_NULL_HANDLE) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

    VkImage *swapchainImages = malloc(sizeof(*swapchainImages) * window->imageCount);
    if (!swapchainImages) return PURRR_BUY_MORE_RAM;
    if (vkGetSwapchainImagesKHR(context->device, window->swapchain, &window->imageCount, swapchainImages) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

    window->images = malloc(sizeof(*window->images) * window->imageCount);
    if (!window->images) return PURRR_BUY_MORE_RAM;

    _Purrr_Image_Create_Info_Vulkan_Ex createInfo = {
      .usage = window->imageUsage,
      .format = PURRR_B8G8R8A8_UNORM,
      .width = window->width,
      .height = window->height,
      .pixels = NULL,
      .sampler = window->sampler,
      .image = VK_NULL_HANDLE,
      .createMemory = false
    };

    for (uint32_t i = 0; i < window->imageCount; ++i) {
      createInfo.image = swapchainImages[i];
      if ((result = _purrr_create_image_vulkan_ex(context, createInfo, &window->images[i])) < PURRR_SUCCESS) return result;
    }

    free(swapchainImages);

    window->framebuffers = malloc(sizeof(*window->framebuffers) * window->imageCount);
    if (!window->framebuffers) return PURRR_BUY_MORE_RAM;
  }

  window->renderSemaphores = malloc(sizeof(*window->renderSemaphores) * window->imageCount);
  if (!window->renderSemaphores) return PURRR_BUY_MORE_RAM;

  for (uint32_t i = 0; i < window->imageCount; ++i) {
    VkSemaphoreCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0
    };

    if (vkCreateSemaphore(context->device, &createInfo, VK_NULL_HANDLE, &window->renderSemaphores[i]) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  }

  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
  if (window->depth) {
    window->depthImages = malloc(sizeof(*window->depthImages) * window->imageCount);
    if (!window->depthImages) return PURRR_BUY_MORE_RAM;

    for (uint32_t i = 0; i < window->imageCount; ++i) {
      if ((result = _purrr_create_image_vulkan(context, (Purrr_Image_Create_Info){
        .usage = PURRR_IMAGE_USAGE_FLAG_ATTACHMENT,
        .format = PURRR_D32_SFLOAT_S8_UINT,
        .width = window->width,
        .height = window->height,
        .pixels = NULL,
        .sampler = PURRR_NULL_HANDLE
      }, &window->depthImages[i])) < PURRR_SUCCESS) return result;
    }
  }

  if ((result = create_render_pass(context->device, window->format, depthFormat, window->depth, &window->renderPass)) < PURRR_SUCCESS) return result;
  if ((result = create_framebuffers(context->device, window->renderPass, window->width, window->height, window->depth, window->imageCount, window->depthImages, window->images, window->framebuffers)) < PURRR_SUCCESS) return result;

  return PURRR_SUCCESS;
}

static void destroy_swapchain(_Purrr_Window_Vulkan *window) {
  _Purrr_Renderer_Vulkan *renderer = window->renderer;
  _Purrr_Context_Vulkan *context = renderer->context;

  if (window->swapchain) vkDestroySwapchainKHR(context->device, window->swapchain, VK_NULL_HANDLE);

  if (window->renderSemaphores) {
    for (uint32_t i = 0; i < window->imageCount; ++i)
      vkDestroySemaphore(context->device, window->renderSemaphores[i], VK_NULL_HANDLE);

    free(window->renderSemaphores);
  }

  if (window->images) {
    for (uint32_t i = 0; i < window->imageCount; ++i)
      (void)_purrr_destroy_image_vulkan(window->images[i]);
    free(window->images);
  }

  if (window->depthImages) {
    for (uint32_t i = 0; i < window->imageCount; ++i)
      (void)_purrr_destroy_image_vulkan(window->depthImages[i]);
    free(window->depthImages);
  }

  if (window->renderPass) vkDestroyRenderPass(context->device, window->renderPass, VK_NULL_HANDLE);

  if (window->framebuffers) {
    for (uint32_t i = 0; i < window->imageCount; ++i)
      vkDestroyFramebuffer(context->device, window->framebuffers[i], VK_NULL_HANDLE);

    free(window->framebuffers);
  }
}