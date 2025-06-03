#ifndef   _PURRR_PROGRAM_HPP_
#define   _PURRR_PROGRAM_HPP_

namespace purrr {

class Program : public Wrapper<Purrr_Program> {
  friend class Window;
public:
  enum class ShaderType {
    Vertex = PURRR_PROGRAM_SHADER_VERTEX,
    Fragment = PURRR_PROGRAM_SHADER_FRAGMENT
  };

  struct ShaderInfo {
    ShaderType type;
    uint32_t codeLength;
    const char *code;
    const char *filepath; // Used only if code is not present
  };

  struct VertexAttributeInfo {
    Format format;
    uint32_t offset;
  };

  struct VertexBindingInfo {
    uint32_t stride;
    uint32_t attributeCount;
    const VertexAttributeInfo *attributes;
  };

  enum class Binding {
    Image = PURRR_PROGRAM_BINDING_IMAGE,
    UniformBuffer = PURRR_PROGRAM_BINDING_UNIFORM_BUFFER
  };

  enum class DepthCompareOp {
    Never = PURRR_PROGRAM_DEPTH_COMPARE_NEVER,
    Less = PURRR_PROGRAM_DEPTH_COMPARE_LESS,
    Equal = PURRR_PROGRAM_DEPTH_COMPARE_EQUAL,
    LessOrEqual = PURRR_PROGRAM_DEPTH_COMPARE_LESS_OR_EQUAL,
    Greater = PURRR_PROGRAM_DEPTH_COMPARE_GREATER,
    NotEqual = PURRR_PROGRAM_DEPTH_COMPARE_NOT_EQUAL,
    GreaterOrEqual = PURRR_PROGRAM_DEPTH_COMPARE_GREATER_OR_EQUAL,
    Always = PURRR_PROGRAM_DEPTH_COMPARE_ALWAYS
  };

  enum class CullMode {
    None = PURRR_PROGRAM_CULL_NONE,
    Front = PURRR_PROGRAM_CULL_FRONT_BIT,
    Back = PURRR_PROGRAM_CULL_BACK_BIT,
    FrontAndBack = PURRR_PROGRAM_CULL_FRONT_AND_BACK
  };

  enum class FrontFace {
    CounterClockwise = PURRR_PROGRAM_FRONT_FACE_COUNTER_CLOCKWISE,
    Clockwise = PURRR_PROGRAM_FRONT_FACE_CLOCKWISE
  };

  struct CreateInfo {
    uint32_t shaderCount;
    const ShaderInfo *shaders;
    uint32_t vertexBindingCount;
    const VertexBindingInfo *vertexBindings;
    uint32_t bindingCount;
    const Binding *bindings;
    bool enableDepth;
    DepthCompareOp compareOp;
    CullMode cullMode;
    FrontFace frontFace;
  };
private:
  inline Program(Purrr_Handle renderTarget, const CreateInfo &createInfo)
  {
    Purrr_Program_Shader_Info *shaders = new Purrr_Program_Shader_Info[createInfo.shaderCount];
    for (uint32_t i = 0; i < createInfo.shaderCount; ++i) {
      const ShaderInfo &shader = createInfo.shaders[i];

      shaders[i] = Purrr_Program_Shader_Info{
        (Purrr_Program_Shader_Type)shader.type,
        shader.codeLength,
        shader.code,
        shader.filepath
      };
    }

    Purrr_Program_Vertex_Binding_Info *vertexBindings = new Purrr_Program_Vertex_Binding_Info[createInfo.vertexBindingCount];
    for (uint32_t i = 0; i < createInfo.vertexBindingCount; ++i) {
      const VertexBindingInfo &binding = createInfo.vertexBindings[i];

      Purrr_Program_Vertex_Attribute_Info *attributes = new Purrr_Program_Vertex_Attribute_Info[binding.attributeCount];
      for (uint32_t j = 0; j < binding.attributeCount; ++j) {
        const VertexAttributeInfo &attribute = binding.attributes[j];

        attributes[j] = Purrr_Program_Vertex_Attribute_Info{
          (Purrr_Format)attribute.format,
          attribute.offset
        };
      }

      vertexBindings[i] = Purrr_Program_Vertex_Binding_Info{
        binding.stride,
        binding.attributeCount,
        attributes
      };
    }

    Purrr_Program_Binding_Type *bindings = new Purrr_Program_Binding_Type[createInfo.bindingCount];
    for (uint32_t i = 0; i < createInfo.bindingCount; ++i)
      bindings[i] = (Purrr_Program_Binding_Type)createInfo.bindings[i];

    Result result = purrr_create_program(renderTarget, (Purrr_Program_Create_Info){
      createInfo.shaderCount,
      shaders,
      createInfo.vertexBindingCount,
      vertexBindings,
      createInfo.bindingCount,
      bindings,
      createInfo.enableDepth,
      (Purrr_Program_Depth_Compare_Op)createInfo.compareOp,
      (Purrr_Program_Cull_Mode)createInfo.cullMode,
      (Purrr_Program_Front_Face)createInfo.frontFace
    }, &mHandle);

    delete[] shaders;

    for (uint32_t i = 0; i < createInfo.vertexBindingCount; ++i) delete[] vertexBindings[i].attributes;
    delete[] vertexBindings;

    delete[] bindings;

    if (!result) throw ResultException(result);
  }
public:
  Program() = default;

  inline Program(Purrr_Program program)
    : Wrapper<Purrr_Program>(program)
  {}
public:
  virtual void destroy() override {
    purrr_destroy_program(mHandle);
  }
};

}

#endif // _PURRR_PROGRAM_HPP_