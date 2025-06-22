#ifndef   _PURRR_RENDERER_HPP_
#define   _PURRR_RENDERER_HPP_

namespace purrr {

struct Color {
  Color(uint32_t hex) // hex = 0xRRGGBBAA
    : r(((hex >> (8*3))&0xFF)/255), g(((hex >> (8*2))&0xFF)/255), b(((hex >> (8*1))&0xFF)/255), a(((hex >> (8*0))&0xFF)/255)
  {}

  Color(float red, float green, float blue, float alpha)
    : r(red), g(green), b(blue), a(alpha)
  {}

  float r, g, b, a;
};

class Renderer : public Wrapper<Purrr_Renderer> {
  friend class Context;
public:
  struct CreateInfo {
    char x;
  };
private:
  inline Renderer(Purrr_Context context, const CreateInfo &createInfo)
  {
    Result result = purrr_create_renderer(context, Purrr_Renderer_Create_Info{
      createInfo.x
    }, &mHandle);
    if (!result) throw ResultException(result);
  }
public:
  Renderer() = default;

  inline Renderer(Purrr_Renderer renderer)
    : Wrapper<Purrr_Renderer>(renderer)
  {}

  inline void wait() {
    Result result = purrr_wait_renderer(mHandle);
    if (!result) throw ResultException(result);
  }

  inline bool begin() {
    Result result = purrr_begin_renderer(mHandle);
    if (!result) throw ResultException(result);
    return result != Result::Inactive;
  }

  inline bool begin(const Window &window, const Color &color) {
    Result result = purrr_renderer_begin(mHandle, (const Purrr_Window)window, (Purrr_Color){
      color.r,
      color.g,
      color.b,
      color.a
    });
    if (!result) throw ResultException(result);
    return result == Result::True;
  }

  inline void bind(const Buffer &buffer, uint32_t index = 0) {
    Result result = purrr_renderer_bind_buffer(mHandle, (const Purrr_Buffer)buffer, index);
    if (!result) throw ResultException(result);
  }

  inline void bind(const Image &image, uint32_t index = 0) {
    Result result = purrr_renderer_bind_image(mHandle, (const Purrr_Image)image, index);
    if (!result) throw ResultException(result);
  }

  inline void bind(const Program &program) {
    Result result = purrr_renderer_bind_program(mHandle, program);
    if (!result) throw ResultException(result);
  }

  inline void drawIndexed(uint32_t indexCount) {
    Result result = purrr_renderer_draw_indexed(mHandle, indexCount);
    if (!result) throw ResultException(result);
  }

  inline void drawIndexed(uint32_t indexCount, uint32_t instanceCount) {
    Result result = purrr_renderer_draw_indexed_instanced(mHandle, indexCount, instanceCount);
    if (!result) throw ResultException(result);
  }

  inline void drawIndexedIndirect(const Buffer &buffer, uint32_t drawCount, uint32_t stride) {
    Result result = purrr_renderer_draw_indexed_indirect(mHandle, buffer, drawCount, stride);
    if (!result) throw ResultException(result);
  }

  inline void end() {
    Result result = purrr_renderer_end(mHandle);
    if (!result) throw ResultException(result);
  }

  inline void render() {
    Result result = purrr_render_renderer(mHandle);
    if (!result) throw ResultException(result);
  }

  inline Window createWindow(const Window::CreateInfo &createInfo) {
    Window window(mHandle, createInfo);
    return std::move(window);
  }
public:
  virtual void destroy() override {
    purrr_destroy_renderer(mHandle);
  }
};

}

#endif // _PURRR_RENDERER_HPP_