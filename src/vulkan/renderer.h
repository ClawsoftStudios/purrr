#ifndef   __PURRR_VULKAN_RENDERER_H_
#define   __PURRR_VULKAN_RENDERER_H_

#include "./vulkan.h"
#include "./context.h"

#include "../internal.h"

typedef struct _Purrr_Window_Vulkan _Purrr_Window_Vulkan;
typedef struct _Purrr_Renderer_Vulkan {
  _Purrr_Context_Vulkan *context;

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

    size_t capacity;
    size_t count;
    size_t activeCount;
  } windows;
} _Purrr_Renderer_Vulkan;

Purrr_Result _purrr_free_renderer_vulkan_windows(struct _Purrr_Renderer_Vulkan_Windows *windows);
Purrr_Result _purrr_resize_renderer_vulkan_windows(struct _Purrr_Renderer_Vulkan_Windows *windows);

Purrr_Result _purrr_create_renderer_vulkan(_Purrr_Context_Vulkan *context, Purrr_Renderer_Create_Info createInfo, _Purrr_Renderer_Vulkan **renderer);
Purrr_Result _purrr_destroy_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_wait_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_begin_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_renderer_begin_vulkan(_Purrr_Renderer_Vulkan *renderer, Purrr_Handle renderTarget, Purrr_Color color);
Purrr_Result _purrr_renderer_end_vulkan(_Purrr_Renderer_Vulkan *renderer);
Purrr_Result _purrr_render_renderer_vulkan(_Purrr_Renderer_Vulkan *renderer);

Purrr_Result _purrr_add_renderer_window(_Purrr_Renderer_Vulkan *renderer, Purrr_Window realWindow, _Purrr_Window_Vulkan *window, uint32_t *index);
Purrr_Result _purrr_remove_renderer_window(_Purrr_Renderer_Vulkan *renderer, uint32_t index);
Purrr_Result _purrr_swap_renderer_windows(_Purrr_Renderer_Vulkan *renderer, uint32_t a, uint32_t b);

#endif // __PURRR_VULKAN_RENDERER_H_