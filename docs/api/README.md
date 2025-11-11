# Prong API Reference

Comprehensive API documentation for the Prong UI framework.

## Quick Navigation

### Core
- [Component](core/Component.md) - Base class for all UI elements
- [Scene](core/Scene.md) - Root container and event coordinator
- [ComponentBuilder](core/ComponentBuilder.md) - Fluent API for component construction
- [Event](core/Event.md) - Unified event structure

### Components
- [Button](components/Button.md) - Clickable button widget
- [Panel](components/Panel.md) - Container panel
- [TextInput](components/TextInput.md) - Single-line text input
- [ListBox](components/ListBox.md) - Scrollable list with selection
- [Dialog](components/Dialog.md) - Modal dialog box
- [Toolbar](components/Toolbar.md) - Horizontal toolbar with items
- [Viewport](components/Viewport.md) - Scrollable viewport container

### Layout Managers
- [FlexLayout](layouts/FlexLayout.md) - Flexbox-inspired flexible layout
- [GridLayout](layouts/GridLayout.md) - CSS Grid-inspired grid layout
- [DockLayout](layouts/DockLayout.md) - Docking panel layout (top, bottom, left, right, center)
- [StackLayout](layouts/StackLayout.md) - Simple horizontal/vertical stacking
- [FlowLayout](layouts/FlowLayout.md) - Wrapping flow layout

### Events
- [Event Struct](events/Event.md) - Event data structure
- [IWindow](events/IWindow.md) - Window abstraction interface
- [IClipboard](events/IClipboard.md) - Clipboard abstraction
- [IKeyboard](events/IKeyboard.md) - Keyboard key mapping

### Rendering
- [IRenderer](rendering/IRenderer.md) - Renderer abstraction interface

### Theming
- [ThemeManager](theming/ThemeManager.md) - Global theme management
- [Color](theming/Color.md) - RGBA color utility class
- [AdvancedTheme](theming/AdvancedTheme.md) - Theme data structure

## Architecture Overview

### Component Hierarchy

```
Component (abstract base)
├── Panel (container)
├── Button (interactive)
├── TextInput (text editing)
├── ListBox (list selection)
├── Dialog (modal overlay)
├── Toolbar (item container)
└── Viewport (scrollable container)
```

### Coordinate Systems

Prong uses a **relative coordinate system**:
- **Local coordinates**: Position relative to parent (used for positioning children)
- **Global coordinates**: Absolute screen-space position (used for rendering and hit testing)
- Coordinates are automatically cached and invalidated as needed

### Event Propagation

Events flow hierarchically:
1. Scene receives event from window
2. Scene propagates to root components
3. Components check bounds and propagate to children
4. Child-to-parent order (topmost rendered gets first chance)
5. Event stops when handler returns `true`

### Layout System

Layout managers use CRTP for zero-overhead polymorphism:
```cpp
template<typename ParentT>
class MyLayout : public LayoutManager<MyLayout<ParentT>> {
  Dimensions measureLayout(const std::vector<Component*>& children) override;
  void layout(std::vector<Component*>& children, const Dimensions& available) override;
};
```

## Common Patterns

### Creating Components

```cpp
auto button = create<Button>("Click Me")
                .withSize(200, 60)
                .withPosition(100, 100)
                .withClickCallback([]() { /* handle click */ })
                .build();

parent->addChild(std::move(button));
```

### Custom Event Handling

```cpp
class MyComponent : public Component {
protected:
  bool handleEventSelf(const Event& event) override {
    if (event.type == Event::Type::MOUSE_PRESS) {
      // Handle event
      return true; // Stop propagation
    }
    return false; // Continue propagation
  }
};
```

### Layout Management

```cpp
auto layout = std::make_unique<FlexLayout<Panel>>();
layout->setDirection(FlexLayout<Panel>::Direction::ROW);
layout->setGap(10);
panel->setLayoutManager(std::move(layout));
```

### Resize Behavior

```cpp
// Unified behavior
component->setResizeBehavior(Component::ResizeBehavior::FILL);

// Per-axis behavior
component->setAxisResizeBehavior(
  Component::AxisResizeBehavior::FIXED,   // Horizontal
  Component::AxisResizeBehavior::FILL     // Vertical
);

// With constraints
Component::ResponsiveConstraints constraints;
constraints.minWidth = 200;
constraints.maxWidth = 600;
component->setConstraints(constraints);
```

## Thread Safety

- **Scene**: Single-threaded, all UI operations must occur on the main thread
- **ThemeManager**: Thread-safe singleton with mutex protection
- **Components**: Not thread-safe, designed for single-threaded use

## Performance Considerations

- **CRTP**: Zero-overhead polymorphism for layouts
- **Coordinate caching**: Global coordinates cached and automatically invalidated
- **Layout caching**: Layouts only recalculate when invalidated
- **Event propagation**: Efficient hit testing and early termination

## Examples

Each API page includes usage examples. For complete working examples, see:
- `examples/basic/` - Simple focused examples
- `examples/intermediate/` - Complex composition patterns
- `examples/advanced/` - Custom components, layouts, and optimization

## Building Applications

```cpp
// 1. Initialize window system (GLFW example)
glfwInit();
GLFWwindow* window = glfwCreateWindow(800, 600, "App", nullptr, nullptr);

// 2. Create adapters
auto windowAdapter = std::make_unique<GLFWWindowAdapter>(window);
auto renderer = std::make_unique<SimpleOpenGLRenderer>();
renderer->initialize(800, 600);

// 3. Create scene
auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

// 4. Build UI
auto button = create<Button>("Hello").build();
scene->addChild(std::move(button));
scene->attach();

// 5. Main loop
while (!windowAdapter->shouldClose()) {
  glfwPollEvents();

  if (renderer->beginFrame()) {
    renderer->clear(0.1f, 0.1f, 0.12f, 1.0f);
    scene->updateAll(0.016);
    scene->renderAll();
    renderer->endFrame();
  }

  glfwSwapBuffers(window);
}
```

## Platform Integration

Prong is platform-agnostic. You need to provide:
- **Window adapter** (IWindow): GLFW, SDL, or native
- **Renderer** (IRenderer): OpenGL, Vulkan, or custom
- **Optional**: IClipboard, IKeyboard for TextInput

See `examples/adapters/` for GLFW reference implementations.

## Version

This documentation is for Prong v1.0.0.

## Contributing

See the main README.md for contribution guidelines.
