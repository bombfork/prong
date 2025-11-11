# Contributing to Prong UI Framework

Thank you for your interest in contributing to Prong! This guide will help you get started with development and ensure your contributions align with the project's standards.

## Table of Contents

- [Development Environment](#development-environment)
- [Building the Project](#building-the-project)
- [Development Workflow](#development-workflow)
- [Code Style](#code-style)
- [Testing](#testing)
- [Pull Request Process](#pull-request-process)
- [Git Hooks](#git-hooks)

## Development Environment

### Required Tools

To build and develop Prong, you need:

- **C++20 compatible compiler**
  - GCC 10+ (Linux)
  - Clang 13+ (macOS/Linux)
  - MSVC 2019+ (Windows)
- **CMake 3.14+** - Build system generator
- **Ninja** - Build system (required, faster than Make)
- **mise** - Task runner for convenient build commands
- **clang-format** - Code formatting (enforced by git hooks)
- **include-what-you-use (iwyu)** - Header dependency checking (enforced by git hooks)
- **Docker** - For containerized builds (see [docs/docker.md](docs/docker.md))

### Installing mise

mise is used to run build commands consistently. Install it using:

```bash
# Linux/macOS
curl https://mise.run | sh
```

After installation, you can see all available tasks with:

```bash
mise tasks
```

## Building the Project

### Quick Build with mise (Recommended)

The simplest way to build everything:

```bash
# Build library, tests, and examples
mise build
```

This command:

- Configures CMake with Ninja generator
- Builds the core library
- Builds all tests
- Builds all example applications

### Running the Demo Application

To build and run the demo app (used for UX testing and feature validation):

```bash
mise demo
```

This builds and launches the demo application found in `examples/demo_app/`.

### Manual CMake Build

If you prefer to use CMake directly:

```bash
# Create build directory
mkdir build && cd build

# Configure (Ninja generator is REQUIRED)
cmake .. -G Ninja \
    -DPRONG_BUILD_EXAMPLES=ON \
    -DPRONG_BUILD_TESTS=ON

# Build
ninja

# Or build specific targets
ninja bombfork_prong        # Library only
ninja prong_tests           # Tests only
ninja prong_demo_app        # Demo app only
```

### Build Options

CMake configuration options:

- `PRONG_BUILD_EXAMPLES=ON/OFF` - Build example applications (default: ON)
- `PRONG_BUILD_TESTS=ON/OFF` - Build unit tests (default: ON)

## Development Workflow

### Code Formatting

Code formatting is using clang-format and is integrated to cmake and git hooks.

You can format all code with:

```bash
mise format
```

This runs clang-format on all source files according to `.clang-format` rules.

### Running Tests

Run the test suite with:

```bash
mise test
```

This builds and executes all unit tests. All tests must pass before submitting a PR.

### Include-What-You-Use (IWYU)

Header dependencies are verified automatically by IWYU and is integrated to cmake and git hooks.

IWYU ensures that:

- Headers only include what they directly use
- No transitive include dependencies
- Header files are self-contained

The git hooks will automatically check this before commits.

### Demo Application Testing

The demo app (`examples/demo_app/`) is used for:

- UX testing and validation
- Manual feature testing
- Gathering user feedback
- Visual verification of components

Keep it up to date when working on UI components. Run it with:

```bash
mise demo
```

## Code Style

### Header-Only by Default

Prong follows a **header-only architecture** where possible. Only add `.cpp` implementation files when absolutely necessary:

- Complex state management
- Large implementations that would bloat compile times
- Platform-specific code requiring conditional compilation

**Existing `.cpp` files:**

- `src/core/coordinate_system.cpp` - World ↔ Screen transformations
- `src/core/async_callback_queue.cpp` - Thread-safe callback management
- `src/theming/theme_manager.cpp` - Global theme state

### CRTP Everywhere

Use the Curiously Recurring Template Pattern (CRTP) for all component hierarchies and layout managers to achieve **zero-cost abstractions**:

```cpp
// Good - CRTP for compile-time polymorphism
template<typename DerivedT>
class LayoutManager {
    void layout(std::vector<Component*>& children) {
        auto* derived = static_cast<DerivedT*>(this);
        // Use derived methods without virtual dispatch
    }
};

class MyLayout : public LayoutManager<MyLayout> {
    // Implementation
};

// Avoid - virtual functions add runtime overhead
class LayoutManager {
    virtual void layout() = 0;  // Runtime cost
};
```

### Relative Coordinate System

Prong uses a **relative coordinate system** where child positions are always relative to their parent:

**DO:**

```cpp
// Position children relative to parent
panel->setPosition(100, 50);        // Panel at (100, 50) in parent space
button->setPosition(20, 10);        // Button at (20, 10) relative to panel
panel->addChild(std::move(button)); // Button now at global (120, 60)
```

**DON'T:**

```cpp
// Never use global coordinates when positioning children
int globalX, globalY;
panel->getGlobalPosition(globalX, globalY);
button->setPosition(globalX + 20, globalY + 10);  // WRONG!
```

**Rendering:**

- Use `getGlobalX()` and `getGlobalY()` in `render()` methods
- Global coordinates are cached automatically
- Never manually track or store global positions

### Component Architecture

When creating components:

1. **Extend Component** - All UI elements inherit from `Component`
2. **Scene-based** - Scene is the root of all UI hierarchies
3. **Event handling** - Override `handleEventSelf()` for event processing
4. **Implement pure virtuals** - `update()` and `render()` must be implemented

Example component structure:

```cpp
template<typename DerivedT = void>
class MyWidget : public Component {
private:
    // State
    std::string text;
    bool isHovered = false;

public:
    explicit MyWidget(rendering::IRenderer* renderer)
        : Component(renderer, "MyWidget") {}

    void update(double deltaTime) override {
        // Update logic
    }

    void render() override {
        int x = getGlobalX();  // Use cached global position
        int y = getGlobalY();
        // Render using IRenderer interface
        renderer->drawRect(x, y, width, height, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    bool handleEventSelf(const Event& event) override {
        // Handle component-specific events
        if (event.type == Event::Type::MOUSE_PRESS) {
            // Handle click
            return true;  // Event consumed
        }
        return false;  // Event not handled
    }
};
```

### Documentation Requirements

All public APIs must have Doxygen documentation:

```cpp
/**
 * @brief Brief description of the class/function
 *
 * Detailed description explaining purpose, behavior, and usage.
 *
 * @param paramName Description of parameter
 * @return Description of return value
 *
 * @note Any important notes
 * @warning Any warnings or gotchas
 *
 * Example:
 * ```cpp
 * MyClass obj;
 * obj.doSomething();
 * ```
 */
```

### Namespace Structure

All code belongs in the `bombfork::prong` namespace with subnamespaces:

```cpp
namespace bombfork::prong::core { /* Core components */ }
namespace bombfork::prong::components { /* UI widgets */ }
namespace bombfork::prong::layout { /* Layout managers */ }
namespace bombfork::prong::theming { /* Theme system */ }
namespace bombfork::prong::events { /* Event handling */ }
namespace bombfork::prong::rendering { /* Renderer interface */ }
```

## Testing

### Writing Tests

Tests use Catch2 and are located in `tests/`:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <bombfork/prong/components/button.h>

TEST_CASE("Button handles click events", "[button]") {
    MockRenderer renderer;
    Button button(&renderer, "Test");

    Event event;
    event.type = Event::Type::MOUSE_PRESS;
    event.localX = 50;
    event.localY = 50;

    REQUIRE(button.handleEvent(event) == true);
}
```

### Running Tests

```bash
# Run all tests
mise test

# Or manually
./build/tests/prong_tests
```

### Test Coverage

- All new features require unit tests
- Bug fixes should include regression tests
- Aim for high coverage of public APIs
- Use mock implementations (`tests/mocks/`) for interfaces

## Pull Request Process

### 1. Fork and Branch

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/prong.git
cd prong

# Create a feature branch
git checkout -b feature/my-new-feature
# or
git checkout -b fix/issue-123
```

### 2. Make Changes

- Write your code following the style guidelines
- Add tests for new functionality
- Update documentation as needed
- Run `mise format` before committing

### 3. Test Your Changes

```bash
# Format code
mise format

# Build everything
mise build

# Run tests
mise test

# Test with demo app
mise demo
```

### 4. Commit

Git hooks will automatically:

- Run clang-format on changed files
- Run iwyu checks
- Validate commit message format

**NEVER bypass git hooks** using `--no-verify`!

```bash
# Good commit messages
git commit -m "Add resize behavior to Panel component"
git commit -m "Fix coordinate calculation bug in FlexLayout"
git commit -m "Update theming documentation"

# Commit messages should be:
# - Imperative mood ("Add" not "Added")
# - Concise but descriptive
# - Reference issue numbers when applicable
```

### 5. Push and Create PR

```bash
git push origin feature/my-new-feature
```

Then create a Pull Request on GitHub with:

**Title:** Clear, concise description of changes

**Description should include:**

- Summary of changes
- Motivation and context
- Related issue numbers (e.g., "Fixes #123", "Closes #456")
- Screenshots/GIFs for UI changes
- Testing performed
- Any breaking changes

**Example PR description:**

```markdown
## Summary
Add responsive resize behavior to Panel component to support dynamic layouts.

## Changes
- Add `ResizeBehavior` enum with FIXED, SCALE, FILL, MAINTAIN_ASPECT options
- Implement `AxisResizeBehavior` for per-axis control
- Add constraint system with min/max dimensions
- Update Panel to handle parent resize events

## Testing
- Unit tests for all resize behaviors
- Manual testing with demo app
- Verified with FlexLayout and DockLayout

## Related Issues
Fixes #45

## Screenshots
[Attach screenshot or GIF showing resize behavior]
```

### 6. Review Process

- Maintainers will review your PR
- Address any requested changes
- Keep commits clean (squash if needed)
- Once approved, your PR will be merged!

## Git Hooks

Prong uses [hk](https://github.com/jdx/hk) to manage git hooks. The configuration is defined in `hk.pkl` at the root of the repository.

### Pre-commit Hook

The pre-commit hook automatically:

- Runs `clang-format` on all staged C++ files (`**/*.cpp`, `**/*.h`)
- Checks formatting with `mise run format-check`
- Automatically fixes formatting issues with `mise run format` when `fix = true`
- Prevents commits if formatting cannot be fixed

**Note:** The hook only checks C++ source files. Other files (like markdown) are not checked by the pre-commit hook.

### Installing Hooks

Install hk hooks with:

```bash
hk install
```

This creates the `.git/hooks/pre-commit` wrapper that calls `hk run pre-commit`.

### Important: Never Bypass Hooks

**DO NOT** use `--no-verify` to bypass hooks:

```bash
# WRONG - Never do this!
git commit --no-verify -m "Quick fix"

# RIGHT - Fix the issues
mise format
git add .
git commit -m "Fix formatting issues"
```

Bypassing hooks can break the CI build and will cause your PR to be blocked.

## Questions?

- Check existing [issues](https://github.com/bombfork/prong/issues)
- Review the [documentation](https://github.com/bombfork/prong/tree/main/docs)
- Read the [CLAUDE.md](CLAUDE.md) for detailed architecture notes
- Ask in the GitHub Discussions

Thank you for contributing to Prong! 🔱
