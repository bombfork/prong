cmake_minimum_required(VERSION 3.20)

# Create executable
add_executable(02_responsive_ui main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

# Link libraries
target_link_libraries(02_responsive_ui PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

# Include directories
target_include_directories(02_responsive_ui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# C++20 standard
target_compile_features(02_responsive_ui PRIVATE cxx_std_20)

# Suppress unused function warnings for STB
target_compile_options(02_responsive_ui PRIVATE -Wno-unused-function)

# Depend on formatting target
add_dependencies(02_responsive_ui format-example-sources)

# Output to build/examples/intermediate/02_responsive_ui/
set_target_properties(02_responsive_ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/intermediate/02_responsive_ui"
)
