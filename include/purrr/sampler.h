#ifndef   _PURRR_SAMPLER_H_
#define   _PURRR_SAMPLER_H_

typedef struct Purrr_Sampler_T *Purrr_Sampler;

typedef enum Purrr_Filter {
  PURRR_NEAREST = 0,
  PURRR_LINEAR,

  COUNT_PURRR_FILTERS
} Purrr_Filter;

typedef enum Purrr_Sampler_Address_Mode {
  PURRR_SAMPLER_ADDRESS_REPEAT = 0,
  PURRR_SAMPLER_ADDRESS_MIRROR,
  PURRR_SAMPLER_ADDRESS_CLAMP_TO_EDGE,
  PURRR_SAMPLER_ADDRESS_CLAMP_TO_BORDER,

  COUNT_PURRR_SAMPLER_ADRESS_MODES
} Purrr_Sampler_Address_Mode;

typedef enum Purrr_Sampler_Border_Color {
  PURRR_SAMPLER_BORDER_FLOAT_TRANSPARENT_BLACK = 0,
  PURRR_SAMPLER_BORDER_INT_TRANSPARENT_BLACK,
  PURRR_SAMPLER_BORDER_FLOAT_OPAQUE_BLACK,
  PURRR_SAMPLER_BORDER_INT_OPAQUE_BLACK,
  PURRR_SAMPLER_BORDER_FLOAT_OPAQUE_WHITE,
  PURRR_SAMPLER_BORDER_INT_OPAQUE_WHITE,

  COUNT_PURRR_SAMPLER_BORDER_COLORS
} Purrr_Sampler_Border_Color;

typedef struct Purrr_Sampler_Create_Info {
  Purrr_Filter magFilter;
  Purrr_Filter minFilter;
  Purrr_Filter mipFilter;
  Purrr_Sampler_Address_Mode addressModeU;
  Purrr_Sampler_Address_Mode addressModeV;
  Purrr_Sampler_Address_Mode addressModeW;
  float mipLodBias;
  float maxAnisotropy;
  float minLod;
  float maxLod;
  Purrr_Sampler_Border_Color borderColor;
} Purrr_Sampler_Create_Info;

Purrr_Result purrr_create_sampler(Purrr_Context context, Purrr_Sampler_Create_Info createInfo, Purrr_Sampler *sampler);
Purrr_Result purrr_destroy_sampler(Purrr_Sampler sampler);

#endif // _PURRR_SAMPLER_H_