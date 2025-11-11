cmake_minimum_required(VERSION 3.20)

add_executable(04_performance main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

target_link_libraries(04_performance PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(04_performance PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(04_performance PRIVATE cxx_std_20)
target_compile_options(04_performance PRIVATE -Wno-unused-function)
add_dependencies(04_performance format-example-sources)

set_target_properties(04_performance PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/advanced/04_performance"
)
