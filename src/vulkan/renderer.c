#include "./renderer.h"
#include "./window.h"

#ifdef PURRR_PLATFORM_WINDOWS
#include "../win32/window.h"
#else
#error "Unsupported"
#endif

#include <stdio.h>
#include <assert.h>

static Purrr_Result create_command_pool(VkDevice device, uint32_t queueFamily, VkCommandPool *commandPool);
static Purrr_Result allocate_command_buffers(VkDevice device, VkCommandPool commandPool, uint32_t count, VkCommandBuffer *commandBuffers);
static Purrr_Result create_fence(VkDevice device, VkFence *fence);

Purrr_Result _purrr_free_renderer_vulkan_windows(struct _Purrr_Renderer_Vulkan_Windows *windows) {
  if (!windows) return PURRR_INVALID_ARGS_ERROR;

  if (!windows->windows) free(windows->windows);
  if (!windows->imageSemaphores) free(windows->imageSemaphores);
  if (!windows->renderSemaphores) free(windows->renderSemaphores);
  if (!windows->swapchains) free(windows->swapchains);
  if (!windows->imageIndices) free(windows->imageIndices);
  if (!windows->results) free(windows->results);
  if (!windows->stageFlags) free(windows->stageFlags);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_resize_renderer_vulkan_windows(struct _Purrr_Renderer_Vulkan_Windows *windows) {
  if (!windows) return PURRR_INVALID_ARGS_ERROR;

  #define x(name) \
    windows->name = realloc(windows->name, sizeof(*windows->name) * windows->capacity); \
    if (!windows->name) { \
      _purrr_free_renderer_vulkan_windows(windows); \
      return PURRR_BUY_MORE_RAM; \
    }

  x(windows)
  x(imageSemaphores)
  x(renderSemaphores)
  x(swapchains)
  x(imageIndices)
  x(results)
  x(stageFlags)

  for (uint32_t i = windows->count; i < windows->capacity; ++i)
    windows->stageFlags[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  #undef x

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_create_renderer_vulkan(_Purrr_Context_Vulkan *context, Purrr_Renderer_Create_Info createInfo, _Purrr_Renderer_Vulkan **renderer) {
  (void)createInfo;

  if (!context || !renderer) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Renderer_Vulkan *rndrr = _purrr_malloc_with_header((_Purrr_Object_Header){
    .backend = PURRR_VULKAN,
    .type = _PURRR_OBJECT_RENDERER,
    .flags = 0
  }, sizeof(*rndrr));
  if (!rndrr) return PURRR_BUY_MORE_RAM;
  memset(rndrr, 0, sizeof(*rndrr));

  rndrr->context = context;

  Purrr_Result result = PURRR_SUCCESS;
  if ((result = create_command_pool(context->device, context->queueFamily, &rndrr->commandPool)) < PURRR_SUCCESS) return result;
  if ((result = allocate_command_buffers(context->device, rndrr->commandPool, 1, &rndrr->commandBuffer)) < PURRR_SUCCESS) return result;
  if ((result = create_fence(context->device, &rndrr->fence)) < PURRR_SUCCESS) return result;

  *renderer = rndrr;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer) {
  if (!renderer) return PURRR_INVALID_ARGS_ERROR;
  if (!renderer->context) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Context_Vulkan *context = renderer->context;

  vkDestroyFence(context->device, renderer->fence, VK_NULL_HANDLE);

  vkFreeCommandBuffers(context->device, renderer->commandPool, 1, &renderer->commandBuffer);
  vkDestroyCommandPool(context->device, renderer->commandPool, VK_NULL_HANDLE);

  for (uint32_t i = 0; i < renderer->windows.count; ++i) {
    vkDestroySemaphore(context->device, renderer->windows.imageSemaphores[i], VK_NULL_HANDLE);
  }

  _purrr_free_renderer_vulkan_windows(&renderer->windows);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_wait_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer) {
  if (!renderer) return PURRR_INVALID_ARGS_ERROR;

  if (vkWaitForFences(renderer->context->device, 1, &renderer->fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_begin_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer) {
  if (!renderer) return PURRR_INVALID_ARGS_ERROR;

  VkResult vkResult = VK_SUCCESS;
  Purrr_Result result = PURRR_SUCCESS;

  for (uint32_t i = renderer->windows.activeCount; i < renderer->windows.count; ++i) {
    Purrr_Window realWindow = renderer->windows.windows[i];
    _Purrr_Window_Vulkan *window = realWindow->backendData;
    if ((result = _purrr_recreate_window_swapchain(realWindow, window)) < PURRR_SUCCESS) return result;
    else if (result == PURRR_TRUE) {
      if ((result = _purrr_swap_renderer_windows(renderer, i, renderer->windows.activeCount++)) < PURRR_SUCCESS) return result;
    }
  }

  for (uint32_t i = 0; i < renderer->windows.activeCount;) {
    Purrr_Window realWindow = renderer->windows.windows[i];
    _Purrr_Window_Vulkan *window = realWindow->backendData;

    Purrr_Result continueResult = PURRR_SUCCESS;
    for (uint32_t j = 0; j < 2 && continueResult == PURRR_SUCCESS; ++j) {
      if (realWindow->width && realWindow->height) {
        vkResult = vkAcquireNextImageKHR(
          renderer->context->device,
          window->swapchain,
          UINT64_MAX,
          renderer->windows.imageSemaphores[i],
          VK_NULL_HANDLE,
          &renderer->windows.imageIndices[i]
        );
      } else vkResult = VK_ERROR_OUT_OF_DATE_KHR;

      if (vkResult == VK_ERROR_OUT_OF_DATE_KHR) {
        if ((result = _purrr_recreate_window_swapchain(realWindow, window)) < PURRR_SUCCESS) return result;
        if ((continueResult = result) == PURRR_MINIMIZED) {
          if ((result = _purrr_swap_renderer_windows(renderer, i, renderer->windows.activeCount-1)) < PURRR_SUCCESS) return result;
          --renderer->windows.activeCount;
          // if (i == renderer->windows.activeCount) continueResult = false;
        }
      } else if (vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) {
        continueResult = PURRR_TRUE;
      } else return PURRR_INTERNAL_ERROR;
    }

    if (continueResult == PURRR_TRUE) {
      renderer->windows.renderSemaphores[i] = window->renderSemaphores[renderer->windows.imageIndices[i]];
      ++i;
    }
  }

  if (!renderer->windows.activeCount) return PURRR_INACTIVE;

  if (vkResetFences(renderer->context->device, 1, &renderer->fence) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  if (vkResetCommandBuffer(renderer->commandBuffer, 0) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkCommandBufferBeginInfo beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .pInheritanceInfo = VK_NULL_HANDLE
  };

  return (vkBeginCommandBuffer(renderer->commandBuffer, &beginInfo) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}

Purrr_Result _purrr_renderer_begin_vulkan(_Purrr_Renderer_Vulkan *renderer, Purrr_Handle renderTarget, Purrr_Color color) {
  if (!renderer) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Object_Header header = _purrr_get_header(renderTarget);
  if (header.backend != PURRR_VULKAN || !(header.flags & _PURRR_OBJECT_FLAG_RENDER_TARGET)) return PURRR_INVALID_ARGS_ERROR;

  switch (header.type) {
  case _PURRR_OBJECT_WINDOW: {
    Purrr_Window realWindow = (Purrr_Window)renderTarget;
    _Purrr_Window_Vulkan *window = (_Purrr_Window_Vulkan*)realWindow->backendData;
    if (window->renderer != renderer) return PURRR_INVALID_ARGS_ERROR;

    if (window->index >= renderer->windows.activeCount) return PURRR_SUCCESS;
    assert(!window->depth); // TODO: Implement

    VkClearValue clearValue = {{{color.r, color.g, color.b, color.a}}};

    VkRenderPassBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = VK_NULL_HANDLE,
      .renderPass = window->renderPass,
      .framebuffer = window->framebuffers[renderer->windows.imageIndices[window->index]],
      .renderArea = {
        .extent = {
          .width = window->width,
          .height = window->height
        }
      },
      .clearValueCount = 1,
      .pClearValues = &clearValue
    };

    vkCmdBeginRenderPass(renderer->commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
      .x = 0,
      .y = 0,
      .width = window->width,
      .height = window->height,
      .minDepth = 0.0f,
      .maxDepth = 0.0f
    };

    VkRect2D scissor = {
      .extent = {
        .width = window->width,
        .height = window->height
      },
      .offset = {0}
    };

    vkCmdSetViewport(renderer->commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(renderer->commandBuffer, 0, 1, &scissor);
  } break;
  case _PURRR_OBJECT_RENDER_TARGET: {
    assert(0 && "Unimplemented");
  } break;
  case _PURRR_OBJECT_CONTEXT:
  case _PURRR_OBJECT_RENDERER:
  case _PURRR_OBJECT_IMAGE:
  case _PURRR_OBJECT_PROGRAM:
  case _COUNT_PURRR_OBJECT_TYPES:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  return PURRR_TRUE;
}

Purrr_Result _purrr_renderer_bind_buffer_vulkan(_Purrr_Renderer_Vulkan *renderer, _Purrr_Buffer_Vulkan *buffer, uint32_t index) {
  if (!renderer || !buffer || renderer->context != buffer->context) return PURRR_INVALID_ARGS_ERROR;

  switch (buffer->type) {
  case PURRR_BUFFER_VERTEX: {
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderer->commandBuffer, index, 1, &buffer->buffer, &offset);
  } break;
  case PURRR_BUFFER_INDEX: {
    vkCmdBindIndexBuffer(renderer->commandBuffer, buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
  } break;
  case PURRR_BUFFER_UNIFORM: {
    if (!renderer->program || !buffer->descriptorSet) return PURRR_INVALID_ARGS_ERROR;
    vkCmdBindDescriptorSets(renderer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->program->layout, index, 1, &buffer->descriptorSet, 0, VK_NULL_HANDLE);
  } break;
  case COUNT_PURRR_BUFFER_TYPES:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_renderer_bind_image_vulkan(_Purrr_Renderer_Vulkan *renderer, _Purrr_Image_Vulkan *image, uint32_t index) {
  if (!renderer || !renderer->program || !image || renderer->context != image->context || !image->descriptorSet) return PURRR_INVALID_ARGS_ERROR;

  vkCmdBindDescriptorSets(renderer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->program->layout, index, 1, &image->descriptorSet, 0, VK_NULL_HANDLE);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_renderer_bind_program_vulkan(_Purrr_Renderer_Vulkan *renderer, _Purrr_Program_Vulkan *program) {
  if (!renderer || !program) return PURRR_INVALID_ARGS_ERROR;

  vkCmdBindPipeline(renderer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (renderer->program = program)->pipeline);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_renderer_draw_indexed_vulkan(_Purrr_Renderer_Vulkan *renderer, uint32_t indexCount) {
  if (!renderer || !indexCount) return PURRR_INVALID_ARGS_ERROR;

  vkCmdDrawIndexed(renderer->commandBuffer, indexCount, 1, 0, 0, 0);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_renderer_end_vulkan(_Purrr_Renderer_Vulkan *renderer) {
  if (!renderer) return PURRR_INVALID_ARGS_ERROR;

  vkCmdEndRenderPass(renderer->commandBuffer);

  renderer->program = PURRR_NULL_HANDLE;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_render_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer) {
  if (!renderer) return PURRR_INVALID_ARGS_ERROR;

  if (vkEndCommandBuffer(renderer->commandBuffer) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = VK_NULL_HANDLE,
    .waitSemaphoreCount = renderer->windows.activeCount,
    .pWaitSemaphores = renderer->windows.imageSemaphores,
    .pWaitDstStageMask = renderer->windows.stageFlags,
    .commandBufferCount = 1,
    .pCommandBuffers = &renderer->commandBuffer,
    .signalSemaphoreCount = renderer->windows.activeCount,
    .pSignalSemaphores = renderer->windows.renderSemaphores
  };

  if (vkQueueSubmit(renderer->context->queue, 1, &submitInfo, renderer->fence) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkPresentInfoKHR presentInfo = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = VK_NULL_HANDLE,
    .waitSemaphoreCount = renderer->windows.activeCount,
    .pWaitSemaphores = renderer->windows.renderSemaphores,
    .swapchainCount = renderer->windows.activeCount,
    .pSwapchains = renderer->windows.swapchains,
    .pImageIndices = renderer->windows.imageIndices,
    .pResults = renderer->windows.results
  };

  VkResult vkResult = vkQueuePresentKHR(renderer->context->queue, &presentInfo);
  if (vkResult != VK_SUCCESS && vkResult != VK_SUBOPTIMAL_KHR && vkResult != VK_ERROR_OUT_OF_DATE_KHR) return PURRR_INTERNAL_ERROR;

  Purrr_Result result = PURRR_SUCCESS;
  for (uint32_t i = 0; i < renderer->windows.activeCount; ++i) {
    if (renderer->windows.results[i] == VK_ERROR_OUT_OF_DATE_KHR || renderer->windows.results[i] == VK_SUBOPTIMAL_KHR) {
      Purrr_Window realWindow = renderer->windows.windows[i];
      _Purrr_Window_Vulkan *window = realWindow->backendData;
      if ((result = _purrr_recreate_window_swapchain(realWindow, window)) < PURRR_SUCCESS) return result;
    }
  }

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_add_renderer_window(_Purrr_Renderer_Vulkan *renderer, Purrr_Window realWindow, _Purrr_Window_Vulkan *window, uint32_t *index) {
  if (!renderer || !window || !index) return PURRR_INVALID_ARGS_ERROR;

  if (renderer->windows.count >= renderer->windows.capacity) {
    if (renderer->windows.capacity) renderer->windows.capacity <<= 1;
    else renderer->windows.capacity = 4;

    Purrr_Result result = PURRR_SUCCESS;
    if ((result = _purrr_resize_renderer_vulkan_windows(&renderer->windows)) < PURRR_SUCCESS) return result;
  }

  renderer->windows.windows[renderer->windows.count] = realWindow;

  VkSemaphoreCreateInfo semaphoreCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0
  };

  if (realWindow->width && realWindow->height) ++renderer->windows.activeCount;

  if (vkCreateSemaphore(renderer->context->device, &semaphoreCreateInfo, VK_NULL_HANDLE, &renderer->windows.imageSemaphores[renderer->windows.count]) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  renderer->windows.swapchains[*index = renderer->windows.count++] = window->swapchain;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_remove_renderer_window(_Purrr_Renderer_Vulkan *renderer, uint32_t index) {
  if (!renderer || renderer->windows.count < index) return PURRR_INVALID_ARGS_ERROR;

  vkDeviceWaitIdle(renderer->context->device);

  _Purrr_Context_Vulkan *context = renderer->context;
  vkDestroySemaphore(context->device, renderer->windows.imageSemaphores[index], VK_NULL_HANDLE);

  --renderer->windows.count;
  if (index < renderer->windows.activeCount) --renderer->windows.activeCount;
  Purrr_Window replacementWindow = renderer->windows.windows[index] = renderer->windows.windows[renderer->windows.count];
  renderer->windows.imageSemaphores[index] = renderer->windows.imageSemaphores[renderer->windows.count];
  renderer->windows.swapchains[index] = renderer->windows.swapchains[renderer->windows.count];
  renderer->windows.imageIndices[index] = renderer->windows.imageIndices[renderer->windows.count];

  ((_Purrr_Window_Vulkan*)replacementWindow->backendData)->index = index;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_swap_renderer_windows(_Purrr_Renderer_Vulkan *renderer, uint32_t a, uint32_t b) {
  if (!renderer || renderer->windows.count < a || renderer->windows.count < b) return PURRR_INVALID_ARGS_ERROR;
  if (a == b) return PURRR_SUCCESS;

  Purrr_Window window = renderer->windows.windows[a];
  VkSemaphore imageSemaphore = renderer->windows.imageSemaphores[a];
  VkSemaphore renderSemaphore = renderer->windows.renderSemaphores[a];
  VkSwapchainKHR swapchain = renderer->windows.swapchains[a];
  uint32_t imageIndice = renderer->windows.imageIndices[a];

  uint32_t index = ((_Purrr_Window_Vulkan*)renderer->windows.windows[a]->backendData)->index;
  ((_Purrr_Window_Vulkan*)renderer->windows.windows[a]->backendData)->index = ((_Purrr_Window_Vulkan*)renderer->windows.windows[b]->backendData)->index;
  ((_Purrr_Window_Vulkan*)renderer->windows.windows[b]->backendData)->index = index;

  renderer->windows.windows[a] = renderer->windows.windows[b];
  renderer->windows.imageSemaphores[a] = renderer->windows.imageSemaphores[b];
  renderer->windows.renderSemaphores[a] = renderer->windows.renderSemaphores[b];
  renderer->windows.swapchains[a] = renderer->windows.swapchains[b];
  renderer->windows.imageIndices[a] = renderer->windows.imageIndices[b];

  renderer->windows.windows[b] = window;
  renderer->windows.imageSemaphores[b] = imageSemaphore;
  renderer->windows.renderSemaphores[b] = renderSemaphore;
  renderer->windows.swapchains[b] = swapchain;
  renderer->windows.imageIndices[b] = imageIndice;

  return PURRR_SUCCESS;
}



static Purrr_Result create_command_pool(VkDevice device, uint32_t queueFamily, VkCommandPool *commandPool) {
  VkCommandPoolCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = queueFamily
  };

  return (vkCreateCommandPool(device, &createInfo, VK_NULL_HANDLE, commandPool) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}

static Purrr_Result allocate_command_buffers(VkDevice device, VkCommandPool commandPool, uint32_t count, VkCommandBuffer *commandBuffers) {
  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .commandPool = commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = count,
  };

  return (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}

static Purrr_Result create_fence(VkDevice device, VkFence *fence) {
  if (!device || !fence) return PURRR_INVALID_ARGS_ERROR;

  VkFenceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };

  return (vkCreateFence(device, &createInfo, VK_NULL_HANDLE, fence) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}