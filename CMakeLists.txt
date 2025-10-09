cmake_minimum_required(VERSION 3.14)
project(prong VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for language servers and tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Force static library only
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# Build options
option(PRONG_BUILD_EXAMPLES "Build example applications" ON)
option(PRONG_BUILD_TESTS "Build unit tests" ON)

# === Mandatory include-what-you-use ===
find_program(IWYU_PATH NAMES include-what-you-use iwyu REQUIRED)
if(NOT IWYU_PATH)
    message(FATAL_ERROR "include-what-you-use is required but not found. Install it with: sudo apt install iwyu (Ubuntu/Debian) or brew install include-what-you-use (macOS)")
endif()
message(STATUS "Found include-what-you-use: ${IWYU_PATH}")

# === Mandatory clang-format ===
find_program(CLANG_FORMAT_PATH NAMES clang-format REQUIRED)
if(NOT CLANG_FORMAT_PATH)
    message(FATAL_ERROR "clang-format is required but not found. Install it with: sudo apt install clang-format (Ubuntu/Debian) or brew install clang-format (macOS)")
endif()
message(STATUS "Found clang-format: ${CLANG_FORMAT_PATH}")

# === Compiler warnings (all warnings as errors) ===
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# === Clang-format target for library sources ===

# Collect library source files for formatting (headers + implementation)
file(GLOB_RECURSE PRONG_LIBRARY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h
)
file(GLOB_RECURSE PRONG_LIBRARY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
)

# Create custom target that runs clang-format on library files
add_custom_target(format-library-sources ALL
    COMMAND ${CLANG_FORMAT_PATH} -i ${PRONG_LIBRARY_HEADERS} ${PRONG_LIBRARY_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-format on library source files..."
    VERBATIM
)

# === Prong Library ===

# Source files (implementation files only - headers are interface)
set(PRONG_SOURCES
    src/core/coordinate_system.cpp
    src/core/async_callback_queue.cpp
    src/theming/theme_manager.cpp
    src/events/event_dispatcher.cpp
    src/generic/dialog.cpp
    src/generic/toolbar.cpp
    src/generic/viewport.cpp
)

# Create library target (STATIC only)
add_library(prong STATIC ${PRONG_SOURCES})

# Make library depend on formatting target
add_dependencies(prong format-library-sources)

# Set library properties
set_target_properties(prong PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)

# Include directories
target_include_directories(prong PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Add alias for use within the build tree
add_library(bombfork::prong ALIAS prong)

# === Installation ===

# Install library
install(TARGETS prong
    EXPORT prongTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install CMake package configuration
include(CMakePackageConfigHelpers)

# Generate version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/prongConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Generate config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prongConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/prongConfig.cmake"
    INSTALL_DESTINATION lib/cmake/prong
)

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/prongConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/prongConfigVersion.cmake"
    DESTINATION lib/cmake/prong
)

# Install export targets
install(EXPORT prongTargets
    FILE prongTargets.cmake
    NAMESPACE bombfork::
    DESTINATION lib/cmake/prong
)

# === Examples (optional) ===

if(PRONG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# === Tests (optional) ===

if(PRONG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# === Summary ===

message(STATUS "")
message(STATUS "Prong UI Framework Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: C++20")
message(STATUS "  Library Type: STATIC")
message(STATUS "  Warnings as Errors: ENABLED")
message(STATUS "  include-what-you-use: ${IWYU_PATH}")
message(STATUS "  clang-format: ${CLANG_FORMAT_PATH}")
message(STATUS "  Build Examples: ${PRONG_BUILD_EXAMPLES}")
message(STATUS "  Build Tests: ${PRONG_BUILD_TESTS}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
