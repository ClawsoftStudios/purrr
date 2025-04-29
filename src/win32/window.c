#ifdef PURRR_PLATFORM_WINDOWS

#include "window.h"

#define UNICODE
#define _UNICODE
#include <windows.h>

#include <stdlib.h>
#include <stdbool.h>

struct _Purrr_Window_Win32 {
  HINSTANCE instance;

  int windowClassNameLength;
  wchar_t *windowClassName;
  WNDCLASS windowClass;

  int titleLength;
  wchar_t *title;

  HWND handle;

  bool shouldClose;
};

static wchar_t *cstr_to_wstr(const char *cstr, int *length) {
  if (!length || !cstr) return NULL;

  int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cstr, -1, NULL, 0);
  if (len == 0) return NULL;

  wchar_t *wstr = malloc(sizeof(wchar_t) * len);
  if (!wstr) return NULL;

  if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cstr, -1, wstr, len) == 0) {
    free(wstr);
    return NULL;
  }

  *length = len - 1;
  return wstr;
}

static LRESULT window_procedure(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_CREATE) {
    CREATESTRUCT *createStruct = (CREATESTRUCT*)lParam;
    SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
    ShowWindow(handle, SW_SHOW);
    return 0;
  }

  _Purrr_Window_Win32 *window = (_Purrr_Window_Win32*)GetWindowLongPtr(handle, GWLP_USERDATA);
  if (!window) return DefWindowProc(handle, msg, wParam, lParam);

  switch (msg) {
  case WM_CLOSE: {
    window->shouldClose = true;
  } break;
  default: return DefWindowProc(handle, msg, wParam, lParam);
  }

  return 0;
}

Purrr_Result _purrr_create_window_win32(Purrr_Window_Create_Info createInfo, _Purrr_Window_Win32 **window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Window_Win32 *win = malloc(sizeof(*win));
  if (!win) return PURRR_BUY_MORE_RAM;
  memset(win, 0, sizeof(*win));

  win->windowClassName = cstr_to_wstr(createInfo.title, &win->windowClassNameLength);
  if (!win->windowClassName) {
    _purrr_destroy_window_win32(win);
    return PURRR_INTERNAL_ERROR;
  }

  win->title = cstr_to_wstr(createInfo.title, &win->titleLength);
  if (!win->title) {
    _purrr_destroy_window_win32(win);
    return PURRR_INTERNAL_ERROR;
  }

  HINSTANCE instance  = GetModuleHandle(NULL);

  win->windowClass = (WNDCLASS){
    .lpfnWndProc   = window_procedure,
    .hInstance     = win->instance,
    .lpszClassName = win->windowClassName,
    .hCursor       = LoadCursor(NULL, IDC_ARROW),
  };

  if (!RegisterClass(&win->windowClass)) {
    _purrr_destroy_window_win32(win);
    return PURRR_INTERNAL_ERROR;
  }

  win->instance = instance;

  win->handle = CreateWindowEx(
    0,
    win->windowClassName,
    win->title,
    WS_OVERLAPPEDWINDOW,

    CW_USEDEFAULT, CW_USEDEFAULT, createInfo.width, createInfo.height,

    NULL, NULL, win->instance, win
  );

  if (!win->handle) {
    _purrr_destroy_window_win32(win);
    return PURRR_INTERNAL_ERROR;
  }

  *window = win;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_window_win32(_Purrr_Window_Win32 *window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;

  if (window->windowClassName) free(window->windowClassName);
  if (window->title) free(window->title);

  if (window->handle) DestroyWindow(window->handle);
  if (window->instance) UnregisterClass(window->windowClassName, window->instance);

  free(window);

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_should_window_close_win32(_Purrr_Window_Win32 *window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;
  return window->shouldClose;
}

void purrr_poll_windows() {
  MSG msg = {0};
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

#endif // PURRR_PLATFORM_WINDOWS