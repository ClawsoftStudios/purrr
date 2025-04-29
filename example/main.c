#include <purrr/purrr.h>

#include <stdio.h>

int main(void) {
  Purrr_Result result = PURRR_SUCCESS;
  Purrr_Window window = {0};
  if ((result = purrr_create_window((Purrr_Window_Create_Info){
    .title = "Purrr example",
    .width = 1920,
    .height = 1080
  }, &window)) < PURRR_SUCCESS) {
    fprintf(stderr, "Failed to create window: %s\n", purrr_result_as_cstr(result));
    return 1;
  }

  while (!purrr_should_window_close(window)) {
    purrr_poll_windows();
  }

  return 0;
}