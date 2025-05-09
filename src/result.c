#include "purrr/result.h"

#include <stddef.h>
#include <assert.h>

const char *purrr_result_as_cstr(Purrr_Result result) {
  switch (result) {
  case PURRR_FILE_SYSTEM_ERROR: return "File system error";
  case PURRR_INTERNAL_ERROR: return "Internal error";
  case PURRR_INVALID_ARGS_ERROR: return "Invalid args error";
  case PURRR_BUY_MORE_RAM: return "Out of memory";
  case PURRR_SUCCESS: return "Success";
  case PURRR_TRUE: return "True";
  case PURRR_MINIMIZED: return "Minimized";
  case PURRR_INACTIVE: return "Inactive";
  }

  assert(0 && "Unreachable");
  return NULL;
}