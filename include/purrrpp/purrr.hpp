#ifndef   _PURRR_HPP_
#define   _PURRR_HPP_

#include <purrr/purrr.h>

#include <iostream>

namespace purrr {

template <typename T>
class Wrapper {
public:
  Wrapper() = default;

  inline ~Wrapper() {
    if (mHandle) destroy();
    mHandle = PURRR_NULL_HANDLE;
  }

  inline Wrapper(T handle)
    : mHandle(handle)
  {}

  Wrapper(const Wrapper<T> &other) = delete;

  inline Wrapper(Wrapper<T> &&other)
    : mHandle(std::move(other.mHandle))
  {
    other.mHandle = PURRR_NULL_HANDLE;
  }

  Wrapper<T> &operator=(const Wrapper<T> &other) = delete;

  inline Wrapper<T> &operator=(Wrapper<T> &&other) {
    if (this == &other) return *this;

    mHandle = std::move(other.mHandle);
    other.mHandle = PURRR_NULL_HANDLE;

    return *this;
  }

  virtual void destroy() {}

  inline operator bool() { return mHandle != PURRR_NULL_HANDLE; }

  inline operator T() { return mHandle; }
  inline operator const T() const { return mHandle; }
protected:
  T mHandle = PURRR_NULL_HANDLE;
};

}

#include "result.hpp"
#include "format.hpp"
#include "buffer.hpp"
#include "sampler.hpp"
#include "image.hpp"
#include "program.hpp"
#include "window.hpp"
#include "renderTarget.hpp"
#include "renderer.hpp"
#include "context.hpp"

#endif // _PURRR_HPP_