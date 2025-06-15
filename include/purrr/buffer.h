#ifndef   _PURRR_BUFFER_H_
#define   _PURRR_BUFFER_H_

typedef struct Purrr_Buffer_T *Purrr_Buffer;

typedef enum Purrr_Buffer_Type {
  PURRR_BUFFER_VERTEX = 0,
  PURRR_BUFFER_INDEX,
  PURRR_BUFFER_UNIFORM,

  COUNT_PURRR_BUFFER_TYPES // Must be less or equal to 8
} Purrr_Buffer_Type;

typedef struct Purrr_Buffer_Create_Info {
  Purrr_Buffer_Type type;
  uint32_t size;
  bool hostVisible;
} Purrr_Buffer_Create_Info;

Purrr_Result purrr_create_buffer(Purrr_Context context, Purrr_Buffer_Create_Info createInfo, Purrr_Buffer *buffer);
Purrr_Result purrr_destroy_buffer(Purrr_Buffer buffer);

Purrr_Result purrr_copy_buffer_data(Purrr_Buffer dst, void *src, uint32_t size, uint32_t offset);
Purrr_Result purrr_copy_buffer(Purrr_Buffer dst, Purrr_Buffer src, uint32_t offset);

#endif // _PURRR_BUFFER_H_