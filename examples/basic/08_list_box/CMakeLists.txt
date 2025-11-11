cmake_minimum_required(VERSION 3.20)

add_executable(08_list_box main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

target_link_libraries(08_list_box PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(08_list_box PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(08_list_box PRIVATE cxx_std_20)

target_compile_options(08_list_box PRIVATE -Wno-unused-function)

add_dependencies(08_list_box format-example-sources)

set_target_properties(08_list_box PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/basic/08_list_box"
)
