#ifndef   __PURRR_INTERNAL_H_
#define   __PURRR_INTERNAL_H_

#include "purrr/purrr.h"

// Here I use object as a name of a structure such as a context or window.
// Didn't have an idea for a better name :p

typedef enum _Purrr_Object_Type {
  _PURRR_OBJECT_CONTEXT = 0,
  _PURRR_OBJECT_RENDERER,
  _PURRR_OBJECT_WINDOW,
  _PURRR_OBJECT_BUFFER,
  _PURRR_OBJECT_IMAGE,
  _PURRR_OBJECT_RENDER_TARGET,
  _PURRR_OBJECT_PROGRAM,

  _COUNT_PURRR_OBJECT_TYPES // Must be less or equal to 16
} _Purrr_Object_Type;

typedef uint8_t _Purrr_Object_Flags;
typedef enum _Purrr_Object_Flag_Bits {
  _PURRR_OBJECT_FLAG_RENDER_TARGET = (1<<0), // If set, context must be first field of the object
} _Purrr_Object_Flag_Bits;

typedef struct _Purrr_Object_Header {
  Purrr_Backend       backend : 3;
  _Purrr_Object_Type  type : 4;
  _Purrr_Object_Flags flags;
} _Purrr_Object_Header;

void *_purrr_malloc_with_header(_Purrr_Object_Header header, size_t size);
_Purrr_Object_Header _purrr_get_header(void *ptr);
void _purrr_free_with_header(void *ptr);



struct Purrr_Window {
  Purrr_Context context;

  bool shouldClose;
  int width, height;

  void *platformData;
  void *backendData;
};

#endif // __PURRR_INTERNAL_H_