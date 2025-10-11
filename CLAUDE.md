# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Prong is a modern C++20 UI framework from BombFork designed for high-performance applications. It's header-mostly, uses CRTP (Curiously Recurring Template Pattern) for zero-cost abstractions, and is both renderer-agnostic and window-agnostic.

**Key Design Principles:**
- Header-mostly architecture (minimal implementation files)
- CRTP-based component system for compile-time polymorphism
- Zero dependencies for core functionality
- Renderer abstraction (`IRenderer`) works with OpenGL, Vulkan, or custom backends
- Window abstraction (`IWindow`) works with GLFW, SDL, or native APIs

## Build Commands

### Standard Build
```bash
mkdir build && cd build
cmake .. -DPRONG_BUILD_EXAMPLES=ON -DPRONG_BUILD_TESTS=ON
cmake --build .
```

### Build Options
- `PRONG_BUILD_EXAMPLES` - Build example applications (default: ON)
- `PRONG_BUILD_TESTS` - Build unit tests (default: ON)

### Installation
```bash
sudo cmake --install .
```

## Architecture

### Core Abstraction Pattern

Prong uses CRTP throughout for zero-overhead polymorphism. The pattern looks like:

```cpp
template<typename DerivedT>
class LayoutManager {
    void layout(std::vector<Component*>& children) {
        auto* derived = static_cast<DerivedT*>(this);
        // Use derived class methods without virtual dispatch
    }
};

class MyLayout : public LayoutManager<MyLayout> {
    // Custom layout behavior
};
```

This eliminates virtual function call overhead while maintaining clean abstractions.

### Component System

The `Component` class (`include/bombfork/prong/core/component.h`) is the foundation:
- **Parent/child relationships**: Components form a tree structure with automatic ownership via `std::unique_ptr`
- **Event propagation**: Events flow down from parent to children, with children handling first (topmost rendered components get priority)
- **Coordinate systems**: Uses both global and local coordinates. Children positions are relative to their parent.
- **Update/render cycle**: `updateAll()` and `renderAll()` recursively traverse the component tree

**Important**: `update()` and `render()` are pure virtual and must be implemented by all concrete components.

### Renderer Abstraction

The `IRenderer` interface (`include/bombfork/prong/rendering/irenderer.h`) provides:
- Frame lifecycle: `beginFrame()`, `endFrame()`, `present()`
- Drawing primitives: `drawRect()`, `drawSprite()`, `drawText()`
- Batching support: `drawSprites()` for efficient multi-sprite rendering
- Clipping: `enableScissorTest()` / `disableScissorTest()` for viewport management

All rendering must go through the `IRenderer` interface. Components receive renderer via constructor and store it as `Component::renderer`.

### Event System

The `EventDispatcher` (`include/bombfork/prong/events/event_dispatcher.h`) routes input:
- **Registration**: Components must be registered with `registerComponent()` to receive events
- **Focus management**: Tracks both keyboard focus and mouse hover state
- **Hit testing**: Uses `Component::containsGlobal()` to find components at screen coordinates
- **Event types**: Mouse (click, press, release, move, scroll), keyboard (key, char)
- **Coordinate conversion**: Automatically converts global to local coordinates when dispatching to components

The event dispatcher maintains rendering order - components rendered last receive events first (z-order).

### Layout System

Layout managers (`include/bombfork/prong/layout/`) use CRTP and provide:
- **FlexLayout**: Flexbox-inspired with direction, justify, align, gap, and grow/shrink factors
- **GridLayout**: CSS Grid-inspired with rows/columns and gaps
- **DockLayout**: Docking panels (top, bottom, left, right, center fill)
- **StackLayout**: Simple vertical/horizontal stacking
- **FlowLayout**: Automatic wrapping layout

Layout managers implement:
- `measureLayout()`: Calculate required space for components
- `layout()`: Position and size components within available space

### Theming System

Located in `include/bombfork/prong/theming/`:
- **ThemeManager**: Singleton managing global themes, thread-safe
- **Color**: RGBA color with utility methods and named constants

Themes use semantic color names.

## Implementation Files

Only these modules require `.cpp` files (all in `src/`):
- `core/coordinate_system.cpp` - World ↔ Screen transformations
- `core/async_callback_queue.cpp` - Thread-safe callback management
- `theming/theme_manager.cpp` - Global theme state
- `events/event_dispatcher.cpp` - Event routing logic
- `generic/dialog.cpp`, `toolbar.cpp`, `viewport.cpp` - Complex generic components

Everything else is header-only.

## Namespace Structure

All code lives under `bombfork::prong` with subnamespaces:
- `bombfork::prong::core` - Component base classes
- `bombfork::prong::components` - UI widgets (Button, Panel, ListBox, TextInput)
- `bombfork::prong::layout` - Layout managers
- `bombfork::prong::theming` - Theme system
- `bombfork::prong::events` - Event handling
- `bombfork::prong::generic` - Higher-level components (Dialog, Toolbar, Viewport)
- `bombfork::prong::rendering` - Renderer interface

## C++20 Features

The codebase requires C++20 and uses:
- Concepts for template constraints
- Ranges for algorithms
- Three-way comparison operator (spaceship)
- Designated initializers
- constexpr improvements

GCC 10+, Clang 13+, or MSVC 2019+ required.

## CMake Integration

When used as a library via FetchContent:
```cmake
FetchContent_Declare(prong
    GIT_REPOSITORY https://github.com/bombfork/prong.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(prong)
target_link_libraries(your_app PRIVATE bombfork::prong)
```

The namespace for the installed target is `bombfork::prong`.

## Development Notes

- **Header-only by default**: Only add `.cpp` files when absolutely necessary (complex state, large implementations)
- **CRTP everywhere**: Prefer CRTP over virtual functions for component hierarchies and layout managers
- **Coordinate systems**: Always be explicit about global vs. local coordinates. Children are positioned relative to parents.
- **Event handling**: Components must be explicitly registered with EventDispatcher to receive events
- **Renderer lifecycle**: All rendering must occur between `beginFrame()` and `endFrame()` calls
- **Focus model**: Only one component can have keyboard focus; hover is tracked separately
- **Thread safety**: ThemeManager is thread-safe; other components assume single-threaded use
- Always use the `mise build` command to build the library, tests and examples.
- The demo app located in @examples/simple_app/ is used to test the library's UX, it is built and run with the `mise demo` command. It is meant to gather feedback from the user, and for overall feature validation.
- Use gh cli to interact with the github repo when needed
- ALWAYS use ninja generator when using cmake.
- NEVER bypass iwyu or clang-format