#ifndef   _PURRR_IMAGE_H_
#define   _PURRR_IMAGE_H_

typedef struct Purrr_Image *Purrr_Image;

typedef enum Purrr_Image_Type {
  PURRR_IMAGE_TEXTURE = 0,
  // PURRR_IMAGE_COLOR_ATTACHMENT,
  // PURRR_IMAGE_DEPTH_ATTACHMENT,

  COUNT_PURRR_IMAGE_TYPES
} Purrr_Image_Type;

typedef struct Purrr_Image_Create_Info {
  Purrr_Image_Type type;
  Purrr_Format format;
  uint32_t width, height;
  void *pixels;
} Purrr_Image_Create_Info;

Purrr_Result purrr_create_image(Purrr_Context context, Purrr_Image_Create_Info createInfo, Purrr_Image *image);
Purrr_Result purrr_destroy_image(Purrr_Image image);

#endif // _PURRR_IMAGE_H_