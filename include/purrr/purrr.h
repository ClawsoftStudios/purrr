#ifndef   _PURRR_H_
#define   _PURRR_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef    __cplusplus
#extern "C" {
#endif // __cplusplus

typedef struct Purrr_Handle *Purrr_Handle;

#include "result.h"
#include "format.h"
#include "context.h"
#include "buffer.h"
#include "program.h"
#include "renderer.h"
#include "window.h"

#ifdef    __cplusplus
}
#endif // __cplusplus

#endif // _PURRR_H_