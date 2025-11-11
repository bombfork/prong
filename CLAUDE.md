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
- **Coordinate systems**: Uses a relative coordinate system with caching - see detailed section below
- **Update/render cycle**: `updateAll()` and `renderAll()` recursively traverse the component tree

**Important**: `update()` and `render()` are pure virtual and must be implemented by all concrete components.

### Coordinate System

Prong uses a **relative coordinate system** where child positions are always relative to their parent's origin. This design provides several benefits:

- **Intuitive positioning**: Child components don't need to know their parent's position
- **Automatic updates**: Moving a parent automatically moves all children
- **Layout flexibility**: Layout managers work with local coordinates only
- **Performance**: Global coordinates are cached and only recalculated when needed

#### Coordinate Spaces

1. **Local Coordinates** - Position relative to parent
   - Stored in `localX`, `localY` member variables
   - Set via `setPosition(x, y)` or `setBounds(x, y, w, h)`
   - Retrieved via `getPosition(x, y)` or `getBounds(x, y, w, h)`
   - For root components (no parent), local coordinates ARE the global coordinates

2. **Global Coordinates** - Absolute screen-space position
   - Calculated by summing all parent positions up the tree
   - Cached for performance and automatically invalidated when positions change
   - Retrieved via `getGlobalPosition(x, y)` or `getGlobalBounds(x, y, w, h)`
   - Used for rendering and hit testing

#### Key APIs

```cpp
// Setting local position (relative to parent)
component->setPosition(50, 75);
component->setBounds(50, 75, 200, 100);

// Reading local position
int x, y, w, h;
component->getPosition(x, y);           // Gets local position
component->getBounds(x, y, w, h);       // Gets local position + size

// Reading global position (for rendering/hit testing)
int gx, gy, gw, gh;
component->getGlobalPosition(gx, gy);   // Gets screen-space position
component->getGlobalBounds(gx, gy, gw, gh);

// Coordinate conversion
component->localToGlobal(localX, localY, globalX, globalY);
component->globalToLocal(globalX, globalY, localX, localY);

// Hit testing (uses global coordinates)
bool hit = component->containsGlobal(screenX, screenY);
```

#### Rendering with Coordinates

In your `render()` implementation, use the protected `getGlobalX()` and `getGlobalY()` helpers:

```cpp
void MyComponent::render() override {
    int x = getGlobalX();  // Screen-space X for rendering
    int y = getGlobalY();  // Screen-space Y for rendering

    // Render at global position
    renderer->drawRect(x, y, width, height, 1.0f, 1.0f, 1.0f, 1.0f);
}
```

#### Layout Managers and Coordinates

Layout managers work entirely with **local coordinates**. When positioning children:

```cpp
void MyLayout::layout(std::vector<Component*>& children, const Dimensions& available) {
    int currentX = 0;  // Local coordinate relative to parent
    for (auto* child : children) {
        child->setBounds(currentX, 0, 100, 50);  // Local position
        currentX += 100;
    }
}
```

#### Event Handling and Coordinates

Event handlers receive coordinates in **local space** relative to the component:

```cpp
bool MyComponent::handleClick(int localX, int localY) override {
    // localX, localY are relative to this component's origin
    // (0, 0) is the top-left corner of this component

    if (localX >= buttonX && localX < buttonX + buttonW) {
        // Handle button click
        return true;
    }
    return false;
}
```

The `Component::handleEvent()` method automatically converts global screen coordinates to local coordinates during event propagation.

#### Cache Invalidation

The global coordinate cache is automatically invalidated when:

- A component's position changes via `setPosition()` or `setBounds()`
- A component is added to a new parent
- Cache invalidation automatically cascades to all descendants

You should never need to manually invalidate the cache.

#### Best Practices

1. **Always use local coordinates** when positioning children or working with layouts
2. **Use global coordinates** only for rendering and hit testing
3. **Never store global coordinates** - they can become stale; always compute them when needed
4. **Let the cache work** - calling `getGlobalPosition()` multiple times in a frame is cheap
5. **Use protected helpers in render()** - `getGlobalX()` and `getGlobalY()` are concise

### Renderer Abstraction

The `IRenderer` interface (`include/bombfork/prong/rendering/irenderer.h`) provides:

- Frame lifecycle: `beginFrame()`, `endFrame()`, `present()`
- Drawing primitives: `drawRect()`, `drawSprite()`, `drawText()`
- Batching support: `drawSprites()` for efficient multi-sprite rendering
- Clipping: `enableScissorTest()` / `disableScissorTest()` for viewport management

All rendering must go through the `IRenderer` interface. Components receive renderer via constructor and store it as `Component::renderer`.

### Event System

Prong uses a **hierarchical event propagation model** where events flow through the component tree:

- **Event handling**: Components override `handleEventSelf()` to respond to events
- **Automatic propagation**: Events propagate from parent to children automatically via `Component::handleEvent()`
- **Hit testing**: Uses `Component::containsEvent()` for positional event checking
- **Event types**: Mouse (press, release, move, scroll), keyboard (key press, key release, char input)
- **Coordinate conversion**: Automatically converts coordinates to child-local space during propagation
- **Z-order**: Children rendered last (topmost) receive events first (reverse iteration)

Components must be enabled and visible to receive events. The Scene is the entry point for all events from the window system.

### Resize Behavior

Components can specify how they respond to parent resize events through resize behaviors. This is essential for responsive UI layouts.

#### Unified Resize Behavior

The `ResizeBehavior` enum provides unified control over both axes:

- `FIXED`: Keep original size and position (default)
- `SCALE`: Scale proportionally with parent
- `FILL`: Fill available parent space
- `MAINTAIN_ASPECT`: Scale while maintaining aspect ratio

```cpp
component->setResizeBehavior(Component::ResizeBehavior::FILL);
```

#### Per-Axis Resize Behavior

For more control, use `AxisResizeBehavior` to set independent horizontal and vertical behavior:

- `AxisResizeBehavior::FIXED`: Keep original size on this axis
- `AxisResizeBehavior::SCALE`: Scale proportionally with parent on this axis
- `AxisResizeBehavior::FILL`: Fill available parent space on this axis

```cpp
// Fixed width, fill height (common for panels in horizontal FlexLayout)
panel->setAxisResizeBehavior(Component::AxisResizeBehavior::FIXED,
                             Component::AxisResizeBehavior::FILL);
```

**Important**: When using FlexLayout, per-axis behavior is usually preferred:

- For horizontal FlexLayout (ROW): Use `FIXED` or `SCALE` horizontally (let FlexLayout control width), `FILL` vertically
- For vertical FlexLayout (COLUMN): Use `FILL` horizontally, `FIXED` or `SCALE` vertically (let FlexLayout control height)

Example from demo app:

```cpp
// Left panel in horizontal FlexLayout: fixed width (200px), fills height
leftPanel->setAxisResizeBehavior(Component::AxisResizeBehavior::FIXED,
                                 Component::AxisResizeBehavior::FILL);

// Center panel: fills both dimensions (grows with FlexLayout)
centerPanel->setAxisResizeBehavior(Component::AxisResizeBehavior::FILL,
                                   Component::AxisResizeBehavior::FILL);
```

#### Responsive Constraints

Combine resize behavior with constraints for bounded resizing:

```cpp
Component::ResponsiveConstraints constraints;
constraints.minWidth = 200;
constraints.maxWidth = 600;
constraints.minHeight = 150;
constraints.maxHeight = 450;
component->setConstraints(constraints);
```

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

All UI components (Button, Panel, ListBox, TextInput, Dialog, Toolbar, Viewport) are fully header-only.

## Namespace Structure

All code lives under `bombfork::prong` with subnamespaces:

- `bombfork::prong::core` - Component base classes
- `bombfork::prong::components` - All UI widgets (Button, Panel, ListBox, TextInput, Dialog, Toolbar, Viewport)
- `bombfork::prong::layout` - Layout managers
- `bombfork::prong::theming` - Theme system
- `bombfork::prong::events` - Event handling and platform abstractions (IClipboard, IKeyboard)
- `bombfork::prong::rendering` - Renderer interface
- `bombfork::prong::adapters` - Platform adapters (in examples only)

## Platform Abstractions

### TextInput Dependencies

The `TextInput` component requires two platform-specific interfaces for full functionality:

**IClipboard** (`include/bombfork/prong/events/iclipboard.h`):

- Provides clipboard access for copy/paste operations
- Must be injected via `textInput->setClipboard(clipboard)`

**IKeyboard** (`include/bombfork/prong/events/ikeyboard.h`):

- Converts platform-specific key codes to Prong's agnostic `Key` enum
- Must be injected via `textInput->setKeyboard(keyboard)`

#### GLFW Usage Example

```cpp
#include "glfw_adapters/glfw_adapters.h"

// Create GLFW adapters
auto adapters = examples::glfw::GLFWAdapters::create(window);

// Create TextInput
auto textInput = create<TextInput>()
    .withPlaceholder("Enter text...")
    .build();

// Inject adapters
textInput->setClipboard(adapters.clipboard.get());
textInput->setKeyboard(adapters.keyboard.get());
```

**Important**: Keep the `adapters` object alive for the lifetime of the TextInput!

#### Unit Testing with Mocks

For unit tests, use the mock implementations in `tests/mocks/`:

```cpp
#include "mocks/mock_clipboard.h"
#include "mocks/mock_keyboard.h"

// Create mocks
MockClipboard clipboard;
MockKeyboard keyboard;

// Create TextInput
TextInput input;
input.setClipboard(&clipboard);
input.setKeyboard(&keyboard);

// Test clipboard operations
clipboard.setString("Test");
input.handleKey(static_cast<int>(Key::V), PRESS, CTRL); // Paste
assert(input.getText() == "Test");
```

The mock implementations provide simple in-memory storage for testing without requiring a windowing system.

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
- **Coordinate systems**: Always use local coordinates when positioning children. Use global coordinates only for rendering and hit testing. See the Coordinate System section for details.
- **Event handling**: Override `handleEventSelf()` for component-specific event handling. The hierarchical propagation happens automatically via `Component::handleEvent()`.
- **Renderer lifecycle**: All rendering must occur between `beginFrame()` and `endFrame()` calls
- **Focus model**: Components track their own focus state via `FocusState` enum; Scene manages global focus
- **Thread safety**: ThemeManager is thread-safe; other components assume single-threaded use
- Always use the `mise build` command to build the library, tests and examples.
- The demo app located in @examples/demo_app/ is used to test the library's UX, it is built and run with the `mise demo` command. It is meant to gather feedback from the user, and for overall feature validation.
- Use gh cli to interact with the github repo when needed
- ALWAYS use ninja generator when using cmake.
- NEVER bypass iwyu or clang-format
- NEVER bypass git hooks
