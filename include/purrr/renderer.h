#ifndef   _PURRR_RENDERER_H_
#define   _PURRR_RENDERER_H_

typedef struct Purrr_Renderer_T *Purrr_Renderer;

typedef struct Purrr_Renderer_Create_Info {
  char x;
} Purrr_Renderer_Create_Info;

typedef struct Purrr_Color {
  float r, g, b, a;
} Purrr_Color;

// `hex` is assumed to be 0xRRGGBBAA
#define PURRR_COLOR(hex) ((Purrr_Color){ \
  .r = (float)(((hex) >> (8*3)) & 0xFF) / 255.0f, \
  .g = (float)(((hex) >> (8*2)) & 0xFF) / 255.0f, \
  .b = (float)(((hex) >> (8*1)) & 0xFF) / 255.0f, \
  .a = (float)(((hex) >> (8*0)) & 0xFF) / 255.0f  \
})

PURRR_API Purrr_Result purrr_create_renderer(Purrr_Context context, Purrr_Renderer_Create_Info createInfo, Purrr_Renderer *renderer);
PURRR_API Purrr_Result purrr_destroy_renderer(Purrr_Renderer renderer);

PURRR_API Purrr_Result purrr_wait_renderer(Purrr_Renderer renderer);
PURRR_API Purrr_Result purrr_begin_renderer(Purrr_Renderer renderer);
PURRR_API Purrr_Result purrr_renderer_begin(Purrr_Renderer renderer, void *renderTarget, Purrr_Color color);
PURRR_API Purrr_Result purrr_renderer_bind_buffer(Purrr_Renderer renderer, Purrr_Buffer buffer, uint32_t index);
PURRR_API Purrr_Result purrr_renderer_bind_image(Purrr_Renderer renderer, Purrr_Image image, uint32_t index);
PURRR_API Purrr_Result purrr_renderer_bind_program(Purrr_Renderer renderer, Purrr_Program program);
PURRR_API Purrr_Result purrr_renderer_draw_indexed(Purrr_Renderer renderer, uint32_t indexCount);
PURRR_API Purrr_Result purrr_renderer_draw_indexed_instanced(Purrr_Renderer renderer, uint32_t indexCount, uint32_t instanceCount);
PURRR_API Purrr_Result purrr_renderer_draw_indexed_indirect(Purrr_Renderer renderer, Purrr_Buffer buffer, uint32_t drawCount);
PURRR_API Purrr_Result purrr_renderer_end(Purrr_Renderer renderer);
PURRR_API Purrr_Result purrr_render_renderer(Purrr_Renderer renderer);

#endif // _PURRR_RENDERER_H_