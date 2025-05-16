#ifndef   _PURRR_RENDER_TARGET_HPP_
#define   _PURRR_RENDER_TARGET_HPP_

namespace purrr {

class RenderTarget : public Wrapper<Purrr_Render_Target> {
  friend class Context;
public:
  struct CreateInfo {
    uint32_t imageCount;
    Image *images;
    uint32_t depthIndex;

    uint32_t width, height;
  };
private:
  inline RenderTarget(Purrr_Context context, const CreateInfo &createInfo)
  {
    if (!createInfo.imageCount || !createInfo.images) throw ResultException(Result::InvalidArgsError);

    Purrr_Image *images = new Purrr_Image[createInfo.imageCount];
    if (!images) throw ResultException(Result::BuyMoreRam);

    for (uint32_t i = 0; i < createInfo.imageCount; ++i)
      images[i] = (Purrr_Image)createInfo.images[i];

    Result result = purrr_create_render_target(context, Purrr_Render_Target_Create_Info{
      createInfo.imageCount,
      images,
      createInfo.depthIndex,
      createInfo.width,
      createInfo.height
    }, &mHandle);

    delete[] images;

    if (!result) throw ResultException(result);
  }
public:
  RenderTarget() = default;

  inline RenderTarget(Purrr_Render_Target render_Target)
    : Wrapper<Purrr_Render_Target>(render_Target)
  {}

  inline void resize(uint32_t newWidth, uint32_t newHeight) {
    Result result = purrr_resize_render_target(mHandle, newWidth, newHeight);
    if (!result) throw ResultException(result);
  }

  Image getImage(uint32_t imageIndex) {
    Purrr_Image image = PURRR_NULL_HANDLE;
    Result result = purrr_get_render_target_image(mHandle, imageIndex, &image);
    if (!result) throw ResultException(result);

    return std::move(Image(image));
  }
private:
  virtual void destroy() override {
    purrr_destroy_render_target(mHandle);
  }
};

}

#endif // _PURRR_RENDER_TARGET_HPP_