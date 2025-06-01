#ifndef   _PURRR_WINDOW_H_
#define   _PURRR_WINDOW_H_

#define _PURRR_WINDOW_GET_KEY(keys, key) ((keys[key/8] & 1<<(key%8))?1:0)

typedef struct Purrr_Window_T *Purrr_Window;

typedef struct Purrr_Window_Create_Info {
  const char *title;
  int width;
  int height;
  bool depth;

  Purrr_Image_Usage_Flags imageUsage;
  Purrr_Sampler sampler;
} Purrr_Window_Create_Info;

typedef enum Purrr_Key { // inspired by https://www.glfw.org/docs/3.3/group__keys.html
  PURRR_KEY_SPACE         = ' ',
  PURRR_KEY_APOSTROPHE    = '\'',
  PURRR_KEY_COMMA         = ',',
  PURRR_KEY_MINUS         = '-',
  PURRR_KEY_PERIOD        = '.',
  PURRR_KEY_SLASH         = '/',
  PURRR_KEY_0             = '0',
  PURRR_KEY_1             = '1',
  PURRR_KEY_2             = '2',
  PURRR_KEY_3             = '3',
  PURRR_KEY_4             = '4',
  PURRR_KEY_5             = '5',
  PURRR_KEY_6             = '6',
  PURRR_KEY_7             = '7',
  PURRR_KEY_8             = '8',
  PURRR_KEY_9             = '9',
  PURRR_KEY_SEMICOLON     = ';',
  PURRR_KEY_EQUAL         = '=',
  PURRR_KEY_A             = 'A',
  PURRR_KEY_B             = 'B',
  PURRR_KEY_C             = 'C',
  PURRR_KEY_D             = 'D',
  PURRR_KEY_E             = 'E',
  PURRR_KEY_F             = 'F',
  PURRR_KEY_G             = 'G',
  PURRR_KEY_H             = 'H',
  PURRR_KEY_I             = 'I',
  PURRR_KEY_J             = 'J',
  PURRR_KEY_K             = 'K',
  PURRR_KEY_L             = 'L',
  PURRR_KEY_M             = 'M',
  PURRR_KEY_N             = 'N',
  PURRR_KEY_O             = 'O',
  PURRR_KEY_P             = 'P',
  PURRR_KEY_Q             = 'Q',
  PURRR_KEY_R             = 'R',
  PURRR_KEY_S             = 'S',
  PURRR_KEY_T             = 'T',
  PURRR_KEY_U             = 'U',
  PURRR_KEY_V             = 'V',
  PURRR_KEY_W             = 'W',
  PURRR_KEY_X             = 'X',
  PURRR_KEY_Y             = 'Y',
  PURRR_KEY_Z             = 'Z',
  PURRR_KEY_LEFT_BRACKET  = '[',
  PURRR_KEY_BACKSLASH     = '\\',
  PURRR_KEY_RIGHT_BRACKET = ']',
  PURRR_KEY_GRAVE_ACCENT  = '`',
  PURRR_KEY_LEFT_CONTROL,
  PURRR_KEY_RIGHT_CONTROL,
  PURRR_KEY_LEFT_SHIFT,
  PURRR_KEY_RIGHT_SHIFT,
  PURRR_KEY_LEFT_ALT,
  PURRR_KEY_RIGHT_ALT,
  PURRR_KEY_LEFT_SUPER,
  PURRR_KEY_RIGHT_SUPER,
  PURRR_KEY_BACKSPACE,
  PURRR_KEY_DELETE,
  PURRR_KEY_END,
  PURRR_KEY_ENTER,
  PURRR_KEY_ESCAPE,
  PURRR_KEY_HOME,
  PURRR_KEY_INSERT,
  PURRR_KEY_MENU,
  PURRR_KEY_PAGE_DOWN,
  PURRR_KEY_PAGE_UP,
  PURRR_KEY_PAUSE,
  PURRR_KEY_TAB,
  PURRR_KEY_CAPS_LOCK,
  PURRR_KEY_NUM_LOCK,
  PURRR_KEY_SCROLL_LOCK,
  PURRR_KEY_F1,
  PURRR_KEY_F2,
  PURRR_KEY_F3,
  PURRR_KEY_F4,
  PURRR_KEY_F5,
  PURRR_KEY_F6,
  PURRR_KEY_F7,
  PURRR_KEY_F8,
  PURRR_KEY_F9,
  PURRR_KEY_F10,
  PURRR_KEY_F11,
  PURRR_KEY_F12,
  PURRR_KEY_F13,
  PURRR_KEY_F14,
  PURRR_KEY_F15,
  PURRR_KEY_F16,
  PURRR_KEY_F17,
  PURRR_KEY_F18,
  PURRR_KEY_F19,
  PURRR_KEY_F20,
  PURRR_KEY_F21,
  PURRR_KEY_F22,
  PURRR_KEY_F23,
  PURRR_KEY_F24,
  PURRR_KEY_PRINT_SCREEN,
  PURRR_KEY_LEFT,
  PURRR_KEY_RIGHT,
  PURRR_KEY_UP,
  PURRR_KEY_DOWN,
  PURRR_KEY_NP_0,
  PURRR_KEY_NP_1,
  PURRR_KEY_NP_2,
  PURRR_KEY_NP_3,
  PURRR_KEY_NP_4,
  PURRR_KEY_NP_5,
  PURRR_KEY_NP_6,
  PURRR_KEY_NP_7,
  PURRR_KEY_NP_8,
  PURRR_KEY_NP_9,
  PURRR_KEY_NP_ADD,
  PURRR_KEY_NP_DECIMAL,
  PURRR_KEY_NP_DIVIDE,
  PURRR_KEY_NP_ENTER,
  PURRR_KEY_NP_EQUAL,
  PURRR_KEY_NP_MULTIPLY,
  PURRR_KEY_NP_SUBTRACT,
  PURRR_KEY_WORLD_2,

  COUNT_PURRR_KEYS
} Purrr_Key;

typedef enum Purrr_Key_Modifier {
  PURRR_KEY_MODIFIER_SHIFT     = (1<<0),
  PURRR_KEY_MODIFIER_CONTROL   = (1<<1),
  PURRR_KEY_MODIFIER_ALT       = (1<<2),
  PURRR_KEY_MODIFIER_SUPER     = (1<<3),
  PURRR_KEY_MODIFIER_CAPS_LOCK = (1<<4),
  PURRR_KEY_MODIFIER_NUM_LOCK  = (1<<5)
} Purrr_Key_Modifier;

typedef uint8_t Purrr_Key_Modifiers;

typedef enum Purrr_Key_Action {
  PURRR_KEY_ACTION_RELEASE = 0,
  PURRR_KEY_ACTION_PRESS,
  PURRR_KEY_ACTION_REPEAT,

  COUNT_PURRR_KEY_ACTIONS
} Purrr_Key_Action;

typedef void (*Purrr_Window_Key_Callback)(Purrr_Window, int16_t, Purrr_Key, Purrr_Key_Action, Purrr_Key_Modifiers);
typedef void (*Purrr_Window_Cursor_Move_Callback)(Purrr_Window, double, double);
typedef void (*Purrr_Window_Cursor_Enter_Callback)(Purrr_Window);
typedef void (*Purrr_Window_Cursor_Leave_Callback)(Purrr_Window);
typedef void (*Purrr_Window_Scroll_Callback)(Purrr_Window, double, double);

Purrr_Result purrr_create_window(Purrr_Renderer renderer, Purrr_Window_Create_Info createInfo, Purrr_Window *window);
Purrr_Result purrr_destroy_window(Purrr_Window window);

Purrr_Result purrr_get_window_image(Purrr_Window window, Purrr_Image *image);

Purrr_Result purrr_should_window_close(Purrr_Window window);
Purrr_Result purrr_get_window_size(Purrr_Window window, int *width, int *height);

Purrr_Result purrr_is_window_key_down(Purrr_Window window, Purrr_Key key);
Purrr_Result purrr_is_window_key_up(Purrr_Window window, Purrr_Key key);

Purrr_Result purrr_get_window_cursor_pos(Purrr_Window window, double *xpos, double *ypos);
Purrr_Result purrr_set_window_cursor_pos(Purrr_Window window, double xpos, double ypos);

void purrr_set_window_key_callback(Purrr_Window window, Purrr_Window_Key_Callback callback);
void purrr_set_window_cursor_move_callback(Purrr_Window window, Purrr_Window_Cursor_Move_Callback callback);
void purrr_set_window_cursor_enter_callback(Purrr_Window window, Purrr_Window_Cursor_Enter_Callback callback);
void purrr_set_window_cursor_leave_callback(Purrr_Window window, Purrr_Window_Cursor_Leave_Callback callback);
void purrr_set_window_scroll_callback(Purrr_Window window, Purrr_Window_Scroll_Callback callback);

void purrr_set_window_user_pointer(Purrr_Window window, void *userPointer);
void *purrr_get_window_user_pointer(Purrr_Window window);

double purrr_get_windows_time();
void purrr_poll_windows();
void purrr_wait_windows();

#endif // _PURRR_WINDOW_H_