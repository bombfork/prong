cmake_minimum_required(VERSION 3.20)

add_executable(06_flow_layout main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

target_link_libraries(06_flow_layout PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(06_flow_layout PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(06_flow_layout PRIVATE cxx_std_20)

target_compile_options(06_flow_layout PRIVATE -Wno-unused-function)

add_dependencies(06_flow_layout format-example-sources)

set_target_properties(06_flow_layout PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/basic/06_flow_layout"
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
