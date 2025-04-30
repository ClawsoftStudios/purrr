#include <purrr/purrr.h>

#include <stdio.h>

#define CHECK(call) \
  if ((result = (call)) < PURRR_SUCCESS) { \
    fprintf(stderr, #call" failed with an error: %s!\n", purrr_result_as_cstr(result)); \
    return 1; \
  }

Purrr_Backend _purrr_get_context_backend(Purrr_Context context);

int main(void) {
  Purrr_Result result = PURRR_SUCCESS;

  Purrr_Context context = {0};
  CHECK(purrr_create_context((Purrr_Context_Create_Info){
    .backend = PURRR_VULKAN
  }, &context));

  Purrr_Window window = {0};
  CHECK(purrr_create_window((Purrr_Window_Create_Info){
    .title = "Purrr example",
    .width = 1920,
    .height = 1080
  }, &window));

  // purrr_should_window_close returns PURRR_INVALID_ARGS_ERROR (-2)
  // if the window passed to it is invalid, otherwise it returns
  // either PURRR_SUCCESS (0) or PURRR_TRUE (1) depending on if the
  // window should close or not.
  while (!purrr_should_window_close(window)) {
    purrr_poll_windows();
  }

  (void)purrr_destroy_window(window);
  (void)purrr_destroy_context(context);

  return 0;
}