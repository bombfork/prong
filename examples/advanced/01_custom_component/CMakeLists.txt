cmake_minimum_required(VERSION 3.20)

# Create executable
add_executable(01_custom_component main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

# Link libraries
target_link_libraries(01_custom_component PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

# Include directories
target_include_directories(01_custom_component PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# C++20 standard
target_compile_features(01_custom_component PRIVATE cxx_std_20)

# Suppress unused function warnings for STB
target_compile_options(01_custom_component PRIVATE -Wno-unused-function)

# Depend on formatting target
add_dependencies(01_custom_component format-example-sources)

# Output to build/examples/advanced/01_custom_component/
set_target_properties(01_custom_component PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/advanced/01_custom_component"
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
