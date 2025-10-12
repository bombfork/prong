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

# === Demo App Example ===

add_executable(prong_demo_app
    demo_app/main.cpp
    adapters/stb_impl.cpp
)

target_link_libraries(prong_demo_app PRIVATE
    bombfork::prong
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
message(STATUS "")
