#ifndef   _PURRR_CONTEXT_H_
#define   _PURRR_CONTEXT_H_

typedef struct Purrr_Context *Purrr_Context;

typedef enum Purrr_Backend {
  PURRR_VULKAN = 0,

  COUNT_PURRR_BACKENDS
} Purrr_Backend;

typedef struct Purrr_Context_Create_Info {
  Purrr_Backend backend;
} Purrr_Context_Create_Info;

Purrr_Result purrr_create_context(Purrr_Context_Create_Info createInfo, Purrr_Context *context);
Purrr_Result purrr_destroy_context(Purrr_Context context);

#endif // _PURRR_CONTEXT_H_