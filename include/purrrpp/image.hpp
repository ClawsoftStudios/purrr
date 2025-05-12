#ifndef   _PURRR_IMAGE_HPP_
#define   _PURRR_IMAGE_HPP_

namespace purrr {

class Image : public Wrapper<Purrr_Image> {
  friend class Context;
public:
  enum class Type {
    Texture = PURRR_IMAGE_TEXTURE,
    // ColorAttachment = PURRR_IMAGE_COLOR_ATTACHMENT,
    // DepthAttachment = PURRR_IMAGE_DEPTH_ATTACHMENT
  };

  struct CreateInfo {
    Type type;
    Format format;
    uint32_t width, height;
    void *pixels;
    const Sampler &sampler;
  };
private:
  inline Image(Purrr_Context context, const CreateInfo &createInfo)
  {
    Result result = purrr_create_image(context, (Purrr_Image_Create_Info){
      (Purrr_Image_Type)createInfo.type,
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
private:
  virtual void destroy() override {
    purrr_destroy_image(mHandle);
  }
};

}

#endif // _PURRR_IMAGE_HPP_