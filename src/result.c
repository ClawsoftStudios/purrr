#include "purrr/result.h"

#include <assert.h>

const char *purrr_result_as_cstr(Purrr_Result result) {
  switch (result) {
  case PURRR_INTERNAL_ERROR: return "Internal error";
  case PURRR_INVALID_ARGS_ERROR: return "Invalid args error";
  case PURRR_BUY_MORE_RAM: return "Out of memory";
  case PURRR_SUCCESS: return "Success";
  case PURRR_TRUE: return "True";
  }

  assert(0 && "Unreachable");
}