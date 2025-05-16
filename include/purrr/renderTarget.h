#ifndef   _PURRR_RENDER_TARGET_H_
#define   _PURRR_RENDER_TARGET_H_

#define PURRR_NO_DEPTH UINT32_MAX

typedef struct Purrr_Render_Target_T *Purrr_Render_Target;

typedef struct Purrr_Render_Target_Create_Info {
  uint32_t imageCount;
  Purrr_Image *images;
  uint32_t depthIndex;

  uint32_t width, height;
} Purrr_Render_Target_Create_Info;

Purrr_Result purrr_create_render_target(Purrr_Context context, Purrr_Render_Target_Create_Info createInfo, Purrr_Render_Target *renderTarget);
Purrr_Result purrr_destroy_render_target(Purrr_Render_Target renderTarget);

Purrr_Result purrr_get_render_target_image(Purrr_Render_Target renderTarget, uint32_t imageIndex, Purrr_Image *image);

Purrr_Result purrr_resize_render_target(Purrr_Render_Target renderTarget, uint32_t newWidth, uint32_t newHeight);

#endif // _PURRR_RENDER_TARGET_H_