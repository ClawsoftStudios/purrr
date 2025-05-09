#include "purrr/purrr.h"

#include "internal.h"

#include "win32/window.h"

#include "vulkan/window.h"

Purrr_Result purrr_create_window(Purrr_Renderer renderer, Purrr_Window_Create_Info createInfo, Purrr_Window *window) {
  if (!renderer || !window) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Object_Header header = _purrr_get_header(renderer);
  struct Purrr_Window *win = _purrr_malloc_with_header((_Purrr_Object_Header){
    .backend = header.backend,
    .type = _PURRR_OBJECT_WINDOW,
    .flags = _PURRR_OBJECT_FLAG_RENDER_TARGET
  }, sizeof(*win));
  if (!win) return PURRR_BUY_MORE_RAM;
  memset(win, 0, sizeof(*win));

  win->width = createInfo.width;
  win->height = createInfo.height;

  Purrr_Result result = PURRR_SUCCESS;

  #ifdef PURRR_PLATFORM_WINDOWS
  win->platformData = malloc(sizeof(_Purrr_Window_Win32));
  if (!win->platformData) {
    purrr_destroy_window(win);
    return PURRR_BUY_MORE_RAM;
  }

  memset(win->platformData, 0, sizeof(_Purrr_Window_Win32));

  if ((result = _purrr_create_window_win32(win, createInfo, (_Purrr_Window_Win32*)win->platformData)) < PURRR_SUCCESS) {
    purrr_destroy_window(win);
    return result;
  }
  #else
  #error "Unsupported"
  #endif

  switch (header.backend) {
  case PURRR_VULKAN: {
    win->context = (Purrr_Context)((_Purrr_Renderer_Vulkan*)renderer)->context;

    win->backendData = malloc(sizeof(_Purrr_Window_Vulkan));
    if (!win->backendData) {
      purrr_destroy_window(win);
      return PURRR_BUY_MORE_RAM;
    }

    memset(win->backendData, 0, sizeof(_Purrr_Window_Vulkan));

    if ((result = _purrr_create_window_vulkan((_Purrr_Renderer_Vulkan*)renderer, createInfo, win->platformData, (_Purrr_Window_Vulkan*)win->backendData, win)) < PURRR_SUCCESS) {
      purrr_destroy_window(win);
      return result;
    }
  } break;
  case COUNT_PURRR_BACKENDS:
  default: return PURRR_INTERNAL_ERROR;
  }

  *window = win;

  return PURRR_SUCCESS;
}

Purrr_Result purrr_destroy_window(Purrr_Window window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;

  Purrr_Result result = PURRR_SUCCESS;
  if (window->platformData) {
    #ifdef PURRR_PLATFORM_WINDOWS
    if ((result = _purrr_destroy_window_win32((_Purrr_Window_Win32*)window->platformData)) < PURRR_SUCCESS) return result;
    #else
    #error "Unsupported"
    #endif
  }

  if (window->backendData) {
    switch (_purrr_get_header(window).backend) {
    case PURRR_VULKAN: {
      if ((result = _purrr_destroy_window_vulkan((_Purrr_Window_Vulkan*)window->backendData)) < PURRR_SUCCESS) return result;
    } break;
    case COUNT_PURRR_BACKENDS:
    default: return PURRR_INTERNAL_ERROR;
    }
  }

  _purrr_free_with_header(window);
  return PURRR_SUCCESS;
}

Purrr_Result purrr_should_window_close(Purrr_Window window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;
  return window->shouldClose;
}

Purrr_Result purrr_get_window_size(Purrr_Window window, int *width, int *height) {
  if (!window || (!width && !height)) return PURRR_INVALID_ARGS_ERROR;
  if (width) *width = window->width;
  if (height) *height = window->height;

  return PURRR_SUCCESS;
}