#ifndef   _PURRR_PROGRAM_BUILDER_HPP_
#define   _PURRR_PROGRAM_BUILDER_HPP_

#include <vector>

namespace purrr {

class ProgramBuilder {
public:
  class VertexBinding {
    friend class ProgramBuilder;
  public:
    inline VertexBinding(uint32_t stride, Program::VertexInputRate inputRate = Program::VertexInputRate::Vertex)
      : mStride(stride), mInputRate(inputRate)
    {}

    inline VertexBinding &addAttribute(Format format, uint32_t offset) {
      mAttributes.push_back(Purrr_Program_Vertex_Attribute_Info{
        (Purrr_Format)format, offset
      });

      return *this;
    }
  private:
    uint32_t mStride;
    Program::VertexInputRate mInputRate;
    std::vector<Purrr_Program_Vertex_Attribute_Info> mAttributes{};
  };
public:
  ProgramBuilder() = default;
  ~ProgramBuilder() = default;

  inline ProgramBuilder &addShader(const Program::ShaderInfo &shaderInfo) {
    mShaders.push_back(Purrr_Program_Shader_Info{
      (Purrr_Program_Shader_Type)shaderInfo.type,
      shaderInfo.codeLength,
      shaderInfo.code,
      shaderInfo.filepath
    });

    return *this;
  }

  inline ProgramBuilder &addVertexBinding(VertexBinding binding) {
    VertexBinding bindingCopy = binding;

    mPurrrVertexBindings.push_back(Purrr_Program_Vertex_Binding_Info{
      bindingCopy.mStride,
      bindingCopy.mAttributes.size(),
      bindingCopy.mAttributes.data(),
      (Purrr_Program_Vertex_Input_Rate)bindingCopy.mInputRate
    });

    mVertexBindings.push_back(std::move(bindingCopy));

    return *this;
  }

  inline ProgramBuilder &addBinding(Program::Binding binding) {
    mBindings.push_back((Purrr_Program_Binding_Type)binding);

    return *this;
  }

  inline ProgramBuilder &setEnableDepth(bool enableDepth) {
    mEnableDepth = enableDepth;
    return *this;
  }

  inline ProgramBuilder &setCompareOp(Program::DepthCompareOp compareOp) {
    mCompareOp = (Purrr_Program_Depth_Compare_Op)compareOp;
    return *this;
  }

  inline ProgramBuilder &setCullMode(Program::CullMode cullMode) {
    mCullMode = (Purrr_Program_Cull_Mode)cullMode;
    return *this;
  }

  inline ProgramBuilder &setFrontFace(Program::FrontFace frontFace) {
    mFrontFace = (Purrr_Program_Front_Face)frontFace;
    return *this;
  }

  Program build(const Window &window) {
    return build((Purrr_Handle)((Purrr_Window)window));
  }

  Program build(const RenderTarget &renderTarget) {
    return build((Purrr_Handle)((Purrr_Render_Target)renderTarget));
  }
private:
  Program build(Purrr_Handle handle) {
    Purrr_Program purrrProgram = PURRR_NULL_HANDLE;
    Result result = purrr_create_program(handle, Purrr_Program_Create_Info{
      (uint32_t)mShaders.size(), mShaders.data(),
      (uint32_t)mPurrrVertexBindings.size(), mPurrrVertexBindings.data(),
      (uint32_t)mBindings.size(), mBindings.data(),
      mEnableDepth,
      mCompareOp,
      mCullMode,
      mFrontFace
    }, &purrrProgram);
    if (!result) throw ResultException(result);

    Program program(purrrProgram);
    return std::move(program);
  }
private:
  std::vector<Purrr_Program_Shader_Info> mShaders{};
  std::vector<Purrr_Program_Vertex_Binding_Info> mPurrrVertexBindings{};
  std::vector<VertexBinding> mVertexBindings{};
  std::vector<Purrr_Program_Binding_Type> mBindings{};
  bool mEnableDepth = false;
  Purrr_Program_Depth_Compare_Op mCompareOp = PURRR_PROGRAM_DEPTH_COMPARE_NEVER;
  Purrr_Program_Cull_Mode mCullMode = PURRR_PROGRAM_CULL_NONE;
  Purrr_Program_Front_Face mFrontFace = PURRR_PROGRAM_FRONT_FACE_COUNTER_CLOCKWISE;
};

}

#endif // _PURRR_PROGRAM_BUILDER_HPP_