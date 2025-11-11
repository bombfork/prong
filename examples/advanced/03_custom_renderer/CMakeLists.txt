cmake_minimum_required(VERSION 3.20)

add_executable(03_custom_renderer main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

target_link_libraries(03_custom_renderer PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(03_custom_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(03_custom_renderer PRIVATE cxx_std_20)
target_compile_options(03_custom_renderer PRIVATE -Wno-unused-function)
add_dependencies(03_custom_renderer format-example-sources)

set_target_properties(03_custom_renderer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/advanced/03_custom_renderer"
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
