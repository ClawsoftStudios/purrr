#include <purrr/purrr.h>

#include <stdio.h>
#include <string.h>

// Local definition
#define LCL_DEF(name, value) enum { name = value }

#define CHECK(call) \
  if ((result = (call)) < PURRR_SUCCESS) { \
    fprintf(stderr, #call" failed with an error: %s!\n", purrr_result_as_cstr(result)); \
    return 1; \
  }

int main(void) {
  Purrr_Result result = PURRR_SUCCESS;

  Purrr_Context context = {0};
  CHECK(purrr_create_context(PURRR_VULKAN, (Purrr_Context_Create_Info) {
    0
  }, & context));

  Purrr_Renderer renderer = { 0 };
  CHECK(purrr_create_renderer(context, (Purrr_Renderer_Create_Info) {
    0
  }, & renderer));

  LCL_DEF(WINDOW_COUNT, 2); // Should be kept below 10

  Purrr_Window windows[WINDOW_COUNT] = {0};

  {
    char windowTitle[9] = {0};
    memcpy(windowTitle, "Window ", 7);
    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      windowTitle[7] = '1'+i;

      CHECK(purrr_create_window(renderer, (Purrr_Window_Create_Info){
        .title = windowTitle,
        .width = 1920/WINDOW_COUNT,
        .height = 1080/WINDOW_COUNT,
        .depth = false
      }, &windows[i]));
    }
  }

  while (true) {
    purrr_poll_windows();

    CHECK(purrr_wait_renderer(renderer));

    bool close = true;
    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      if (!windows[i]) continue;

      // purrr_should_window_close returns PURRR_INVALID_ARGS_ERROR (-2)
      // if the window passed to it is invalid, otherwise it returns
      // either PURRR_SUCCESS (0) or PURRR_TRUE (1) depending on if the
      // window should close or not.
      if (!purrr_should_window_close(windows[i])) close = false;
      else {
        (void)purrr_destroy_window(windows[i]);
        windows[i] = NULL;
      }
    }

    if (close) break;

    CHECK(purrr_begin_renderer(renderer));

    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      if (!windows[i]) continue;

      // window is marked as a render target, therefore it's fine to pass it here
      CHECK(purrr_renderer_begin(renderer, windows[i], PURRR_COLOR(0x181818FF)));
      if (result) {
        CHECK(purrr_renderer_end(renderer));
      }
    }

    CHECK(purrr_render_renderer(renderer));
  }

  CHECK(purrr_wait_renderer(renderer));

  for (uint32_t i = 0; i < WINDOW_COUNT; ++i)
    (void)purrr_destroy_window(windows[i]);

  (void)purrr_destroy_renderer(renderer);
  (void)purrr_destroy_context(context);

  return 0;
}