#include "vulkan.h"

#include <stdlib.h>

void *_purrr_malloc_item_vulkan(size_t size) {
  Purrr_Backend *ptr = malloc(sizeof(Purrr_Backend) + size);
  if (!ptr) return ptr;
  *ptr = PURRR_VULKAN;
  return &ptr[1];
}