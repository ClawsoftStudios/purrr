#ifndef   _PURRR_RESULT_H_
#define   _PURRR_RESULT_H_

typedef enum Purrr_Result {
  PURRR_FILE_SYSTEM_ERROR = -4,
  PURRR_INTERNAL_ERROR = -3,
  PURRR_INVALID_ARGS_ERROR = -2,
  PURRR_BUY_MORE_RAM = -1,
  PURRR_SUCCESS = 0,
  PURRR_TRUE = 1,
  PURRR_MINIMIZED = 2,
  PURRR_INACTIVE = 3,
} Purrr_Result;

const char *purrr_result_as_cstr(Purrr_Result result);

#endif // _PURRR_RESULT_H_