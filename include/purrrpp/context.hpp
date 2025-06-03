#ifndef   _PURRR_CONTEXT_HPP_
#define   _PURRR_CONTEXT_HPP_

namespace purrr {

enum class Backend {
  Vulkan = 0,
};

class Version {
public:
  inline static Version currentVersion() {
    return Version(0, 0, 0);
  }
public:
  inline Version(uint32_t major, uint32_t minor, uint32_t patch)
    : mVersion(((major & 0x3FF) << 22U) | ((minor & 0x3FF) << 12U) | (patch & 0xFFF))
  {}

  inline operator uint32_t() const { return mVersion; }
private:
  uint32_t mVersion;
};

class Context : public Wrapper<Purrr_Context> {
public:
  struct CreateInfo {
    const char *applicationName;
    Version applicationVersion;
    const char *engineName;
    Version engineVersion;
  };
public:
  Context() = default;

  inline Context(Purrr_Context context)
    : Wrapper<Purrr_Context>(context)
  {}

  inline Context(Backend backend, const CreateInfo &createInfo)
  {
    Result result = purrr_create_context((Purrr_Backend)backend, Purrr_Context_Create_Info{
      createInfo.applicationName,
      createInfo.applicationVersion,
      createInfo.engineName,
      createInfo.engineVersion
    }, &mHandle);
    if (!result) throw ResultException(result);
  }

  inline Buffer createBuffer(const Buffer::CreateInfo &createInfo) {
    Buffer buffer(mHandle, createInfo);
    return std::move(buffer);
  }

  inline Sampler createSampler(const Sampler::CreateInfo &createInfo) {
    Sampler sampler(mHandle, createInfo);
    return std::move(sampler);
  }

  inline Image createImage(const Image::CreateInfo &createInfo) {
    Image image(mHandle, createInfo);
    return std::move(image);
  }

  inline Renderer createRenderer(const Renderer::CreateInfo &createInfo) {
    Renderer renderer(mHandle, createInfo);
    return std::move(renderer);
  }
public:
  virtual void destroy() override {
    purrr_destroy_context(mHandle);
  }
};

}

#endif // _PURRR_CONTEXT_HPP_