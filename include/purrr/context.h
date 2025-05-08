#ifndef   _PURRR_CONTEXT_H_
#define   _PURRR_CONTEXT_H_

typedef struct Purrr_Context *Purrr_Context;

typedef enum Purrr_Backend {
  PURRR_VULKAN = 0,

  COUNT_PURRR_BACKENDS // Must be less or equal to 8
} Purrr_Backend;

typedef uint32_t Purrr_Version;
#define PURRR_MAKE_VERSION(major, minor, patch) (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch))
#define PURRR_VERSION PURRR_MAKE_VERSION(1, 0, 0)

typedef struct Purrr_Context_Create_Info {
  const char *applicationName;
  Purrr_Version applicationVersion;
  const char *engineName;
  Purrr_Version engineVersion;
} Purrr_Context_Create_Info;

Purrr_Result purrr_create_context(Purrr_Backend backend, Purrr_Context_Create_Info createInfo, Purrr_Context *context);
Purrr_Result purrr_destroy_context(Purrr_Context context);

#endif // _PURRR_CONTEXT_H_