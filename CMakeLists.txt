# Heavily inspired by GLFW's CMakeLists.txt

cmake_minimum_required(VERSION 3.23)

project(purrr)

set(PURRR_VERSION 0.1.0)

find_package(Vulkan REQUIRED)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(PURRR_LIBRARY_TYPE "${PURRR_LIBRARY_TYPE}" CACHE STRING
    "Library type override for PURRR (SHARED, STATIC, OBJECT, or empty to follow BUILD_SHARED_LIBS)")

if(PURRR_LIBRARY_TYPE)
  if (PURRR_LIBRARY_TYPE STREQUAL "SHARED")
    set(PURRR_BUILD_SHARED_LIBRARY TRUE)
  else()
    set(PURRR_BUILD_SHARED_LIBRARY FALSE)
  endif()
else()
  set(PURRR_BUILD_SHARED_LIBRARY ${BUILD_SHARED_LIBS})
endif()

file(GLOB_RECURSE PURRR_SOURCES "src/**.c" "src/**.h" "include/**.h*")
add_library(purrr ${PURRR_LIBRARY_TYPE} ${PURRR_SOURCES})

target_include_directories(purrr PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(purrr Vulkan::Vulkan)

set_property(TARGET purrr PROPERTY C_STANDARD 99)
set_property(TARGET purrr PROPERTY CXX_STANDARD 17)

target_compile_definitions(purrr
  PRIVATE
    $<$<PLATFORM_ID:Windows>:PURRR_PLATFORM_WINDOWS>
    $<$<PLATFORM_ID:Windows>:VK_USE_PLATFORM_WIN32_KHR>
    $<$<BOOL:PURRR_BUILD_SHARED_LIBRARY>:PURRR_BUILD_DLL>
  INTERFACE
    $<$<BOOL:PURRR_BUILD_SHARED_LIBRARY>:PURRR_DLL>
)

target_compile_options(purrr PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<C_COMPILER_ID:GNU,Clang,AppleClang>>:-Wall -Wextra -Werror -Wpedantic>
  $<$<AND:$<CONFIG:Debug>,$<C_COMPILER_ID:MSVC>>:/W4 /WX>
)

include(GNUInstallDirs)
set(PURRR_CONFIG_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/purrr")

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/purrrConfig.cmake.in
                              src/purrrConfig.cmake
                              INSTALL_DESTINATION "${PURRR_CONFIG_PATH}"
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(src/purrrConfigVersion.cmake
                                 VERSION ${PURRR_VERSION}
                                 COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/purrrConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/src/purrrConfigVersion.cmake"
        DESTINATION "${PURRR_CONFIG_PATH}")

install(EXPORT purrrTargets
  FILE purrrTargets.cmake
  NAMESPACE purrr::
  DESTINATION ${PURRR_CONFIG_PATH}
)

install(TARGETS purrr
        EXPORT purrrTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})