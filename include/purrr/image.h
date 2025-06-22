#ifndef   _PURRR_IMAGE_H_
#define   _PURRR_IMAGE_H_

typedef struct Purrr_Image_T *Purrr_Image;

typedef enum Purrr_Image_Usage_Flag_Bits {
  PURRR_IMAGE_USAGE_FLAG_TEXTURE = (1<<0),
  PURRR_IMAGE_USAGE_FLAG_ATTACHMENT = (1<<1),

  COUNT_PURRR_IMAGE_USAGES
} Purrr_Image_Usage_Flag_Bits;
typedef uint32_t Purrr_Image_Usage_Flags;

typedef struct Purrr_Image_Create_Info {
  Purrr_Image_Usage_Flags usage;
  Purrr_Format format;
  uint32_t width, height;
  void *pixels;
  Purrr_Sampler sampler;
} Purrr_Image_Create_Info;

PURRR_API Purrr_Result purrr_create_image(Purrr_Context context, Purrr_Image_Create_Info createInfo, Purrr_Image *image);
PURRR_API Purrr_Result purrr_destroy_image(Purrr_Image image);

#endif // _PURRR_IMAGE_H_