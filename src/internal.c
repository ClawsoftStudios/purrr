#include "./internal.h"

#include <stdlib.h>
#include <string.h>

void *_purrr_malloc_with_header(_Purrr_Object_Header header, size_t size) {
  _Purrr_Object_Header *ptr = malloc(sizeof(_Purrr_Object_Header) + size);
  if (!ptr) return ptr;
  memcpy(ptr, &header, sizeof(header));
  return &ptr[1]; // I think I saw that trick while watching a Tsoding session when he went deepah into stb_ds' hash maps, I forgot what the stream was about tho
}

_Purrr_Object_Header _purrr_get_header(void *ptr) {
  if (!ptr) return (_Purrr_Object_Header){
    .backend = COUNT_PURRR_BACKENDS,
    .type = COUNT_PURRR_BACKENDS
  };
  return ((_Purrr_Object_Header*)ptr)[-1];
}

void _purrr_free_with_header(void *ptr) {
  if (ptr) free(&((_Purrr_Object_Header*)ptr)[-1]);
}