# Prong Framework Architecture

## Overview

Prong is a modern C++20 UI framework designed for high-performance applications with minimal overhead. It employs a scene-based architecture with a component hierarchy system, CRTP-based polymorphism, and platform abstraction layers for maximum flexibility.

**Core Design Principles:**
- **Header-mostly architecture**: Minimal implementation files, most functionality in headers
- **Zero-cost abstractions**: CRTP eliminates virtual function overhead
- **Platform agnostic**: Renderer and window abstractions work with any backend
- **Relative coordinate system**: Intuitive parent-child positioning with automatic caching
- **Hierarchical event model**: Events propagate naturally through the component tree

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Application                          │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                          Scene                              │
│  - Root component for UI hierarchy                          │
│  - Window resize handling                                   │
│  - Entry point for all events                               │
└─────────┬───────────────────────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────────────────────────────────┐
│                    Component Tree                           │
│  Panel ──┬── Button                                         │
│          ├── TextInput                                      │
│          └── Panel ──┬── ListBox                            │
│                      └── Dialog                             │
└─────────────────────────────────────────────────────────────┘
          │
          ▼
┌──────────────────────┬──────────────────┬──────────────────┐
│   IRenderer          │   IWindow        │   Layout         │
│   - OpenGL           │   - GLFW         │   Managers       │
│   - Vulkan           │   - SDL          │   - FlexLayout   │
│   - Custom           │   - Native       │   - GridLayout   │
│                      │                  │   - DockLayout   │
└──────────────────────┴──────────────────┴──────────────────┘
```

## Scene-Based System

### Scene as Root Container

The `Scene` class is the top-level container that manages the entire UI hierarchy. It serves as the bridge between your application window and the component tree.

**Responsibilities:**
- Owns references to window and renderer interfaces
- Automatically fills window dimensions
- Handles window resize events and propagates them to children
- Entry point for all window events into the component hierarchy
- Provides simplified `updateAll()`, `renderAll()`, and `present()` interface

**Lifecycle:**

```cpp
#include <bombfork/prong/core/scene.h>

using namespace bombfork::prong;

// Create window and renderer (platform-specific)
IWindow* window = createWindow();
IRenderer* renderer = createRenderer();

// Create scene with window and renderer
Scene scene(window, renderer);
scene.attach();

// Build UI - components automatically inherit renderer
auto mainPanel = create<Panel<>>()
    .withSize(800, 600)
    .build();
scene.addChild(std::move(mainPanel));

// Main loop
while (!window->shouldClose()) {
    // Poll window events
    window->pollEvents();

    // Update and render
    scene.updateAll(deltaTime);

    renderer->beginFrame();
    renderer->clear(0.1f, 0.1f, 0.1f, 1.0f);
    scene.renderAll();
    renderer->endFrame();

    scene.present();
}

scene.detach();
```

## Component Architecture

### Component Base Class

The `Component` class is the foundation for all UI elements. It provides essential functionality for rendering, events, geometry management, and parent-child relationships.

**Key Features:**
- **Pure virtual methods**: `update()` and `render()` must be implemented by concrete components
- **Parent-child tree**: Components form a hierarchy with automatic ownership via `std::unique_ptr`
- **Relative coordinates**: Local positioning with automatic global coordinate caching
- **State management**: Visibility, enabled state, focus state
- **Event handling**: Hierarchical event propagation through the tree
- **Layout integration**: Optional layout manager attachment

**Component Tree Structure:**

```
Scene (root)
├─ Panel (FlexLayout, horizontal)
│  ├─ Button "File"
│  ├─ Button "Edit"
│  └─ Button "View"
├─ Panel (DockLayout)
│  ├─ Panel "Sidebar" (left dock)
│  │  └─ ListBox
│  ├─ Panel "Content" (center fill)
│  │  └─ TextInput
│  └─ Toolbar (bottom dock)
└─ Dialog (modal, floating)
```

Each component:
- Has a local position relative to its parent
- Can have zero or more children
- Receives events in reverse rendering order (topmost first)
- Updates and renders recursively

### CRTP Pattern for Zero-Cost Abstractions

Prong uses the Curiously Recurring Template Pattern (CRTP) throughout to achieve compile-time polymorphism without virtual function overhead.

**How CRTP Works:**

```cpp
// Base class template parameterized by derived type
template<typename DerivedT>
class LayoutManager {
public:
    void performLayout(std::vector<Component*>& children) {
        // Cast to derived type at compile time
        auto* derived = static_cast<DerivedT*>(this);

        // Call derived implementation without virtual dispatch
        derived->layout(children, availableSpace);
    }

    // Interface methods that derived classes implement
    virtual Dimensions measureLayout(const std::vector<Component*>&) = 0;
    virtual void layout(std::vector<Component*>&, const Dimensions&) = 0;
};

// Derived class inherits from base with itself as template parameter
class FlexLayout : public LayoutManager<FlexLayout> {
public:
    Dimensions measureLayout(const std::vector<Component*>& components) override {
        // Flex-specific measurement
        return calculateFlexDimensions(components);
    }

    void layout(std::vector<Component*>& components, const Dimensions& space) override {
        // Flex-specific layout algorithm
        performFlexLayout(components, space);
    }
};
```

**Benefits:**
- **Zero overhead**: No virtual function table lookups
- **Compile-time optimization**: Inlining and dead code elimination
- **Type safety**: Compile-time type checking
- **Clean interface**: Similar to traditional inheritance

**Where CRTP is Used:**
- Layout managers (`LayoutManager<DerivedT>`)
- Template-based component extensions
- Compile-time behavior customization

### Component Lifecycle

```
┌──────────────┐
│   Creation   │  - Constructor initializes state
│              │  - Renderer passed (or inherited from parent)
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ Add to Tree  │  - addChild() establishes parent-child relationship
│              │  - Renderer propagated to children
│              │  - Global cache invalidated
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Update     │  - update(deltaTime) called recursively
│   Cycle      │  - State changes, animations, logic
│              │  - updateAll() traverses entire tree
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Render     │  - render() called recursively
│   Cycle      │  - Uses global coordinates for drawing
│              │  - Draws primitives via IRenderer
│              │  - renderAll() traverses entire tree
└──────┬───────┘
       │
       ▼
┌──────────────┐
│Event Handling│  - handleEvent() propagates to children
│              │  - Hit testing with global coordinates
│              │  - Automatic conversion to local coordinates
│              │  - Reverse rendering order (topmost first)
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ Destruction  │  - Automatic cleanup via unique_ptr
│              │  - Children destroyed recursively
└──────────────┘
```

## Coordinate System

Prong uses a **relative coordinate system** with automatic caching for optimal performance and developer ergonomics.

### Coordinate Spaces

**Local Coordinates (Relative):**
- Position relative to parent's origin
- Used for positioning children and layout calculations
- Set via `setPosition(x, y)` or `setBounds(x, y, w, h)`
- For root components (no parent), local coordinates ARE global coordinates

**Global Coordinates (Absolute):**
- Absolute screen-space position
- Calculated by walking up parent chain and summing local positions
- Cached for performance, automatically invalidated on position changes
- Used for rendering and hit testing
- Retrieved via `getGlobalPosition(x, y)` or `getGlobalBounds(x, y, w, h)`

### Caching System

```
Component Position Change
         │
         ▼
   ┌──────────┐
   │ Mark     │
   │ Dirty    │
   └────┬─────┘
        │
        ▼
   ┌──────────┐
   │Propagate │    Invalidate entire
   │to        │◄── descendant tree
   │Children  │
   └────┬─────┘
        │
        ▼
   On Next Global Access
        │
        ▼
   ┌──────────┐
   │Calculate │    Walk up to parent,
   │Global    │◄── sum local positions
   │Position  │
   └────┬─────┘
        │
        ▼
   ┌──────────┐
   │ Cache    │
   │ Result   │    O(1) for subsequent accesses
   └──────────┘
```

**Performance Characteristics:**
- Cache hit: O(1) - return cached values
- Cache miss (single component): O(1) - calculate from parent's cache
- Cache miss (entire tree): O(depth) - walk to root once
- Moving parent: O(children) - invalidate all descendants

**See Also:** [Coordinate System Design](coordinate_system.md) for comprehensive details.

## Renderer Abstraction

The `IRenderer` interface provides platform-agnostic drawing operations. Implementations can use OpenGL, Vulkan, DirectX, software rendering, or any custom backend.

### IRenderer Interface

```cpp
class IRenderer {
public:
    // Frame lifecycle
    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void present() = 0;
    virtual void clear(float r, float g, float b, float a) = 0;

    // Drawing primitives
    virtual void drawRect(int x, int y, int width, int height,
                         float r, float g, float b, float a) = 0;

    virtual void drawSprite(int x, int y, int width, int height,
                           int textureID, float u0, float v0,
                           float u1, float v1) = 0;

    virtual void drawText(int x, int y, const std::string& text,
                         float r, float g, float b, float a) = 0;

    // Batch rendering
    virtual void drawSprites(const std::vector<SpriteData>& sprites) = 0;

    // Clipping (for viewports and scroll areas)
    virtual void enableScissorTest(int x, int y, int width, int height) = 0;
    virtual void disableScissorTest() = 0;

    // Viewport management
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual void getViewportSize(int& width, int& height) const = 0;
};
```

### Rendering in Components

Components receive the renderer via constructor and use it in their `render()` method:

```cpp
class MyComponent : public Component {
public:
    void render() override {
        // Always use GLOBAL coordinates for rendering
        int x = getGlobalX();
        int y = getGlobalY();

        // Draw background
        renderer->drawRect(x, y, width, height, 0.2f, 0.2f, 0.2f, 1.0f);

        // Draw border
        renderer->drawRect(x, y, width, 2, 1.0f, 1.0f, 1.0f, 1.0f);  // Top
        renderer->drawRect(x, y + height - 2, width, 2, 1.0f, 1.0f, 1.0f, 1.0f);  // Bottom

        // Draw text with offset
        renderer->drawText(x + 10, y + 20, "Hello", 1.0f, 1.0f, 1.0f, 1.0f);
    }
};
```

### Frame Lifecycle

```cpp
// Main render loop
while (!window->shouldClose()) {
    if (renderer->beginFrame()) {
        renderer->clear(0.1f, 0.1f, 0.1f, 1.0f);

        // All rendering happens here
        scene.renderAll();

        renderer->endFrame();
    }

    scene.present();
    window->swapBuffers();
}
```

## Event System

Prong uses a **hierarchical event propagation model** where events flow naturally through the component tree.

### Event Flow

```
Window Event
     │
     ▼
   Scene
     │
     ▼
┌──────────────────┐
│  Hit Testing     │  Check if event position is within bounds
│  (Global Coords) │  using containsGlobal(x, y)
└────────┬─────────┘
         │
         ▼
    ┌─────────┐
    │ Parent  │ handleEvent()
    └────┬────┘
         │
         ├─► handleEventSelf() ───► Process event
         │                          Return true if handled
         │
         ▼
    ┌─────────────────┐
    │ Propagate to    │  Convert to local coordinates
    │ Children        │  Test in reverse order (topmost first)
    │ (if not handled)│
    └────┬────────────┘
         │
         ├─► Child 3 (topmost) ───► handleEvent()
         ├─► Child 2          ───► handleEvent()
         └─► Child 1 (bottom) ───► handleEvent()
```

### Event Types

**Mouse Events:**
- `MOUSE_PRESS` - Mouse button pressed
- `MOUSE_RELEASE` - Mouse button released
- `MOUSE_MOVE` - Mouse cursor moved
- `MOUSE_SCROLL` - Mouse wheel scrolled

**Keyboard Events:**
- `KEY_PRESS` - Key pressed
- `KEY_RELEASE` - Key released
- `CHAR_INPUT` - Character input (for text entry)

**Window Events:**
- `WINDOW_RESIZE` - Window size changed
- `WINDOW_FOCUS` - Window gained/lost focus

### Handling Events

Components override `handleEventSelf()` to respond to events:

```cpp
class MyButton : public Component {
protected:
    bool handleEventSelf(const Event& event) override {
        if (event.type == EventType::MOUSE_PRESS) {
            // Event coordinates are already in LOCAL space
            if (containsLocal(event.mouseX, event.mouseY)) {
                onClick();
                return true;  // Event consumed
            }
        }
        return false;  // Event not handled, continue propagation
    }

    bool containsLocal(int localX, int localY) const {
        return localX >= 0 && localX < width &&
               localY >= 0 && localY < height;
    }
};
```

**Key Points:**
- Event coordinates are automatically converted to local space during propagation
- Return `true` to stop propagation (event consumed)
- Return `false` to allow event to continue to other components
- Children rendered last (topmost) receive events first
- Components must be enabled and visible to receive events

### Hit Testing

The framework uses global coordinates for hit testing:

```cpp
bool Component::handleEvent(const Event& event) {
    if (!visible || !enabled) {
        return false;
    }

    // Hit test using global coordinates
    if (!containsEvent(event)) {
        return false;  // Event outside bounds
    }

    // Let component handle first
    if (handleEventSelf(event)) {
        return true;
    }

    // Propagate to children in reverse order
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        // Convert event to child's local coordinates
        Event localEvent = event;
        globalToLocal(event.mouseX, event.mouseY, localEvent.mouseX, localEvent.mouseY);

        if ((*it)->handleEvent(localEvent)) {
            return true;
        }
    }

    return false;
}
```

## Focus Management

The focus system tracks which component is actively receiving keyboard input.

### Focus States

```cpp
enum class FocusState {
    NONE,     // Component has no focus
    HOVERING, // Mouse is over component
    FOCUSED,  // Component has keyboard focus
    ACTIVE    // Component is being interacted with (e.g., button pressed)
};
```

### Focus Flow

```
User Interaction
       │
       ▼
┌──────────────┐
│  Scene       │  Manages global focus
│  Focus       │  Tracks focused component
│  Manager     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Component   │  setFocus(true) called
│  Gains Focus │
└──────┬───────┘
       │
       ├─► focusState = FOCUSED
       │
       └─► focusCallback(true) ───► Notify listeners
```

**Focus Behavior:**
- Only one component can have keyboard focus at a time
- Scene manages focus globally via `setFocusedComponent()`
- Components can subscribe to focus changes via `setFocusCallback()`
- Mouse hover sets `HOVERING` state automatically during event handling
- Active state (e.g., button held down) managed by component

## Platform Abstractions

### IWindow Interface

The `IWindow` interface abstracts window creation and management:

```cpp
class IWindow {
public:
    virtual void getSize(int& width, int& height) const = 0;
    virtual bool shouldClose() const = 0;
    virtual void pollEvents() = 0;
    virtual void swapBuffers() = 0;
};
```

Implementations available:
- GLFW (`GLFWWindowAdapter` in examples)
- SDL (community implementations)
- Native OS APIs (Windows, macOS, Linux)

### Platform-Specific Interfaces

Some components require additional platform abstractions:

**IClipboard** - For copy/paste operations:
```cpp
class IClipboard {
public:
    virtual std::string getString() const = 0;
    virtual void setString(const std::string& text) = 0;
};
```

**IKeyboard** - For key code translation:
```cpp
class IKeyboard {
public:
    virtual Key translateKey(int platformKey) const = 0;
};
```

These are injected into components that need them (e.g., `TextInput`):

```cpp
auto textInput = create<TextInput>()
    .withPlaceholder("Enter text...")
    .build();

// Inject platform adapters
textInput->setClipboard(clipboard);
textInput->setKeyboard(keyboard);
```

## Namespace Structure

All code lives under `bombfork::prong` with logical subnamespaces:

```
bombfork::prong
├── core               - Component, Scene, Event
├── components         - Button, Panel, ListBox, TextInput, Dialog, Toolbar, Viewport
├── layout             - FlexLayout, GridLayout, DockLayout, StackLayout, FlowLayout
├── rendering          - IRenderer and rendering primitives
├── events             - Event types, IClipboard, IKeyboard, IWindow
└── theming            - ThemeManager, Color

bombfork::prong::adapters  (examples only)
├── glfw               - GLFW adapters
└── opengl             - OpenGL renderer implementation
```

## Component Ownership Model

Prong uses `std::unique_ptr` for component ownership, following modern C++ RAII principles:

```cpp
// Creating components
auto button = std::make_unique<Button>(renderer, "Click Me");
auto panel = std::make_unique<Panel<>>(renderer);

// Adding to parent transfers ownership
panel->addChild(std::move(button));  // panel now owns button

// button pointer is now null, panel manages lifetime
// When panel is destroyed, button is automatically destroyed
```

**With ComponentBuilder:**

```cpp
auto panel = create<Panel<>>()
    .withChildren(
        create<Button>("OK").build(),      // Ownership transferred
        create<Button>("Cancel").build()   // Ownership transferred
    )
    .build();
// panel owns both buttons
```

## Threading Model

**Single-threaded by default:**
- Component tree, event handling, rendering are single-threaded
- Main thread owns the UI hierarchy

**Thread-safe components:**
- `ThemeManager` - Thread-safe singleton for global theme access
- `AsyncCallbackQueue` - Thread-safe callback queue for async operations

**Async Operations:**

```cpp
// Queue callback from worker thread
asyncQueue.enqueue([weakComponent = std::weak_ptr(component)]() {
    if (auto comp = weakComponent.lock()) {
        comp->updateData();
    }
});

// Process on main thread (during update cycle)
scene.updateAll(deltaTime);  // Processes queued callbacks
```

## Build System Integration

### CMake Integration

**As a subproject:**

```cmake
add_subdirectory(external/prong)
target_link_libraries(myapp PRIVATE bombfork::prong)
```

**Via FetchContent:**

```cmake
include(FetchContent)
FetchContent_Declare(prong
    GIT_REPOSITORY https://github.com/bombfork/prong.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(prong)
target_link_libraries(myapp PRIVATE bombfork::prong)
```

**Build Options:**
- `PRONG_BUILD_EXAMPLES` - Build example applications (default: ON)
- `PRONG_BUILD_TESTS` - Build unit tests (default: ON)

## Implementation Details

### Header-Mostly Architecture

Only these modules require `.cpp` implementation files:
- `core/coordinate_system.cpp` - World ↔ Screen transformations
- `core/async_callback_queue.cpp` - Thread-safe callback management
- `theming/theme_manager.cpp` - Global theme state

All UI components are fully header-only:
- `Button`, `Panel`, `ListBox`, `TextInput`, `Dialog`, `Toolbar`, `Viewport`
- All layout managers
- Component base class

**Benefits:**
- Faster compilation (no need to recompile library)
- Better optimization (compiler sees full implementation)
- Easier integration (just include headers)

### C++20 Features

The codebase requires C++20 and uses:
- **Concepts** - Template constraints for type safety
- **Ranges** - Modern iteration and algorithms
- **Three-way comparison** (spaceship operator `<=>`)
- **Designated initializers** - Clear struct initialization
- **constexpr improvements** - More compile-time computation

**Compiler Requirements:**
- GCC 10+
- Clang 13+
- MSVC 2019 16.11+

## Performance Considerations

### Zero-Cost Abstractions

- CRTP eliminates virtual function overhead in hot paths
- Relative coordinates with caching avoid redundant calculations
- Layout calculations happen only when marked invalid
- Batch rendering APIs for sprite-heavy scenes

### Memory Layout

- Components use contiguous storage for children (`std::vector`)
- Cache-friendly iteration during update/render
- Small memory footprint per component (~200 bytes base)

### Optimization Opportunities

- **Dirty rectangle tracking** - Only redraw changed regions
- **Culling** - Skip rendering for off-screen components
- **Layout caching** - Avoid redundant layout calculations
- **Event pruning** - Skip disabled/invisible subtrees

## See Also

- [Coordinate System Design](coordinate_system.md) - Detailed coordinate system documentation
- [ComponentBuilder Pattern](component_builder.md) - Fluent API for creating components
- [Layout System Guide](layouts.md) - Comprehensive layout manager documentation
- [Best Practices](best_practices.md) - Guidelines for effective Prong usage
