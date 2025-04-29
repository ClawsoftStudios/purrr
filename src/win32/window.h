#ifndef   __PURRR_WIN32_WINDOW_H_
#define   __PURRR_WIN32_WINDOW_H_

#include "purrr/purrr.h"

typedef struct _Purrr_Window_Win32 _Purrr_Window_Win32;

Purrr_Result _purrr_create_window_win32(Purrr_Window_Create_Info createInfo, _Purrr_Window_Win32 **window);
Purrr_Result _purrr_destroy_window_win32(_Purrr_Window_Win32 *window);
Purrr_Result _purrr_should_window_close_win32(_Purrr_Window_Win32 *window);

#endif // __PURRR_WIN32_WINDOW_H_