cmake_minimum_required(VERSION 3.20)

# Create executable
add_executable(01_nested_panels main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

# Link libraries
target_link_libraries(01_nested_panels PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

# Include directories
target_include_directories(01_nested_panels PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# C++20 standard
target_compile_features(01_nested_panels PRIVATE cxx_std_20)

# Suppress unused function warnings for STB
target_compile_options(01_nested_panels PRIVATE -Wno-unused-function)

# Depend on formatting target
add_dependencies(01_nested_panels format-example-sources)

# Output to build/examples/intermediate/01_nested_panels/
set_target_properties(01_nested_panels PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/intermediate/01_nested_panels"
)
