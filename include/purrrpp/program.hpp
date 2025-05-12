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

  struct CreateInfo {
    uint32_t shaderCount;
    const ShaderInfo *shaders;
    uint32_t vertexBindingCount;
    const VertexBindingInfo *vertexBindings;
    uint32_t bindingCount;
    const Binding *bindings;
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
      bindings
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
private:
  virtual void destroy() override {
    purrr_destroy_program(mHandle);
  }
};

}

#endif // _PURRR_PROGRAM_HPP_