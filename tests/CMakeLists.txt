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

# === Auto-discover and build tests ===

# Find all test_*.cpp files in the tests directory
file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)

# Function to convert test filename to test name
# Example: test_color.cpp -> ColorTest, test_coordinate_system.cpp -> CoordinateSystemTest
function(get_test_name test_file out_var)
    get_filename_component(test_name ${test_file} NAME_WE)
    # Remove "test_" prefix
    string(REGEX REPLACE "^test_" "" name_without_prefix ${test_name})
    # Convert to CamelCase: split by underscore, capitalize each word
    string(REPLACE "_" ";" name_parts ${name_without_prefix})
    set(camel_name "")
    foreach(part ${name_parts})
        string(SUBSTRING ${part} 0 1 first_char)
        string(SUBSTRING ${part} 1 -1 rest_chars)
        string(TOUPPER ${first_char} first_char_upper)
        string(APPEND camel_name "${first_char_upper}${rest_chars}")
    endforeach()
    string(APPEND camel_name "Test")
    set(${out_var} ${camel_name} PARENT_SCOPE)
endfunction()

# Build and register each test
set(TEST_NAMES "")
foreach(test_source ${TEST_SOURCES})
    # Get the test executable name (e.g., test_color)
    get_filename_component(test_exec_name ${test_source} NAME_WE)

    # Get the test name for CTest (e.g., ColorTest)
    get_test_name(${test_source} test_name)

    # Create executable
    add_executable(${test_exec_name} ${test_source})

    # Link with prong library
    target_link_libraries(${test_exec_name} PRIVATE prong)

    # Set properties
    set_target_properties(${test_exec_name} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};-Xiwyu;--error;-Xiwyu;--verbose=3"
    )

    # Ensure formatting runs before building
    add_dependencies(${test_exec_name} format-test-sources)

    # Register test with CTest
    add_test(NAME ${test_name} COMMAND ${test_exec_name})

    # Track test names for summary
    list(APPEND TEST_NAMES ${test_name})
endforeach()

# Print summary
message(STATUS "Tests configured:")
foreach(test_name ${TEST_NAMES})
    message(STATUS "  - ${test_name}")
endforeach()
