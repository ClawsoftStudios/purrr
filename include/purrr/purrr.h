#ifndef   _PURRR_H_
#define   _PURRR_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) && defined(PURRR_BUILD_DLL)
  #define PURRR_API __declspec(dllexport)
#elif defined(_WIN32) && defined(PURRR_DLL)
  #define PURRR_API __declspec(dllimport)
#elif defined(PURRR_BUILD_DLL)
  #define PURRR_API __attribute__((visibility("default")))
#else
  #define PURRR_API
#endif

#ifdef    __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct Purrr_Handle_T *Purrr_Handle;

#include "result.h"
#include "format.h"
#include "context.h"
#include "buffer.h"
#include "sampler.h"
#include "image.h"
#include "program.h"
#include "renderer.h"
#include "window.h"
#include "renderTarget.h"

#ifdef    __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus
#define PURRR_NULL_HANDLE nullptr
#else
#define PURRR_NULL_HANDLE NULL
#endif // __cplusplus

#endif // _PURRR_H_