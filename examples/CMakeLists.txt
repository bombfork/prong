# Prong Examples CMakeLists.txt

cmake_minimum_required(VERSION 3.20)

# === FetchContent for dependencies ===
include(FetchContent)

# Fetch STB (single-file public domain libraries)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# === Clang-format target for example sources ===

# Collect example source files for formatting (both .cpp and .h files)
file(GLOB_RECURSE EXAMPLE_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/**/*.h
)

# Create custom target that runs clang-format on example files
add_custom_target(format-example-sources ALL
    COMMAND ${CLANG_FORMAT_PATH} -i ${EXAMPLE_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-format on example source files..."
    VERBATIM
)

# Examples require GLFW and OpenGL
find_package(glfw3 QUIET)
find_package(OpenGL QUIET)

if(NOT glfw3_FOUND)
    message(WARNING "GLFW not found - examples will not be built")
    message(WARNING "Install GLFW: sudo apt-get install libglfw3-dev (Ubuntu/Debian)")
    return()
endif()

if(NOT OpenGL_FOUND)
    message(WARNING "OpenGL not found - examples will not be built")
    return()
endif()

message(STATUS "Building Prong examples:")
message(STATUS "  GLFW: ${glfw3_VERSION}")
message(STATUS "  OpenGL: ${OPENGL_LIBRARIES}")

# Make STB available
FetchContent_MakeAvailable(stb)

# Create interface library for STB headers
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${stb_SOURCE_DIR})

message(STATUS "  STB: ${stb_SOURCE_DIR}")

# === Common GLFW Adapters ===
# These provide reference implementations for GLFW-specific adapters

add_library(glfw_adapters STATIC
    common/glfw_adapters/glfw_keyboard.cpp
)

target_link_libraries(glfw_adapters PUBLIC
    bombfork::prong
    glfw
)

target_include_directories(glfw_adapters PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common/glfw_adapters
)

target_compile_features(glfw_adapters PUBLIC cxx_std_20)

# Enable iwyu for glfw_adapters
set_target_properties(glfw_adapters PROPERTIES
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)

# === Demo App Example ===

add_executable(prong_demo_app
    demo_app/main.cpp
    adapters/stb_impl.cpp
)

target_link_libraries(prong_demo_app PRIVATE
    bombfork::prong
    glfw_adapters
    glfw
    OpenGL::GL
    stb_headers
)

target_include_directories(prong_demo_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set C++20 standard
target_compile_features(prong_demo_app PRIVATE cxx_std_20)

# Suppress unused function warnings for STB (header-only library with many utilities)
target_compile_options(prong_demo_app PRIVATE -Wno-unused-function)

# Enable iwyu for prong_demo_app
set_target_properties(prong_demo_app PROPERTIES
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)

# Make example depend on formatting target
add_dependencies(prong_demo_app format-example-sources)

# Platform-specific settings
if(APPLE)
    target_link_libraries(prong_demo_app PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
    )
endif()

# Copy to examples directory in build
set_target_properties(prong_demo_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)

# Copy fonts folder to build directory
add_custom_command(TARGET prong_demo_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts
        ${CMAKE_BINARY_DIR}/examples/fonts
    COMMENT "Copying fonts to build directory..."
    VERBATIM
)

message(STATUS "  + prong_demo_app")

# === Basic Examples ===
# Simple, focused examples demonstrating individual concepts

add_subdirectory(basic/01_hello_button)
add_subdirectory(basic/02_stack_layout)
add_subdirectory(basic/03_flex_layout)
add_subdirectory(basic/04_grid_layout)
add_subdirectory(basic/05_dock_layout)
add_subdirectory(basic/06_flow_layout)
add_subdirectory(basic/07_text_input)
add_subdirectory(basic/08_list_box)

message(STATUS "  + basic examples (8 examples)")

# === Intermediate Examples ===
# More complex examples showing layout composition and advanced patterns

add_subdirectory(intermediate/01_nested_panels)
add_subdirectory(intermediate/02_responsive_ui)
add_subdirectory(intermediate/03_event_handling)
add_subdirectory(intermediate/04_dynamic_layout)

message(STATUS "  + intermediate examples (4 examples)")

# === Advanced Examples ===
# Advanced topics: custom components, layouts, renderers, and optimization

add_subdirectory(advanced/01_custom_component)
add_subdirectory(advanced/02_custom_layout)
add_subdirectory(advanced/03_custom_renderer)
add_subdirectory(advanced/04_performance)

message(STATUS "  + advanced examples (4 examples)")
message(STATUS "")
