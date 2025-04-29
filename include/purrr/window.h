#ifndef   _PURRR_WINDOW_H_
#define   _PURRR_WINDOW_H_

typedef struct Purrr_Window *Purrr_Window;

typedef struct Purrr_Window_Create_Info {
  const char *title;
  int width;
  int height;
} Purrr_Window_Create_Info;

Purrr_Result purrr_create_window(Purrr_Window_Create_Info createInfo, Purrr_Window *window);
Purrr_Result purrr_destroy_window(Purrr_Window window);

Purrr_Result purrr_should_window_close(Purrr_Window window);
void purrr_poll_windows();

#endif // _PURRR_WINDOW_H_