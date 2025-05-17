#ifndef   _PURRR_SAMPLER_HPP_
#define   _PURRR_SAMPLER_HPP_

namespace purrr {

class Sampler : public Wrapper<Purrr_Sampler> {
  friend class Context;
public:
  enum class Filter {
    Nearest = PURRR_SAMPLER_FILTER_NEAREST,
    Linear  = PURRR_SAMPLER_FILTER_LINEAR
  };

  enum class AddressMode {
    Repeat        = PURRR_SAMPLER_ADDRESS_MODE_REPEAT,
    Mirror        = PURRR_SAMPLER_ADDRESS_MODE_MIRROR,
    ClampToEdge   = PURRR_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    ClampToBorder = PURRR_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
  };

  enum class BorderColor {
    FloatTransparentBlack = PURRR_SAMPLER_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
    IntTransparentBlack   = PURRR_SAMPLER_BORDER_COLOR_INT_TRANSPARENT_BLACK,
    FloatOpaqueBlack      = PURRR_SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
    IntOpaqueBlack        = PURRR_SAMPLER_BORDER_COLOR_INT_OPAQUE_BLACK,
    FloatOpaqueWhite      = PURRR_SAMPLER_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
    IntOpaqueWhite        = PURRR_SAMPLER_BORDER_COLOR_INT_OPAQUE_WHITE
  };

  struct CreateInfo {
    Filter magFilter;
    Filter minFilter;
    Filter mipFilter;
    AddressMode addressModeU;
    AddressMode addressModeV;
    AddressMode addressModeW;
    float mipLodBias;
    float maxAnisotropy;
    float minLod;
    float maxLod;
    BorderColor borderColor;
  };
private:
  inline Sampler(Purrr_Context context, const CreateInfo &createInfo)
  {
    Result result = purrr_create_sampler(context, Purrr_Sampler_Create_Info{
      (Purrr_Sampler_Filter)createInfo.magFilter,
      (Purrr_Sampler_Filter)createInfo.minFilter,
      (Purrr_Sampler_Filter)createInfo.mipFilter,
      (Purrr_Sampler_Address_Mode)createInfo.addressModeU,
      (Purrr_Sampler_Address_Mode)createInfo.addressModeV,
      (Purrr_Sampler_Address_Mode)createInfo.addressModeW,
      createInfo.mipLodBias,
      createInfo.maxAnisotropy,
      createInfo.minLod,
      createInfo.maxLod,
      (Purrr_Sampler_Border_Color)createInfo.borderColor,
    }, &mHandle);
    if (!result) throw ResultException(result);
  }
public:
  Sampler() = default;

  inline Sampler(Purrr_Sampler sampler)
    : Wrapper<Purrr_Sampler>(sampler)
  {}
private:
  virtual void destroy() override {
    purrr_destroy_sampler(mHandle);
  }
};

}

#endif // _PURRR_SAMPLER_HPP_