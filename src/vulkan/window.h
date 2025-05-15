#ifndef   __PURRR_VULKAN_WINDOW_H_
#define   __PURRR_VULKAN_WINDOW_H_

#include "./vulkan.h"
#include "./renderer.h"

struct _Purrr_Window_Vulkan {
  _Purrr_Renderer_Vulkan *renderer;
  uint32_t index;

  bool depth;

  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;

  int fullWidth, fullHeight;
  uint32_t width, height;

  VkFormat format;
  VkFormat depthFormat;

  uint32_t imageCount;
  VkImage *images;
  VkImageView *imageViews;

  _Purrr_Image_Vulkan **depthImages;

  VkRenderPass renderPass;
  VkFramebuffer *framebuffers;

  VkSemaphore *renderSemaphores;
};

Purrr_Result _purrr_create_window_surface(_Purrr_Context_Vulkan *context, void *win, _Purrr_Window_Vulkan *vulkanWindow);

Purrr_Result _purrr_create_window_vulkan(_Purrr_Renderer_Vulkan *renderer, Purrr_Window_Create_Info createInfo, void *platform, _Purrr_Window_Vulkan *window, Purrr_Window realWindow);
Purrr_Result _purrr_destroy_window_vulkan(_Purrr_Window_Vulkan *window);



Purrr_Result _purrr_recreate_window_swapchain(Purrr_Window realWindow, _Purrr_Window_Vulkan *vulkanWindow);

#endif // __PURRR_VULKAN_WINDOW_H_