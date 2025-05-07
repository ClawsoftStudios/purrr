#ifdef PURRR_PLATFORM_WINDOWS

#include "window.h"
#include "../internal.h"

#include "../vulkan/window.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

static wchar_t *cstr_to_wstr(const char *cstr, int *length) {
  if (!length || !cstr) return NULL;

  int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cstr, -1, NULL, 0);
  if (len == 0) return NULL;

  wchar_t *wstr = malloc(sizeof(*wstr) * len);
  if (!wstr) return NULL;

  if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cstr, -1, wstr, len) == 0) {
    free(wstr);
    return NULL;
  }

  return (*length = len - 1, wstr);
}

static LRESULT window_procedure(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_CREATE) {
    CREATESTRUCT *createStruct = (CREATESTRUCT*)lParam;
    SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
    ShowWindow(handle, SW_SHOW);
    return 0;
  }

  Purrr_Window window = (Purrr_Window)GetWindowLongPtr(handle, GWLP_USERDATA);
  if (!window) return DefWindowProc(handle, msg, wParam, lParam);
  if (_purrr_get_header(window).type != _PURRR_OBJECT_WINDOW) return DefWindowProc(handle, msg, wParam, lParam);

  switch (msg) {
  case WM_CLOSE: {
    window->shouldClose = true;
  } break;
  case WM_SIZE: {
    window->width = LOWORD(lParam);
    window->height = HIWORD(lParam);
  } break;
  default: return DefWindowProc(handle, msg, wParam, lParam);
  }

  return 0;
}

Purrr_Result _purrr_create_window_win32(Purrr_Window realWindow, Purrr_Window_Create_Info createInfo, _Purrr_Window_Win32 *window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;

  window->windowClassName = cstr_to_wstr(createInfo.title, &window->windowClassNameLength);
  if (!window->windowClassName) {
    _purrr_destroy_window_win32(window);
    return PURRR_INTERNAL_ERROR;
  }

  window->title = cstr_to_wstr(createInfo.title, &window->titleLength);
  if (!window->title) {
    _purrr_destroy_window_win32(window);
    return PURRR_INTERNAL_ERROR;
  }

  HINSTANCE instance = GetModuleHandle(NULL);

  window->windowClass = (WNDCLASS){
    .lpfnWndProc   = window_procedure,
    .hInstance     = instance,
    .lpszClassName = window->windowClassName,
    .hCursor       = LoadCursor(NULL, IDC_ARROW),
  };

  if (!RegisterClass(&window->windowClass)) {
    _purrr_destroy_window_win32(window);
    return PURRR_INTERNAL_ERROR;
  }

  window->instance = instance;

  window->handle = CreateWindowEx(
    0,
    window->windowClassName,
    window->title,
    WS_OVERLAPPEDWINDOW,

    CW_USEDEFAULT, CW_USEDEFAULT, createInfo.width, createInfo.height,

    NULL, NULL, window->instance, realWindow
  );

  if (!window->handle) {
    _purrr_destroy_window_win32(window);
    return PURRR_INTERNAL_ERROR;
  }

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_window_win32(_Purrr_Window_Win32 *window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;

  if (window->handle) DestroyWindow(window->handle);
  if (window->instance) UnregisterClass(window->windowClassName, window->instance);

  if (window->windowClassName) free(window->windowClassName);
  if (window->title) free(window->title);

  free(window);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_get_window_size_win32(_Purrr_Window_Win32 *window, int *width, int *height) {
  if (!window || (!width && !height)) return PURRR_INVALID_ARGS_ERROR;

  RECT rect = {0};
  if (!GetWindowRect(window->handle, &rect)) return PURRR_INTERNAL_ERROR;

  if (width) *width = rect.right - rect.left;
  if (height) *height = rect.bottom - rect.top;

  return PURRR_SUCCESS;
}

void purrr_poll_windows() {
  MSG msg = {0};
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void purrr_wait_windows() {
  WaitMessage();
  purrr_poll_windows();
}



Purrr_Result _purrr_create_window_surface(_Purrr_Context_Vulkan *context, void *win, _Purrr_Window_Vulkan *vulkanWindow) {
  if (!context || !win || !vulkanWindow) return PURRR_INVALID_ARGS_ERROR;
  _Purrr_Window_Win32 *window = (_Purrr_Window_Win32*)win;

  VkWin32SurfaceCreateInfoKHR createInfo = {
    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .hinstance = window->instance,
    .hwnd = window->handle
  };

  return (vkCreateWin32SurfaceKHR(context->instance, &createInfo, VK_NULL_HANDLE, &vulkanWindow->surface) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}

#endif // PURRR_PLATFORM_WINDOWS