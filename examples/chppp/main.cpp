#include <purrrpp/purrr.hpp>

using namespace purrr;

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define WINDOW_COUNT 1

struct Vertex {
  float x, y;
  float u, v;
};

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

using Index = uint32_t;

static Index sIndices[] = {
  0, 2, 1,
  2, 0, 3
};

struct UBO {
  float n;
};

static Image loadImage(const std::string &filepath, Context &context, const Sampler &sampler, int &width, int &height);

int main(void) {
  try {
    Context context(Backend::Vulkan, Context::CreateInfo{
      "purrr chp example",
      Version(1, 0, 0),
      "purrr",
      Version::currentVersion()
    });

    Buffer vertexBuffer = context.createBuffer(Buffer::CreateInfo{
      Buffer::Type::Vertex,
      sizeof(sVertices)
    });

    vertexBuffer.copyData(sVertices, sizeof(sVertices), 0);

    Buffer indexBuffer = context.createBuffer(Buffer::CreateInfo{
      Buffer::Type::Index,
      sizeof(sIndices)
    });

    indexBuffer.copyData(sIndices, sizeof(sIndices), 0);

    Sampler sampler = context.createSampler(Sampler::CreateInfo{
      Sampler::Filter::Linear,
      Sampler::Filter::Linear,
      Sampler::Filter::Linear,
      Sampler::AddressMode::Repeat,
      Sampler::AddressMode::Repeat,
      Sampler::AddressMode::Repeat,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      Sampler::BorderColor::FloatTransparentBlack
    });

    int width = 0, height = 0;

    Image image = loadImage("./assets/chp.png", context, sampler, width, height);

    Renderer renderer = context.createRenderer(Renderer::CreateInfo{
      0
    });

    std::array<Window, WINDOW_COUNT> windows{};
    std::array<Program, WINDOW_COUNT> programs{};
    std::array<Buffer, WINDOW_COUNT> ubos{};
    std::array<float, WINDOW_COUNT> ns{ 0.0f };

    {
      for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
        std::stringstream ss{};
        ss << "Window " << (i+1);

        windows[i] = renderer.createWindow(Window::CreateInfo{
          ss.str().c_str(),
          (uint32_t)width,
          (uint32_t)height,
          false
        });

        windows[i].setKeyCallback([](const Window &window, int16_t scancode, Key key, KeyAction action, KeyModifiers modifiers) {
          std::cout << "Key: " << (uint32_t)key << ", action: " << (uint32_t)action << '\n';
        });

        Program::ShaderInfo shaders[] = {
          Program::ShaderInfo{
            Program::ShaderType::Vertex, 0, nullptr, "./assets/chp.vert.spv"
          },
          Program::ShaderInfo{
            Program::ShaderType::Fragment, 0, nullptr, "./assets/chp.frag.spv"
          }
        };

        Program::VertexAttributeInfo attributes[] = {
          Program::VertexAttributeInfo{
            Format::R32G32Sfloat, sizeof(float)*0
          },
          Program::VertexAttributeInfo{
            Format::R32G32Sfloat, sizeof(float)*2
          }
        };

        Program::VertexBindingInfo vertexBinding = {
          sizeof(Vertex),
          2, attributes
        };

        Program::Binding bindings[] = {
          Program::Binding::Image, Program::Binding::UniformBuffer
        };

        programs[i] = windows[i].createProgram(Program::CreateInfo{
          2, shaders,
          1, &vertexBinding,
          2, bindings,
          false,
          Program::DepthCompareOp::Never
        });

        ubos[i] = context.createBuffer(Buffer::CreateInfo{
          Buffer::Type::Uniform,
          sizeof(UBO)
        });
      }
    }

    while (true) {
      Window::poll();

      renderer.wait();

      bool close = true;

      while (1) {
        close = true;

        for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
          if (!windows[i]) continue;

          if (windows[i].isKeyDown(Key::Escape)) {
            windows[i].~Window();
            continue;
          }

          // purrr_should_window_close returns PURRR_INVALID_ARGS_ERROR (-2)
          // if the window passed to it is invalid, otherwise it returns
          // either PURRR_SUCCESS (0) or PURRR_TRUE (1) depending on if the
          // window should close or not.
          if (!windows[i].shouldClose()) close = false;
          else {
            windows[i].~Window();
          }
        }

        if (close) return 0;

        if (!renderer.begin()) Window::wait();
        else break;
      }

      for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
        if (!windows[i]) continue;

        if (windows[i].isKeyDown(Key::Left)) ns[i] -= 0.0001f;
        if (windows[i].isKeyDown(Key::Right)) ns[i] += 0.0001f;

        if (ns[i] > 1.0f) ns[i] = 0.0f;
        else if (ns[i] < 0.0f) ns[i] = 1.0f;

        ubos[i].copyData(&ns[i], sizeof(float), 0);
      }

      renderer.bind(vertexBuffer, 0);
      renderer.bind(indexBuffer);

      for (uint32_t i = 0; i < WINDOW_COUNT; ++i) {
        if (!windows[i]) continue;

        // window is marked as a render target, therefore it's fine to pass it here
        if (renderer.begin(windows[i], Color(0x181818FF))) {
          renderer.bind(programs[i]);

          renderer.bind(image, 0);
          renderer.bind(ubos[i], 1);

          renderer.drawIndexed(sizeof(sIndices) / sizeof(*sIndices));

          renderer.end();
        }
      }

      renderer.render();
    }

    renderer.wait();
  } catch (const ResultException &result) {
    std::cerr << "Caught error: \"" << result.result().toCstr() << "\"!";
    std::cerr.flush();

    return 1;
  }

  return 0;
}

static Image loadImage(const std::string &filepath, Context &context, const Sampler &sampler, int &width, int &height) {
  stbi_uc *pixels = stbi_load(filepath.c_str(), &width, &height, NULL, STBI_rgb_alpha);
  if (!pixels) throw Result::FileSystemError;

  Image image = context.createImage(Image::CreateInfo{
    Image::Type::Texture,
    Format::R8G8B8A8Unorm,
    (uint32_t)width, (uint32_t)height,
    pixels,
    sampler
  });

  stbi_image_free(pixels);

  return image;
}