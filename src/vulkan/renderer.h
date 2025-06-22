#ifndef   __PURRR_VULKAN_RENDERER_H_
#define   __PURRR_VULKAN_RENDERER_H_

#include "./vulkan.h"
#include "./context.h"
#include "./buffer.h"
#include "./image.h"
#include "./program.h"

#include "../internal.h"

typedef struct _Purrr_Window_Vulkan _Purrr_Window_Vulkan;
typedef struct _Purrr_Renderer_Vulkan {
  _Purrr_Context_Vulkan *context;

  _Purrr_Program_Vulkan *program; // Currently bound program

  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;

  VkFence fence;
  struct _Purrr_Renderer_Vulkan_Windows {
    Purrr_Window *windows;
    VkSemaphore *imageSemaphores;
    VkSemaphore *renderSemaphores;
    VkSwapchainKHR *swapchains;
    uint32_t *imageIndices; // for VkPresentInfoKHR
    VkResult *results;
    VkPipelineStageFlags *stageFlags;

    uint32_t capacity;
    uint32_t count;
    uint32_t activeCount;
  } windows;
} _Purrr_Renderer_Vulkan;

Purrr_Result _purrr_free_renderer_vulkan_windows(struct _Purrr_Renderer_Vulkan_Windows *windows);
Purrr_Result _purrr_resize_renderer_vulkan_windows(struct _Purrr_Renderer_Vulkan_Windows *windows);

Purrr_Result _purrr_create_renderer_vulkan(_Purrr_Context_Vulkan *context, Purrr_Renderer_Create_Info createInfo, _Purrr_Renderer_Vulkan **renderer);
Purrr_Result _purrr_destroy_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_wait_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_begin_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_renderer_begin_vulkan(_Purrr_Renderer_Vulkan *renderer, Purrr_Handle renderTarget, Purrr_Color color);
Purrr_Result _purrr_renderer_bind_buffer_vulkan(_Purrr_Renderer_Vulkan *renderer, _Purrr_Buffer_Vulkan *buffer, uint32_t index);
Purrr_Result _purrr_renderer_bind_image_vulkan(_Purrr_Renderer_Vulkan *renderer, _Purrr_Image_Vulkan *image, uint32_t index);
Purrr_Result _purrr_renderer_bind_program_vulkan(_Purrr_Renderer_Vulkan *renderer, _Purrr_Program_Vulkan *program);
Purrr_Result _purrr_renderer_draw_indexed_vulkan(_Purrr_Renderer_Vulkan *renderer, uint32_t indexCount, uint32_t instanceCount);
Purrr_Result _purrr_renderer_draw_indexed_indirect_vulkan(_Purrr_Renderer_Vulkan *renderer, _Purrr_Buffer_Vulkan *buffer, uint32_t drawCount, uint32_t stride);
Purrr_Result _purrr_renderer_end_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_render_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);

Purrr_Result _purrr_add_renderer_window(_Purrr_Renderer_Vulkan *renderer, Purrr_Window realWindow, _Purrr_Window_Vulkan *window, uint32_t *index);
Purrr_Result _purrr_remove_renderer_window(_Purrr_Renderer_Vulkan *renderer, uint32_t index);
Purrr_Result _purrr_swap_renderer_windows(_Purrr_Renderer_Vulkan *renderer, uint32_t a, uint32_t b);

#endif // __PURRR_VULKAN_RENDERER_H_