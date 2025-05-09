#include <purrr/purrr.h>

#include <stdio.h>
#include <string.h>

#define WINDOW_COUNT 1

#define CHECK(call) \
  if ((result = (call)) < PURRR_SUCCESS) { \
    fprintf(stderr, #call" failed with an error: %s!\n", purrr_result_as_cstr(result)); \
    return 1; \
  }

typedef struct Vertex {
  float x, y;
  float u, v;
} Vertex;

static Vertex sVertices[] = {
  (Vertex){
    .x = -1.0f, .y = -1.0f,
    .u =  1.0f, .v =  0.0f,
  },
  (Vertex){
    .x =  1.0f, .y = -1.0f,
    .u =  0.0f, .v =  0.0f,
  },
  (Vertex){
    .x =  1.0f, .y =  1.0f,
    .u =  0.0f, .v =  1.0f,
  },
  (Vertex){
    .x = -1.0f, .y =  1.0f,
    .u =  1.0f, .v =  1.0f,
  }
};

typedef uint32_t Index;

static Index sIndices[] = {
  0, 2, 1,
  2, 0, 3
};

int main(void) {
  Purrr_Result result = PURRR_SUCCESS;

  Purrr_Context context = {0};
  CHECK(purrr_create_context(PURRR_VULKAN, (Purrr_Context_Create_Info) {
    .applicationName = "purrr example",
    .applicationVersion = PURRR_MAKE_VERSION(0, 1, 0),
    .engineName = "purrr",
    .engineVersion = PURRR_VERSION
  }, & context));

  Purrr_Buffer vertexBuffer = {0};
  CHECK(purrr_create_buffer(context, (Purrr_Buffer_Create_Info){
    .type = PURRR_BUFFER_VERTEX,
    .size = sizeof(sVertices)
  }, &vertexBuffer));

  CHECK(purrr_copy_buffer_data(vertexBuffer, sVertices, sizeof(sVertices), 0));

  Purrr_Buffer indexBuffer = {0};
  CHECK(purrr_create_buffer(context, (Purrr_Buffer_Create_Info){
    .type = PURRR_BUFFER_INDEX,
    .size = sizeof(sIndices)
  }, &indexBuffer));

  CHECK(purrr_copy_buffer_data(indexBuffer, sIndices, sizeof(sIndices), 0));

  Purrr_Renderer renderer = { 0 };
  CHECK(purrr_create_renderer(context, (Purrr_Renderer_Create_Info) {
    0
  }, & renderer));

  Purrr_Window windows[WINDOW_COUNT] = {0};
  Purrr_Program programs[WINDOW_COUNT] = {0};

  {
    char windowTitle[9] = {0};
    memcpy(windowTitle, "Window ", 7);
    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      windowTitle[7] = '1'+i;

      CHECK(purrr_create_window(renderer, (Purrr_Window_Create_Info){
        .title = windowTitle,
        .width = 1920/WINDOW_COUNT,
        .height = 1080/WINDOW_COUNT,
        .depth = false
      }, &windows[i]));

      CHECK(purrr_create_program((Purrr_Handle)windows[i], (Purrr_Program_Create_Info){
        .shaderCount = 2,
        .shaders = (Purrr_Program_Shader_Info[]){
          (Purrr_Program_Shader_Info){
            .type = PURRR_PROGRAM_SHADER_VERTEX,
            .filepath = "./vertexShader.spv"
          },
          (Purrr_Program_Shader_Info){
            .type = PURRR_PROGRAM_SHADER_FRAGMENT,
            .filepath = "./fragmentShader.spv"
          }
        },
        .vertexBindingCount = 1,
        .vertexBindings = (Purrr_Program_Vertex_Binding_Info[]){
          (Purrr_Program_Vertex_Binding_Info){
            .stride = sizeof(Vertex),
            .attributeCount = 2,
            .attributes = (Purrr_Program_Vertex_Attribute_Info[]){
              (Purrr_Program_Vertex_Attribute_Info){
                .format = PURRR_R32G32_SFLOAT,
                .offset = sizeof(float)*0
              },
              (Purrr_Program_Vertex_Attribute_Info){
                .format = PURRR_R32G32_SFLOAT,
                .offset = sizeof(float)*2
              }
            }
          }
        }
      }, &programs[i]));
    }
  }

  while (true) {
    purrr_poll_windows();

    CHECK(purrr_wait_renderer(renderer));

    bool close = true;

    while (1) {
      close = true;

      for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
        if (!windows[i]) continue;

        // purrr_should_window_close returns PURRR_INVALID_ARGS_ERROR (-2)
        // if the window passed to it is invalid, otherwise it returns
        // either PURRR_SUCCESS (0) or PURRR_TRUE (1) depending on if the
        // window should close or not.
        if (!purrr_should_window_close(windows[i])) close = false;
        else {
          (void)purrr_destroy_window(windows[i]);
          windows[i] = NULL;
        }
      }

      if (close) break;

      CHECK(purrr_begin_renderer(renderer));
      if (result == PURRR_INACTIVE) purrr_wait_windows();
      else break;
    }

    if (close) break;

    CHECK(purrr_renderer_bind_buffer(renderer, vertexBuffer, 0));
    CHECK(purrr_renderer_bind_buffer(renderer, indexBuffer, 0));

    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      if (!windows[i]) continue;

      // window is marked as a render target, therefore it's fine to pass it here
      CHECK(purrr_renderer_begin(renderer, windows[i], PURRR_COLOR(0x181818FF)));
      if (result) {
        CHECK(purrr_renderer_bind_program(renderer, programs[i]));

        CHECK(purrr_renderer_draw_indexed(renderer, sizeof(sIndices) / sizeof(*sIndices)));

        CHECK(purrr_renderer_end(renderer));
      }
    }

    CHECK(purrr_render_renderer(renderer));
  }

  CHECK(purrr_wait_renderer(renderer));

  for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
    (void)purrr_destroy_window(windows[i]);
    (void)purrr_destroy_program(programs[i]);
  }

  (void)purrr_destroy_renderer(renderer);

  (void)purrr_destroy_buffer(vertexBuffer);
  (void)purrr_destroy_buffer(indexBuffer);

  (void)purrr_destroy_context(context);

  return 0;
}