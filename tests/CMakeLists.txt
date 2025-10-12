cmake_minimum_required(VERSION 3.14)

# Enable testing
include(CTest)

# === Clang-format target for test sources ===

# Collect test source files for formatting (recursive to catch all test files)
file(GLOB_RECURSE TEST_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Create custom target that runs clang-format on test files
# Marked as ALL to ensure it runs on every build
add_custom_target(format-test-sources ALL
    COMMAND ${CLANG_FORMAT_PATH} -i ${TEST_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-format on test source files..."
    VERBATIM
    SOURCES ${TEST_SOURCE_FILES}
)

# Test executable for Color class
add_executable(test_color test_color.cpp)
target_link_libraries(test_color PRIVATE prong)
set_target_properties(test_color PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
add_dependencies(test_color format-test-sources)
add_test(NAME ColorTest COMMAND test_color)

# Test executable for CoordinateSystem class
add_executable(test_coordinate_system test_coordinate_system.cpp)
target_link_libraries(test_coordinate_system PRIVATE prong)
set_target_properties(test_coordinate_system PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
add_dependencies(test_coordinate_system format-test-sources)
add_test(NAME CoordinateSystemTest COMMAND test_coordinate_system)

# Test executable for Scene class
add_executable(test_scene test_scene.cpp)
target_link_libraries(test_scene PRIVATE prong)
set_target_properties(test_scene PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
add_dependencies(test_scene format-test-sources)
add_test(NAME SceneTest COMMAND test_scene)

# Test executable for Component class
add_executable(test_component test_component.cpp)
target_link_libraries(test_component PRIVATE prong)
set_target_properties(test_component PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
)
add_dependencies(test_component format-test-sources)
add_test(NAME ComponentTest COMMAND test_component)

message(STATUS "Tests configured:")
message(STATUS "  - ColorTest")
message(STATUS "  - CoordinateSystemTest")
message(STATUS "  - SceneTest")
message(STATUS "  - ComponentTest")
