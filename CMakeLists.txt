cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)

project(webgpu-demo)

add_subdirectory(vendor/webgpu)

if (NOT EMSCRIPTEN)
    add_subdirectory(vendor/glfw)
else()
    # Emscripten has built-in support for GLFW but requires the `-sUSE_GLFW=3` link option:
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

add_subdirectory(vendor/glfw3webgpu)

add_subdirectory(vendor/imgui)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable("${CMAKE_PROJECT_NAME}")

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )

if (EMSCRIPTEN)
    target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -sASYNCIFY)
    set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES SUFFIX ".html")
endif()

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE webgpu glfw glfw3webgpu imgui)

target_copy_webgpu_binaries("${CMAKE_PROJECT_NAME}")

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/" 
    "${CMAKE_BINARY_DIR}/assets/"
    COMMENT "Copying assets"
)
add_dependencies(${CMAKE_PROJECT_NAME} copy_assets)