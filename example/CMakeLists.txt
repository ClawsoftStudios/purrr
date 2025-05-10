add_executable(example main.c)
target_link_libraries(example purrr)

add_custom_target(copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
  COMMENT "Copying assets"
)

file(GLOB_RECURSE GLSL_SHADERS "${CMAKE_CURRENT_BINARY_DIR}/assets/*.vert" "${CMAKE_CURRENT_BINARY_DIR}/assets/*.frag")

find_program(GLSLANG_VALIDATOR glslangValidator)
if(NOT GLSLANG_VALIDATOR)
  message(FATAL_ERROR "glslangValidator not found! Please install it and ensure it's in your PATH.")
endif()

set(SPIRV_OUTPUTS "")

foreach(GLSL_FILE IN LISTS GLSL_SHADERS)
  get_filename_component(FILE_NAME ${GLSL_FILE} NAME_WE)
  set(SPV_FILE "${CMAKE_CURRENT_BINARY_DIR}/assets/${FILE_NAME}.spv")

  add_custom_command(
    OUTPUT ${SPV_FILE}
    COMMAND ${GLSLANG_VALIDATOR} -V ${GLSL_FILE} -o ${SPV_FILE}
    DEPENDS ${GLSL_FILE}
    COMMENT "Compiling ${GLSL_FILE} to ${SPV_FILE}"
  )

  list(APPEND SPIRV_OUTPUTS ${SPV_FILE})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_OUTPUTS})