#ifndef   _PURRR_PROGRAM_H_
#define   _PURRR_PROGRAM_H_

typedef struct Purrr_Program_T *Purrr_Program;

typedef enum Purrr_Program_Shader_Type {
  PURRR_PROGRAM_SHADER_VERTEX = 0,
  PURRR_PROGRAM_SHADER_FRAGMENT,

  COUNT_PURRR_PROGRAM_SHADER_TYPES
} Purrr_Program_Shader_Type;

typedef struct Purrr_Program_Shader_Info {
  Purrr_Program_Shader_Type type;
  uint32_t codeLength;
  const char *code;
  const char *filepath; // Used only if code is not present
} Purrr_Program_Shader_Info;

typedef struct Purrr_Program_Vertex_Attribute_Info {
  Purrr_Format format;
  uint32_t offset;
} Purrr_Program_Vertex_Attribute_Info;

typedef struct Purrr_Program_Vertex_Binding_Info {
  uint32_t stride;
  uint32_t attributeCount;
  const Purrr_Program_Vertex_Attribute_Info *attributes;
} Purrr_Program_Vertex_Binding_Info;

typedef enum Purrr_Program_Binding_Type {
  PURRR_PROGRAM_BINDING_IMAGE = 0,
  PURRR_PROGRAM_BINDING_UNIFORM_BUFFER,

  COUNT_PURRR_PROGRAM_BINDING_TYPES
} Purrr_Program_Binding_Type;

typedef enum Purrr_Program_Depth_Compare_Op {
  PURRR_PROGRAM_DEPTH_COMPARE_NEVER = 0,
  PURRR_PROGRAM_DEPTH_COMPARE_LESS,
  PURRR_PROGRAM_DEPTH_COMPARE_EQUAL,
  PURRR_PROGRAM_DEPTH_COMPARE_LESS_OR_EQUAL,
  PURRR_PROGRAM_DEPTH_COMPARE_GREATER,
  PURRR_PROGRAM_DEPTH_COMPARE_NOT_EQUAL,
  PURRR_PROGRAM_DEPTH_COMPARE_GREATER_OR_EQUAL,
  PURRR_PROGRAM_DEPTH_COMPARE_ALWAYS,

  COUNT_PURRR_PROGRAM_DEPTH_COMPARE_OPS
} Purrr_Program_Depth_Compare_Op;

typedef struct Purrr_Program_Create_Info {
  uint32_t shaderCount;
  const Purrr_Program_Shader_Info *shaders;
  uint32_t vertexBindingCount;
  const Purrr_Program_Vertex_Binding_Info *vertexBindings;
  uint32_t bindingCount;
  const Purrr_Program_Binding_Type *bindings;
  bool enableDepth;
  Purrr_Program_Depth_Compare_Op compareOp;
} Purrr_Program_Create_Info;

Purrr_Result purrr_create_program(Purrr_Handle renderTarget, Purrr_Program_Create_Info createInfo, Purrr_Program *program);
Purrr_Result purrr_destroy_program(Purrr_Program program);

#endif // _PURRR_PROGRAM_H_