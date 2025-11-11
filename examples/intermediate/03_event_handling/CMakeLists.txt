cmake_minimum_required(VERSION 3.20)

# Create executable
add_executable(03_event_handling main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../adapters/stb_impl.cpp
)

# Link libraries
target_link_libraries(03_event_handling PRIVATE
    bombfork::prong
    glfw
    OpenGL::GL
    stb_headers
)

# Include directories
target_include_directories(03_event_handling PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# C++20 standard
target_compile_features(03_event_handling PRIVATE cxx_std_20)

# Suppress unused function warnings for STB
target_compile_options(03_event_handling PRIVATE -Wno-unused-function)

# Depend on formatting target
add_dependencies(03_event_handling format-example-sources)

# Output to build/examples/intermediate/03_event_handling/
set_target_properties(03_event_handling PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/intermediate/03_event_handling"
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
