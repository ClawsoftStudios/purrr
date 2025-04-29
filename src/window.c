#include "purrr/purrr.h"

#include "win32/window.h"

Purrr_Result purrr_create_window(Purrr_Window_Create_Info createInfo, Purrr_Window *window) {
  #ifdef PURRR_PLATFORM_WINDOWS
  return _purrr_create_window_win32(createInfo, (_Purrr_Window_Win32**)window);
  #else
  #error "Unsupported"
  #endif
}

Purrr_Result purrr_destroy_window(Purrr_Window window) {
  #ifdef PURRR_PLATFORM_WINDOWS
  return _purrr_destroy_window_win32((_Purrr_Window_Win32*)window);
  #else
  #error "Unsupported"
  #endif
}

Purrr_Result purrr_should_window_close(Purrr_Window window) {
  #ifdef PURRR_PLATFORM_WINDOWS
  return _purrr_should_window_close_win32((_Purrr_Window_Win32*)window);
  #else
  #error "Unsupported"
  #endif
}