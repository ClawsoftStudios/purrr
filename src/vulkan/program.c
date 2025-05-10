#include "./program.h"

#include "./format.h"

#include <stdio.h>

#include "window.h"

static VkShaderStageFlagBits shader_type_to_stage(Purrr_Program_Shader_Type type);
static Purrr_Result read_file(const char *filepath, char **code, uint32_t *codeLength);

Purrr_Result _purrr_create_program_vulkan(Purrr_Handle renderTarget, Purrr_Program_Create_Info createInfo, _Purrr_Program_Vulkan **program) {
  if (!renderTarget || !program) return PURRR_INVALID_ARGS_ERROR;
  if ((_purrr_get_header(renderTarget).flags & _PURRR_OBJECT_FLAG_RENDER_TARGET) != _PURRR_OBJECT_FLAG_RENDER_TARGET) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Program_Vulkan *prog = _purrr_malloc_with_header((_Purrr_Object_Header){
    .backend = PURRR_VULKAN,
    .type = _PURRR_OBJECT_PROGRAM,
    .flags = 0
  }, sizeof(*prog));
  if (!prog) return PURRR_BUY_MORE_RAM;
  memset(prog, 0, sizeof(*prog));

  prog->renderTarget = renderTarget;

  VkRenderPass renderPass = VK_NULL_HANDLE;
  switch (_purrr_get_header(renderTarget).type) {
  case _PURRR_OBJECT_WINDOW: {
    renderPass = ((_Purrr_Window_Vulkan*)((Purrr_Window)renderTarget)->backendData)->renderPass;
    prog->context = ((_Purrr_Window_Vulkan*)((Purrr_Window)renderTarget)->backendData)->renderer->context;
  } break;
  case _PURRR_OBJECT_CONTEXT:
  case _PURRR_OBJECT_RENDERER:
  case _PURRR_OBJECT_BUFFER:
  case _PURRR_OBJECT_IMAGE:
  case _PURRR_OBJECT_RENDER_TARGET:
  case _PURRR_OBJECT_PROGRAM:
  case _COUNT_PURRR_OBJECT_TYPES:
  default: return PURRR_INVALID_ARGS_ERROR;
  }

  VkVertexInputBindingDescription *bindings = malloc(sizeof(*bindings) * createInfo.vertexBindingCount);
  if (!bindings) {
    _purrr_destroy_program_vulkan(prog);
    return PURRR_BUY_MORE_RAM;
  }

  uint32_t attributeCount = 0;
  VkVertexInputAttributeDescription *attributes = NULL;

  for (uint32_t i = 0; i < createInfo.vertexBindingCount; ++i) {
    Purrr_Program_Vertex_Binding_Info binding = createInfo.vertexBindings[i];

    bindings[i].binding = i;
    bindings[i].stride = binding.stride;
    bindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributes = realloc(attributes, sizeof(*attributes) * (attributeCount + binding.attributeCount));
    if (!attributes) {
      free(bindings);
      _purrr_destroy_program_vulkan(prog);
      return PURRR_BUY_MORE_RAM;
    }

    for (uint32_t j = 0; j < binding.attributeCount; ++j) {
      Purrr_Program_Vertex_Attribute_Info attribute = binding.attributes[j];

      attributes[attributeCount++] = (VkVertexInputAttributeDescription){
        .binding = i,
        .location = j,
        .format = _purrr_format_to_vk_format(attribute.format),
        .offset = attribute.offset
      };
    }
  }

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .vertexBindingDescriptionCount = createInfo.vertexBindingCount,
    .pVertexBindingDescriptions = bindings,
    .vertexAttributeDescriptionCount = attributeCount,
    .pVertexAttributeDescriptions = attributes
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  VkPipelineViewportStateCreateInfo viewportState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .viewportCount = 1,
    .pViewports = VK_NULL_HANDLE,
    .scissorCount = 1,
    .pScissors = VK_NULL_HANDLE
  };

  VkPipelineRasterizationStateCreateInfo rasterizer = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .lineWidth = 1.0f,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE
  };

  VkPipelineMultisampleStateCreateInfo multisampling = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 0,
    .pSampleMask = VK_NULL_HANDLE,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE
  };

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {
    .blendEnable = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
  };

  VkPipelineColorBlendStateCreateInfo colorBlending = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &colorBlendAttachment,
    .blendConstants[0] = 0.0f,
    .blendConstants[1] = 0.0f,
    .blendConstants[2] = 0.0f,
    .blendConstants[3] = 0.0f
  };

  VkPipelineDynamicStateCreateInfo dynamicState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .dynamicStateCount = 2,
    .pDynamicStates = (VkDynamicState[]){
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    }
  };

  VkDescriptorSetLayout *layouts = malloc(sizeof(*layouts) * createInfo.bindingCount);
  if (!layouts) {
    free(bindings);
    free(attributes);
    _purrr_destroy_program_vulkan(prog);
    return PURRR_INTERNAL_ERROR;
  }

  for (uint32_t i = 0; i < createInfo.bindingCount; ++i) {
    switch (createInfo.bindings[i]) {
    case PURRR_PROGRAM_BINDING_IMAGE: {
      layouts[i] = prog->context->textureLayout;
    } break;
    case PURRR_PROGRAM_BINDING_UNIFORM_BUFFER: {
      layouts[i] = prog->context->uniformBufferLayout;
    } break;
    case COUNT_PURRR_PROGRAM_BINDING_TYPES:
    default: {
      free(bindings);
      free(attributes);
      free(layouts);
      _purrr_destroy_program_vulkan(prog);
      return PURRR_INVALID_ARGS_ERROR;
    }
    }
  }

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .setLayoutCount = createInfo.bindingCount,
    .pSetLayouts = layouts,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges = VK_NULL_HANDLE
  };

  if (vkCreatePipelineLayout(prog->context->device, &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &prog->layout) != VK_SUCCESS) {
    free(bindings);
    free(attributes);
    free(layouts);
    _purrr_destroy_program_vulkan(prog);
    return PURRR_INTERNAL_ERROR;
  }

  free(layouts);

  VkPipelineShaderStageCreateInfo *stages = malloc(sizeof(*stages) * createInfo.shaderCount);
  if (!stages) {
    free(bindings);
    free(attributes);
    _purrr_destroy_program_vulkan(prog);
    return PURRR_BUY_MORE_RAM;
  }

  for (uint32_t i = 0; i < createInfo.shaderCount; ++i) {
    stages[i] = (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .stage = shader_type_to_stage(createInfo.shaders[i].type),
      .module = VK_NULL_HANDLE,
      .pName = "main",
      .pSpecializationInfo = VK_NULL_HANDLE
    };

    char *code = (char*)createInfo.shaders[i].code;
    uint32_t codeLength = createInfo.shaders[i].codeLength;
    if (!code) {
      if (!createInfo.shaders[i].filepath) {
        free(bindings);
        free(attributes);
        free(stages);
        _purrr_destroy_program_vulkan(prog);
        return PURRR_INVALID_ARGS_ERROR;
      }

      Purrr_Result result = PURRR_SUCCESS;
      if ((result = read_file(createInfo.shaders[i].filepath, &code, &codeLength)) < PURRR_SUCCESS) {
        free(bindings);
        free(attributes);
        free(stages);
        _purrr_destroy_program_vulkan(prog);
        return result;
      }
    }

    VkShaderModuleCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .codeSize = codeLength,
      .pCode = (uint32_t*)code
    };

    if (vkCreateShaderModule(prog->context->device, &createInfo, VK_NULL_HANDLE, &stages[i].module) != VK_SUCCESS) {
      free(bindings);
      free(attributes);
      free(stages);
      _purrr_destroy_program_vulkan(prog);
      return PURRR_INTERNAL_ERROR;
    }
  }

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .stageCount = createInfo.shaderCount,
    .pStages = stages,
    .pVertexInputState = &vertexInputInfo,
    .pInputAssemblyState = &inputAssembly,
    .pTessellationState = VK_NULL_HANDLE,
    .pViewportState = &viewportState,
    .pRasterizationState = &rasterizer,
    .pMultisampleState = &multisampling,
    .pDepthStencilState = VK_NULL_HANDLE,
    .pColorBlendState = &colorBlending,
    .pDynamicState = &dynamicState,
    .layout = prog->layout,
    .renderPass = renderPass,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = 0
  };

  if (vkCreateGraphicsPipelines(prog->context->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &prog->pipeline) != VK_SUCCESS) {
    _purrr_destroy_program_vulkan(prog);
    return PURRR_INTERNAL_ERROR;
  }

  free(attributes);
  free(bindings);

  for (uint32_t i = 0; i < createInfo.shaderCount; ++i) vkDestroyShaderModule(prog->context->device, stages[i].module, VK_NULL_HANDLE);
  free(stages);

  *program = prog;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_program_vulkan(_Purrr_Program_Vulkan *program) {
  if (!program) return PURRR_INVALID_ARGS_ERROR;

  if (program->layout) vkDestroyPipelineLayout(program->context->device, program->layout, VK_NULL_HANDLE);
  if (program->pipeline) vkDestroyPipeline(program->context->device, program->pipeline, VK_NULL_HANDLE);

  return PURRR_SUCCESS;
}



static VkShaderStageFlagBits shader_type_to_stage(Purrr_Program_Shader_Type type) {
  switch (type) {
  case PURRR_PROGRAM_SHADER_VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
  case PURRR_PROGRAM_SHADER_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
  case COUNT_PURRR_PROGRAM_SHADER_TYPES:
  default: return 0;
  }
}

static Purrr_Result read_file(const char *filepath, char **code, uint32_t *codeLength) {
  if (!filepath || !code || !codeLength) return PURRR_INVALID_ARGS_ERROR;

  FILE *fd = fopen(filepath, "rb");
  if (!fd) return PURRR_FILE_SYSTEM_ERROR;

  fseek(fd, 0, SEEK_END);
  *codeLength = (uint32_t)ftell(fd);
  fseek(fd, 0, SEEK_SET);

  *code = malloc(*codeLength);
  if (!*code) {
    fclose(fd);
    return PURRR_BUY_MORE_RAM;
  }

  if (!fread(*code, *codeLength, 1, fd)) {
    fclose(fd);
    return PURRR_FILE_SYSTEM_ERROR;
  }

  fclose(fd);

  return PURRR_SUCCESS;
}