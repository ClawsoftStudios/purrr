cmake_minimum_required(VERSION 3.14)

project(purrr)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE PURRR_SOURCES "src/**.c" "src/**.h" "include/**.h")
add_library(purrr STATIC ${PURRR_SOURCES})
target_include_directories(purrr PUBLIC include/)
target_link_libraries(purrr Vulkan::Vulkan)

target_compile_definitions(purrr PRIVATE
  $<$<PLATFORM_ID:Windows>:PURRR_PLATFORM_WINDOWS>
  $<$<PLATFORM_ID:Windows>:VK_USE_PLATFORM_WIN32_KHR>
)

string(COMPARE EQUAL ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} PURRR_INCLUDE_EXAMPLE)
if(${PURRR_INCLUDE_EXAMPLE})
  add_subdirectory(example/)
endif()