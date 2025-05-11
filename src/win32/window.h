#ifndef   __PURRR_WIN32_WINDOW_H_
#define   __PURRR_WIN32_WINDOW_H_

#include "purrr/purrr.h"

#define UNICODE
#define _UNICODE
#include <windows.h>

typedef struct _Purrr_Window_Win32 {
  HINSTANCE instance;
  HWND handle;

  int windowClassNameLength;
  wchar_t *windowClassName;
  WNDCLASS windowClass;

  int titleLength;
  wchar_t *title;

  Purrr_Key keycodes[512];
  int16_t scancodes[PURRR_KEY_LAST];
} _Purrr_Window_Win32;

Purrr_Result _purrr_create_window_win32(Purrr_Window realWindow, Purrr_Window_Create_Info createInfo, _Purrr_Window_Win32 *window);
Purrr_Result _purrr_destroy_window_win32(_Purrr_Window_Win32 *window);
Purrr_Result _purrr_get_window_size_win32(_Purrr_Window_Win32 *window, int *width, int *height);

#endif // __PURRR_WIN32_WINDOW_H_