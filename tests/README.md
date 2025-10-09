# Prong Library Tests

This directory contains unit tests for the Prong UI Framework, using CTest as the test runner.

## Test Files

### `test_color.cpp`
Tests the `Color` class from `bombfork::prong::theming`:
- Color construction (default and parameterized)
- Equality operators
- Predefined colors (WHITE, BLACK, RED, GREEN, BLUE, etc.)
- Color blending operations
- Brightness adjustments (additive and multiplicative)
- Alpha channel manipulation
- Constexpr operations and compile-time evaluation

### `test_coordinate_system.cpp`
Tests the `CoordinateSystem` class from `bombfork::prong`:
- Viewport construction and updates
- Camera position and zoom management
- Cell size calculations with zoom
- World ↔ Screen coordinate transformations
- Cell ↔ Screen coordinate conversions
- Viewport cell counting
- Visible region calculations
- Cell visibility testing
- Screen position validation
- Coordinate key generation and parsing
- Map bounds validation and clamping
- Distance calculations (world and screen space)

## Building Tests

From the project root directory:

```bash
mkdir build && cd build
cmake .. -DPRONG_BUILD_TESTS=ON
cmake --build .
```

## Running Tests

### Run all tests with CTest:
```bash
cd build
ctest --output-on-failure
```

### Run individual test executables:
```bash
cd build
./tests/test_color
./tests/test_coordinate_system
```

### Verbose test output:
```bash
ctest --verbose
```

## Test Framework

These tests use a simple assertion-based approach with:
- `assert()` for test conditions
- Custom helper functions for floating-point comparisons
- Clear test function organization
- Descriptive output with ✓ checkmarks for passed tests
- Return codes: 0 for success, 1 for failure

## Adding New Tests

To add a new test:

1. Create a new `.cpp` file in the `tests/` directory
2. Add the test executable to `CMakeLists.txt`:
   ```cmake
   add_executable(test_newfeature test_newfeature.cpp)
   target_link_libraries(test_newfeature PRIVATE prong)
   set_target_properties(test_newfeature PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
   add_test(NAME NewFeatureTest COMMAND test_newfeature)
   ```
3. Follow the existing test structure with:
   - Individual test functions for each feature
   - A main() function that runs all tests
   - Proper error handling and descriptive output

## Notes

- All tests are built with C++20 standard
- Tests link against the static `prong` library
- Warnings are treated as errors (`-Werror`)
- Tests use the header-only portions of the library plus implementation files
