#ifndef   _PURRR_IMAGE_HPP_
#define   _PURRR_IMAGE_HPP_

namespace purrr {

class Image : public Wrapper<Purrr_Image> {
  friend class Context;
public:
  enum class UsageFlagBits {
    Texture = PURRR_IMAGE_USAGE_FLAG_TEXTURE,
    Attachment = PURRR_IMAGE_USAGE_FLAG_ATTACHMENT
  };

  using UsageFlags = Purrr_Image_Usage_Flags;

  struct CreateInfo {
    UsageFlags usage;
    Format format;
    uint32_t width, height;
    void *pixels;
    const Sampler &sampler;
  };
private:
  inline Image(Purrr_Context context, const CreateInfo &createInfo)
  {
    Result result = purrr_create_image(context, (Purrr_Image_Create_Info){
      createInfo.usage,
      (Purrr_Format)createInfo.format,
      createInfo.width,
      createInfo.height,
      createInfo.pixels,
      (Purrr_Sampler)createInfo.sampler
    }, &mHandle);
    if (!result) throw ResultException(result);
  }
public:
  Image() = default;

  inline Image(Purrr_Image image)
    : Wrapper<Purrr_Image>(image)
  {}
public:
  virtual void destroy() override {
    purrr_destroy_image(mHandle);
  }
};

}

#endif // _PURRR_IMAGE_HPP_