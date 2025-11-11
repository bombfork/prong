cmake_minimum_required(VERSION 3.20)

# Create executable
add_executable(04_dynamic_layout main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

# Link libraries
target_link_libraries(04_dynamic_layout PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

# Include directories
target_include_directories(04_dynamic_layout PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# C++20 standard
target_compile_features(04_dynamic_layout PRIVATE cxx_std_20)

# Suppress unused function warnings for STB
target_compile_options(04_dynamic_layout PRIVATE -Wno-unused-function)

# Depend on formatting target
add_dependencies(04_dynamic_layout format-example-sources)

# Output to build/examples/intermediate/04_dynamic_layout/
set_target_properties(04_dynamic_layout PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/intermediate/04_dynamic_layout"
)
