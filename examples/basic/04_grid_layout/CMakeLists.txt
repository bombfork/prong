cmake_minimum_required(VERSION 3.20)

add_executable(04_grid_layout main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

target_link_libraries(04_grid_layout PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(04_grid_layout PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(04_grid_layout PRIVATE cxx_std_20)

target_compile_options(04_grid_layout PRIVATE -Wno-unused-function)

add_dependencies(04_grid_layout format-example-sources)

set_target_properties(04_grid_layout PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/basic/04_grid_layout"
)
