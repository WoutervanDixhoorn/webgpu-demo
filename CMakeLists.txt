cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)


project(webgpu-demo)


add_subdirectory(vendor/webgpu)

if (NOT EMSCRIPTEN)
    add_subdirectory(vendor/glfw)
else()
    # Emscripten has built-in support for GLFW but requires the `-sUSE_GLFW=3` link option:
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()


file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")


add_executable("${CMAKE_PROJECT_NAME}")


target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )

if (EMSCRIPTEN)
    target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -sASYNCIFY)
    set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES SUFFIX ".html")
endif()

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE webgpu glfw)

target_copy_webgpu_binaries("${CMAKE_PROJECT_NAME}")