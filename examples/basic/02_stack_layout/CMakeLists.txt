cmake_minimum_required(VERSION 3.20)

add_executable(02_stack_layout main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

target_link_libraries(02_stack_layout PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(02_stack_layout PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(02_stack_layout PRIVATE cxx_std_20)

target_compile_options(02_stack_layout PRIVATE -Wno-unused-function)

add_dependencies(02_stack_layout format-example-sources)

set_target_properties(02_stack_layout PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/basic/02_stack_layout"
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
