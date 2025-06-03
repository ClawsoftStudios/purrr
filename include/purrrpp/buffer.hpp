#ifndef   _PURRR_BUFFER_HPP_
#define   _PURRR_BUFFER_HPP_

namespace purrr {

class Buffer : public Wrapper<Purrr_Buffer> {
  friend class Context;
public:
  enum class Type {
    Vertex = PURRR_BUFFER_VERTEX,
    Index = PURRR_BUFFER_INDEX,
    Uniform = PURRR_BUFFER_UNIFORM
  };

  struct CreateInfo {
    Type type;
    uint32_t size;
  };
private:
  inline Buffer(Purrr_Context context, const CreateInfo &createInfo)
  {
    Result result = purrr_create_buffer(context, Purrr_Buffer_Create_Info{
      (Purrr_Buffer_Type)createInfo.type,
      createInfo.size
    }, &mHandle);
    if (!result) throw ResultException(result);
  }
public:
  Buffer() = default;

  inline Buffer(Purrr_Buffer buffer)
    : Wrapper<Purrr_Buffer>(buffer)
  {}

  inline void copyData(void *data, uint32_t size, uint32_t offset) {
    Result result = purrr_copy_buffer_data(mHandle, data, size, offset);
    if (!result) throw ResultException(result);
  }
public:
  virtual void destroy() override {
    purrr_destroy_buffer(mHandle);
  }
};

}

#endif // _PURRR_BUFFER_HPP_