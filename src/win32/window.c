#ifdef PURRR_PLATFORM_WINDOWS

#include "window.h"
#include "../internal.h"

#include "../vulkan/window.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

static void create_key_tables(_Purrr_Window_Win32 *window);
static Purrr_Key_Modifiers get_modifiers();

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

  Purrr_Window realWindow = (Purrr_Window)GetWindowLongPtr(handle, GWLP_USERDATA);
  if (!realWindow) return DefWindowProc(handle, msg, wParam, lParam);
  if (_purrr_get_header(realWindow).type != _PURRR_OBJECT_WINDOW) return DefWindowProc(handle, msg, wParam, lParam);

  _Purrr_Window_Win32 *window = realWindow->platformData;

  switch (msg) {
  case WM_CLOSE: {
    realWindow->shouldClose = true;
  } break;
  case WM_SIZE: {
    realWindow->width = LOWORD(lParam);
    realWindow->height = HIWORD(lParam);
  } break;
  case WM_KEYDOWN:
  case WM_KEYUP: {
    WORD scancode = (HIWORD(lParam) & 0x1FF);
    if (!scancode) scancode = (WORD)MapVirtualKeyW((UINT)wParam, MAPVK_VK_TO_VSC);
    if (scancode == 0x54) scancode = 0x137;
    if (scancode == 0x146) scancode = 0x45;
    if (scancode == 0x136) scancode = 0x36;

    Purrr_Key key = window->keycodes[scancode];
    if (wParam == VK_CONTROL) { // Taken from https://github.com/glfw/glfw/blob/master/src/win32_window.c#L739
      if (HIWORD(lParam) & KF_EXTENDED) key = PURRR_KEY_RIGHT_CONTROL;
      else {
        MSG next = {0};
        const DWORD time = GetMessageTime();

        if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE)) {
          if (next.message == WM_KEYDOWN ||
              next.message == WM_SYSKEYDOWN ||
              next.message == WM_KEYUP ||
              next.message == WM_SYSKEYUP) {
            if (next.wParam == VK_MENU &&
                (HIWORD(next.lParam) & KF_EXTENDED) &&
                next.time == time)
              return DefWindowProc(handle, msg, wParam, lParam);
          }
        }

        key = PURRR_KEY_LEFT_CONTROL;
      }
    } else if (wParam == VK_PROCESSKEY) return DefWindowProc(handle, msg, wParam, lParam);

    Purrr_Key_Modifiers modifiers = get_modifiers();

    bool up = (HIWORD(lParam) & KF_UP);
    if (up && wParam == VK_SHIFT) {
      _purrr_set_window_key(realWindow, scancode, PURRR_KEY_LEFT_SHIFT, false, modifiers);
      _purrr_set_window_key(realWindow, scancode, PURRR_KEY_RIGHT_SHIFT, false, modifiers);
    } else if (wParam != VK_SNAPSHOT) _purrr_set_window_key(realWindow, scancode, key, !up, modifiers);

    else return DefWindowProc(handle, msg, wParam, lParam);
  } break;
  default: return DefWindowProc(handle, msg, wParam, lParam);
  }

  return 0;
}

Purrr_Result _purrr_create_window_win32(Purrr_Window realWindow, Purrr_Window_Create_Info createInfo, _Purrr_Window_Win32 *window) {
  if (!window) return PURRR_INVALID_ARGS_ERROR;

  create_key_tables(window);

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

static uint64_t sTimerFrequency = 0;

double purrr_get_windows_time() { // Based on https://github.com/glfw/glfw/blob/master/src/win32_time.c
  if (!sTimerFrequency) QueryPerformanceFrequency((LARGE_INTEGER*)&sTimerFrequency);

  uint64_t value = 0;
  QueryPerformanceCounter((LARGE_INTEGER*)&value);
  return ((double)value / (double)sTimerFrequency);
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

// I tried to search for a table of scan codes, but unfortunately, I couldn't find one.
// I thought that https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#scan-codes
// would be correct but it for some reason wasn't. So I stole (and modified) it from
// https://github.com/glfw/glfw/blob/master/src/win32_init.c#L198 :3
static void create_key_tables(_Purrr_Window_Win32 *window) {
  assert(window);

  memset(window->keycodes, -1, sizeof(window->keycodes));
  memset(window->scancodes, -1, sizeof(window->scancodes));

  window->keycodes[0x00B] = PURRR_KEY_0;
  window->keycodes[0x002] = PURRR_KEY_1;
  window->keycodes[0x003] = PURRR_KEY_2;
  window->keycodes[0x004] = PURRR_KEY_3;
  window->keycodes[0x005] = PURRR_KEY_4;
  window->keycodes[0x006] = PURRR_KEY_5;
  window->keycodes[0x007] = PURRR_KEY_6;
  window->keycodes[0x008] = PURRR_KEY_7;
  window->keycodes[0x009] = PURRR_KEY_8;
  window->keycodes[0x00A] = PURRR_KEY_9;
  window->keycodes[0x01E] = PURRR_KEY_A;
  window->keycodes[0x030] = PURRR_KEY_B;
  window->keycodes[0x02E] = PURRR_KEY_C;
  window->keycodes[0x020] = PURRR_KEY_D;
  window->keycodes[0x012] = PURRR_KEY_E;
  window->keycodes[0x021] = PURRR_KEY_F;
  window->keycodes[0x022] = PURRR_KEY_G;
  window->keycodes[0x023] = PURRR_KEY_H;
  window->keycodes[0x017] = PURRR_KEY_I;
  window->keycodes[0x024] = PURRR_KEY_J;
  window->keycodes[0x025] = PURRR_KEY_K;
  window->keycodes[0x026] = PURRR_KEY_L;
  window->keycodes[0x032] = PURRR_KEY_M;
  window->keycodes[0x031] = PURRR_KEY_N;
  window->keycodes[0x018] = PURRR_KEY_O;
  window->keycodes[0x019] = PURRR_KEY_P;
  window->keycodes[0x010] = PURRR_KEY_Q;
  window->keycodes[0x013] = PURRR_KEY_R;
  window->keycodes[0x01F] = PURRR_KEY_S;
  window->keycodes[0x014] = PURRR_KEY_T;
  window->keycodes[0x016] = PURRR_KEY_U;
  window->keycodes[0x02F] = PURRR_KEY_V;
  window->keycodes[0x011] = PURRR_KEY_W;
  window->keycodes[0x02D] = PURRR_KEY_X;
  window->keycodes[0x015] = PURRR_KEY_Y;
  window->keycodes[0x02C] = PURRR_KEY_Z;

  window->keycodes[0x028] = PURRR_KEY_APOSTROPHE;
  window->keycodes[0x02B] = PURRR_KEY_BACKSLASH;
  window->keycodes[0x033] = PURRR_KEY_COMMA;
  window->keycodes[0x00D] = PURRR_KEY_EQUAL;
  window->keycodes[0x029] = PURRR_KEY_GRAVE_ACCENT;
  window->keycodes[0x01A] = PURRR_KEY_LEFT_BRACKET;
  window->keycodes[0x00C] = PURRR_KEY_MINUS;
  window->keycodes[0x034] = PURRR_KEY_PERIOD;
  window->keycodes[0x01B] = PURRR_KEY_RIGHT_BRACKET;
  window->keycodes[0x027] = PURRR_KEY_SEMICOLON;
  window->keycodes[0x035] = PURRR_KEY_SLASH;
  window->keycodes[0x056] = PURRR_KEY_WORLD_2;

  window->keycodes[0x00E] = PURRR_KEY_BACKSPACE;
  window->keycodes[0x153] = PURRR_KEY_DELETE;
  window->keycodes[0x14F] = PURRR_KEY_END;
  window->keycodes[0x01C] = PURRR_KEY_ENTER;
  window->keycodes[0x001] = PURRR_KEY_ESCAPE;
  window->keycodes[0x147] = PURRR_KEY_HOME;
  window->keycodes[0x152] = PURRR_KEY_INSERT;
  window->keycodes[0x15D] = PURRR_KEY_MENU;
  window->keycodes[0x151] = PURRR_KEY_PAGE_DOWN;
  window->keycodes[0x149] = PURRR_KEY_PAGE_UP;
  window->keycodes[0x045] = PURRR_KEY_PAUSE;
  window->keycodes[0x039] = PURRR_KEY_SPACE;
  window->keycodes[0x00F] = PURRR_KEY_TAB;
  window->keycodes[0x03A] = PURRR_KEY_CAPS_LOCK;
  window->keycodes[0x145] = PURRR_KEY_NUM_LOCK;
  window->keycodes[0x046] = PURRR_KEY_SCROLL_LOCK;
  window->keycodes[0x03B] = PURRR_KEY_F1;
  window->keycodes[0x03C] = PURRR_KEY_F2;
  window->keycodes[0x03D] = PURRR_KEY_F3;
  window->keycodes[0x03E] = PURRR_KEY_F4;
  window->keycodes[0x03F] = PURRR_KEY_F5;
  window->keycodes[0x040] = PURRR_KEY_F6;
  window->keycodes[0x041] = PURRR_KEY_F7;
  window->keycodes[0x042] = PURRR_KEY_F8;
  window->keycodes[0x043] = PURRR_KEY_F9;
  window->keycodes[0x044] = PURRR_KEY_F10;
  window->keycodes[0x057] = PURRR_KEY_F11;
  window->keycodes[0x058] = PURRR_KEY_F12;
  window->keycodes[0x064] = PURRR_KEY_F13;
  window->keycodes[0x065] = PURRR_KEY_F14;
  window->keycodes[0x066] = PURRR_KEY_F15;
  window->keycodes[0x067] = PURRR_KEY_F16;
  window->keycodes[0x068] = PURRR_KEY_F17;
  window->keycodes[0x069] = PURRR_KEY_F18;
  window->keycodes[0x06A] = PURRR_KEY_F19;
  window->keycodes[0x06B] = PURRR_KEY_F20;
  window->keycodes[0x06C] = PURRR_KEY_F21;
  window->keycodes[0x06D] = PURRR_KEY_F22;
  window->keycodes[0x06E] = PURRR_KEY_F23;
  window->keycodes[0x076] = PURRR_KEY_F24;
  window->keycodes[0x038] = PURRR_KEY_LEFT_ALT;
  window->keycodes[0x01D] = PURRR_KEY_LEFT_CONTROL;
  window->keycodes[0x02A] = PURRR_KEY_LEFT_SHIFT;
  window->keycodes[0x15B] = PURRR_KEY_LEFT_SUPER;
  window->keycodes[0x137] = PURRR_KEY_PRINT_SCREEN;
  window->keycodes[0x138] = PURRR_KEY_RIGHT_ALT;
  window->keycodes[0x11D] = PURRR_KEY_RIGHT_CONTROL;
  window->keycodes[0x036] = PURRR_KEY_RIGHT_SHIFT;
  window->keycodes[0x15C] = PURRR_KEY_RIGHT_SUPER;
  window->keycodes[0x150] = PURRR_KEY_DOWN;
  window->keycodes[0x14B] = PURRR_KEY_LEFT;
  window->keycodes[0x14D] = PURRR_KEY_RIGHT;
  window->keycodes[0x148] = PURRR_KEY_UP;

  window->keycodes[0x052] = PURRR_KEY_NP_0;
  window->keycodes[0x04F] = PURRR_KEY_NP_1;
  window->keycodes[0x050] = PURRR_KEY_NP_2;
  window->keycodes[0x051] = PURRR_KEY_NP_3;
  window->keycodes[0x04B] = PURRR_KEY_NP_4;
  window->keycodes[0x04C] = PURRR_KEY_NP_5;
  window->keycodes[0x04D] = PURRR_KEY_NP_6;
  window->keycodes[0x047] = PURRR_KEY_NP_7;
  window->keycodes[0x048] = PURRR_KEY_NP_8;
  window->keycodes[0x049] = PURRR_KEY_NP_9;
  window->keycodes[0x04E] = PURRR_KEY_NP_ADD;
  window->keycodes[0x053] = PURRR_KEY_NP_DECIMAL;
  window->keycodes[0x135] = PURRR_KEY_NP_DIVIDE;
  window->keycodes[0x11C] = PURRR_KEY_NP_ENTER;
  window->keycodes[0x059] = PURRR_KEY_NP_EQUAL;
  window->keycodes[0x037] = PURRR_KEY_NP_MULTIPLY;
  window->keycodes[0x04A] = PURRR_KEY_NP_SUBTRACT;

  for (WORD scancode = 0; scancode < 512; ++scancode) {
    if (window->keycodes[scancode] != (Purrr_Key)-1)
      window->scancodes[window->keycodes[scancode]] = scancode;
  }
}

static Purrr_Key_Modifiers get_modifiers() {
  Purrr_Key_Modifiers modifiers = 0;

  if (GetKeyState(VK_SHIFT) & 0x8000)
    modifiers |= PURRR_KEY_MODIFIER_SHIFT;
  if (GetKeyState(VK_CONTROL) & 0x8000)
    modifiers |= PURRR_KEY_MODIFIER_CONTROL;
  if (GetKeyState(VK_MENU) & 0x8000)
    modifiers |= PURRR_KEY_MODIFIER_ALT;
  if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
    modifiers |= PURRR_KEY_MODIFIER_SUPER;
  if (GetKeyState(VK_CAPITAL) & 1)
    modifiers |= PURRR_KEY_MODIFIER_CAPS_LOCK;
  if (GetKeyState(VK_NUMLOCK) & 1)
    modifiers |= PURRR_KEY_MODIFIER_NUM_LOCK;

  return modifiers;
}

#endif // PURRR_PLATFORM_WINDOWS