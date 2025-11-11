cmake_minimum_required(VERSION 3.20)

add_executable(07_text_input main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

target_link_libraries(07_text_input PRIVATE
    bombfork::prong
    glfw_adapters
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(07_text_input PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(07_text_input PRIVATE cxx_std_20)

target_compile_options(07_text_input PRIVATE -Wno-unused-function)

add_dependencies(07_text_input format-example-sources)

set_target_properties(07_text_input PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/basic/07_text_input"
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
