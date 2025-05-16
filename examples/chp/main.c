#include <purrr/purrr.h>

#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    .u =  0.0f, .v =  0.0f,
  },
  (Vertex){
    .x =  1.0f, .y = -1.0f,
    .u =  1.0f, .v =  0.0f,
  },
  (Vertex){
    .x =  1.0f, .y =  1.0f,
    .u =  1.0f, .v =  1.0f,
  },
  (Vertex){
    .x = -1.0f, .y =  1.0f,
    .u =  0.0f, .v =  1.0f,
  }
};

typedef uint32_t Index;

static Index sIndices[] = {
  0, 2, 1,
  2, 0, 3
};

typedef struct UBO {
  float n;
} UBO;

static Purrr_Result load_image(const char *filepath, Purrr_Context context, Purrr_Sampler sampler, Purrr_Image *image, uint32_t *width, uint32_t *height);

void on_key(Purrr_Window window, int16_t scancode, Purrr_Key key, Purrr_Key_Action action, Purrr_Key_Modifiers modifiers) {
  printf("Key: %d, action: %d\n", key, action);
}

int main(void) {
  Purrr_Result result = PURRR_SUCCESS;

  Purrr_Context context = PURRR_NULL_HANDLE;
  CHECK(purrr_create_context(PURRR_VULKAN, (Purrr_Context_Create_Info) {
    .applicationName = "purrr chp example",
    .applicationVersion = PURRR_MAKE_VERSION(1, 0, 0),
    .engineName = "purrr",
    .engineVersion = PURRR_VERSION
  }, & context));

  Purrr_Buffer vertexBuffer = PURRR_NULL_HANDLE;
  CHECK(purrr_create_buffer(context, (Purrr_Buffer_Create_Info){
    .type = PURRR_BUFFER_VERTEX,
    .size = sizeof(sVertices)
  }, &vertexBuffer));

  CHECK(purrr_copy_buffer_data(vertexBuffer, sVertices, sizeof(sVertices), 0));

  Purrr_Buffer indexBuffer = PURRR_NULL_HANDLE;
  CHECK(purrr_create_buffer(context, (Purrr_Buffer_Create_Info){
    .type = PURRR_BUFFER_INDEX,
    .size = sizeof(sIndices)
  }, &indexBuffer));

  CHECK(purrr_copy_buffer_data(indexBuffer, sIndices, sizeof(sIndices), 0));

  Purrr_Sampler sampler = PURRR_NULL_HANDLE;
  CHECK(purrr_create_sampler(context, (Purrr_Sampler_Create_Info){
    .magFilter = PURRR_LINEAR,
    .minFilter = PURRR_LINEAR,
    .mipFilter = PURRR_LINEAR,
    .addressModeU = PURRR_SAMPLER_ADDRESS_REPEAT,
    .addressModeV = PURRR_SAMPLER_ADDRESS_REPEAT,
    .addressModeW = PURRR_SAMPLER_ADDRESS_REPEAT,
    .mipLodBias = 0.0f,
    .maxAnisotropy = 0.0f,
    .minLod = 0.0f,
    .maxLod = 0.0f,
    .borderColor = PURRR_SAMPLER_BORDER_FLOAT_TRANSPARENT_BLACK
  }, &sampler));

  uint32_t width = 0, height = 0;

  Purrr_Image image = PURRR_NULL_HANDLE;
  CHECK(load_image("./assets/chp.png", context, sampler, &image, &width, &height));

  Purrr_Renderer renderer = { 0 };
  CHECK(purrr_create_renderer(context, (Purrr_Renderer_Create_Info) {
    0
  }, & renderer));

  Purrr_Window windows[WINDOW_COUNT] = { PURRR_NULL_HANDLE };
  Purrr_Program programs[WINDOW_COUNT] = { PURRR_NULL_HANDLE };
  Purrr_Buffer ubos[WINDOW_COUNT] = { PURRR_NULL_HANDLE };

  {
    char windowTitle[9] = {0};
    memcpy(windowTitle, "Window ", 7);
    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      windowTitle[7] = '1'+i;

      CHECK(purrr_create_window(renderer, (Purrr_Window_Create_Info){
        .title = windowTitle,
        .width = width,
        .height = height,
        .depth = true
      }, &windows[i]));

      purrr_set_window_key_callback(windows[i], on_key);

      CHECK(purrr_create_program((Purrr_Handle)windows[i], (Purrr_Program_Create_Info){
        .shaderCount = 2,
        .shaders = (Purrr_Program_Shader_Info[]){
          (Purrr_Program_Shader_Info){
            .type = PURRR_PROGRAM_SHADER_VERTEX,
            .filepath = "./assets/chp.vert.spv"
          },
          (Purrr_Program_Shader_Info){
            .type = PURRR_PROGRAM_SHADER_FRAGMENT,
            .filepath = "./assets/chp.frag.spv"
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
        },
        .bindingCount = 2,
        .bindings = (Purrr_Program_Binding_Type[]){
          PURRR_PROGRAM_BINDING_IMAGE, PURRR_PROGRAM_BINDING_UNIFORM_BUFFER
        },
        .enableDepth = false,
        .compareOp = PURRR_PROGRAM_DEPTH_COMPARE_NEVER
      }, &programs[i]));

      CHECK(purrr_create_buffer(context, (Purrr_Buffer_Create_Info){
        .type = PURRR_BUFFER_UNIFORM,
        .size = sizeof(UBO)
      }, &ubos[i]));
    }
  }

  float ns[WINDOW_COUNT] = { 0.0f };

  while (true) {
    purrr_poll_windows();

    CHECK(purrr_wait_renderer(renderer));

    bool close = true;

    while (1) {
      close = true;

      for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
        if (!windows[i]) continue;

        if (purrr_is_window_key_down(windows[i], PURRR_KEY_ESCAPE) == PURRR_TRUE) {
          (void)purrr_destroy_window(windows[i]);
          windows[i] = NULL;
          continue;
        }

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

    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      Purrr_Result left = purrr_is_window_key_down(windows[i], PURRR_KEY_LEFT);
      Purrr_Result right = purrr_is_window_key_down(windows[i], PURRR_KEY_RIGHT);

      if (left == PURRR_TRUE) ns[i] -= 0.0001f;
      if (right == PURRR_TRUE) ns[i] += 0.0001f;

      if (ns[i] > 1.0f) ns[i] = 0.0f;
      else if (ns[i] < 0.0f) ns[i] = 1.0f;

      CHECK(purrr_copy_buffer_data(ubos[i], &ns[i], sizeof(float), 0));
    }

    CHECK(purrr_renderer_bind_buffer(renderer, vertexBuffer, 0));
    CHECK(purrr_renderer_bind_buffer(renderer, indexBuffer, 0));

    for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
      if (!windows[i]) continue;

      // window is marked as a render target, therefore it's fine to pass it here
      CHECK(purrr_renderer_begin(renderer, windows[i], PURRR_COLOR(0x181818FF)));
      if (result) {
        CHECK(purrr_renderer_bind_program(renderer, programs[i]));

        CHECK(purrr_renderer_bind_image(renderer, image, 0));
        CHECK(purrr_renderer_bind_buffer(renderer, ubos[i], 1));
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
    (void)purrr_destroy_buffer(ubos[i]);
  }

  (void)purrr_destroy_renderer(renderer);

  (void)purrr_destroy_image(image);
  (void)purrr_destroy_sampler(sampler);

  (void)purrr_destroy_buffer(vertexBuffer);
  (void)purrr_destroy_buffer(indexBuffer);

  (void)purrr_destroy_context(context);

  return 0;
}

static Purrr_Result load_image(const char *filepath, Purrr_Context context, Purrr_Sampler sampler, Purrr_Image *image, uint32_t *width, uint32_t *height) {
  if (!filepath || !context || !image) return PURRR_INVALID_ARGS_ERROR;

  stbi_uc *pixels = stbi_load(filepath, width, height, NULL, STBI_rgb_alpha);
  if (!pixels) return PURRR_FILE_SYSTEM_ERROR;

  Purrr_Result result = PURRR_SUCCESS;
  if ((result = purrr_create_image(context, (Purrr_Image_Create_Info){
    .usage = PURRR_IMAGE_USAGE_FLAG_TEXTURE,
    .format = PURRR_R8G8B8A8_UNORM,
    .width = *width,
    .height = *height,
    .pixels = pixels,
    .sampler = sampler
  }, image)) < PURRR_SUCCESS) return result;

  stbi_image_free(pixels);

  return PURRR_SUCCESS;
}