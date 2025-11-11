# Component

Base class for all UI elements in Prong. All components inherit from this class.

## Header

```cpp
#include <bombfork/prong/core/component.h>
```

## Class Declaration

```cpp
class Component {
public:
  // Construction
  explicit Component(IRenderer* renderer = nullptr, const std::string& debugName = "Component");
  virtual ~Component() = default;

  // Pure virtual methods (must be implemented by derived classes)
  virtual void update(double deltaTime) = 0;
  virtual void render() = 0;

  // Geometry
  virtual void setBounds(int x, int y, int width, int height);
  virtual void setPosition(int x, int y);
  virtual void setSize(int width, int height);
  virtual void getPosition(int& x, int& y) const;
  virtual void getSize(int& width, int& height) const;
  virtual void getBounds(int& x, int& y, int& width, int& height) const;

  // Global coordinates (for rendering and hit testing)
  void getGlobalPosition(int& x, int& y) const;
  void getGlobalBounds(int& x, int& y, int& width, int& height) const;

  // Coordinate conversion
  void localToGlobal(int localX, int localY, int& globalX, int& globalY) const;
  void globalToLocal(int globalX, int globalY, int& localX, int& localY) const;

  // Hit testing
  bool containsGlobal(int globalX, int globalY) const;

  // Parent/child relationships
  void addChild(std::unique_ptr<Component> child);
  void removeChild(Component* child);
  std::vector<std::unique_ptr<Component>>& getChildren();
  const std::vector<std::unique_ptr<Component>>& getChildren() const;
  Component* getParent() const;

  // Visibility and interaction
  void setVisible(bool visible);
  bool isVisible() const;
  void setEnabled(bool enabled);
  bool isEnabled() const;

  // Focus management
  enum class FocusState { NONE, FOCUSABLE, FOCUSED };
  void requestFocus();
  FocusState getFocusState() const;
  void setFocusState(FocusState state);

  // Resize behavior
  enum class ResizeBehavior { FIXED, SCALE, FILL, MAINTAIN_ASPECT };
  enum class AxisResizeBehavior { FIXED, SCALE, FILL };

  void setResizeBehavior(ResizeBehavior behavior);
  void setAxisResizeBehavior(AxisResizeBehavior horizontal, AxisResizeBehavior vertical);

  struct ResponsiveConstraints {
    int minWidth = 0, maxWidth = INT_MAX;
    int minHeight = 0, maxHeight = INT_MAX;
  };
  void setConstraints(const ResponsiveConstraints& constraints);

  // Layout management
  template<typename LayoutT>
  void setLayoutManager(std::unique_ptr<LayoutT> manager);
  void invalidateLayout();

  // Event handling
  bool handleEvent(const core::Event& event);

  // Renderer
  void setRenderer(IRenderer* renderer);
  IRenderer* getRenderer() const;

  // Recursive operations
  void updateAll(double deltaTime);
  void renderAll();

protected:
  // Override in derived classes for custom event handling
  virtual bool handleEventSelf(const core::Event& event);

  // Protected accessors for rendering
  int getGlobalX() const;
  int getGlobalY() const;

  // Members
  IRenderer* renderer;
  int localX, localY, width, height;
  bool visible, enabled;
  FocusState focusState;
  bool isCurrentlyHovered;
  Component* parent;
  std::vector<std::unique_ptr<Component>> children;
};
```

## Key Concepts

### Coordinate Systems

- **Local coordinates**: Position relative to parent. Use `setPosition()`, `getPosition()`.
- **Global coordinates**: Absolute screen position. Use `getGlobalPosition()`, `getGlobalBounds()`.
- For rendering, use `getGlobalX()` and `getGlobalY()` protected helpers.

### Ownership Model

- Parent owns children via `std::unique_ptr`
- Use `addChild(std::move(child))` to transfer ownership
- Children destroyed automatically when parent destroyed
- Use `removeChild(ptr)` for explicit removal

### Event Handling

Override `handleEventSelf()` in derived classes:
```cpp
bool handleEventSelf(const Event& event) override {
  if (event.type == Event::Type::MOUSE_PRESS) {
    // Handle event
    return true; // Stop propagation
  }
  return false; // Continue propagation
}
```

Events are in **local coordinates** relative to this component.

### Resize Behavior

**Unified (both axes same)**:
```cpp
component->setResizeBehavior(Component::ResizeBehavior::FILL);
```

**Per-axis (independent control)**:
```cpp
component->setAxisResizeBehavior(
  Component::AxisResizeBehavior::FIXED,
  Component::AxisResizeBehavior::FILL
);
```

**With constraints**:
```cpp
Component::ResponsiveConstraints constraints;
constraints.minWidth = 200;
constraints.maxWidth = 600;
component->setConstraints(constraints);
```

## Common Usage Patterns

### Creating Custom Components

```cpp
class MyComponent : public Component {
public:
  MyComponent(IRenderer* renderer) : Component(renderer, "MyComponent") {
    width = 200;
    height = 100;
  }

  void update(double deltaTime) override {
    // Update logic
  }

  void render() override {
    if (!renderer) return;
    int gx = getGlobalX();
    int gy = getGlobalY();
    renderer->drawRect(gx, gy, width, height, 1.0f, 1.0f, 1.0f, 1.0f);
  }

protected:
  bool handleEventSelf(const Event& event) override {
    // Custom event handling
    return false;
  }
};
```

### Parent-Child Hierarchy

```cpp
auto parent = std::make_unique<Panel>(renderer);
auto child1 = std::make_unique<Button>("Button 1");
auto child2 = std::make_unique<Button>("Button 2");

child1->setPosition(10, 10);
child2->setPosition(10, 80);

parent->addChild(std::move(child1));
parent->addChild(std::move(child2));
```

### Hit Testing

```cpp
int mouseX = 150, mouseY = 200; // Screen coordinates
if (component->containsGlobal(mouseX, mouseY)) {
  // Mouse is over this component
}
```

### Coordinate Conversion

```cpp
// Local to global
int globalX, globalY;
component->localToGlobal(10, 20, globalX, globalY);

// Global to local
int localX, localY;
component->globalToLocal(globalX, globalY, localX, localY);
```

## Lifecycle

1. **Construction**: Create component, set initial properties
2. **Add to parent**: Transfer ownership via `addChild()`
3. **Update loop**: `updateAll()` called each frame
4. **Render loop**: `renderAll()` called each frame
5. **Event handling**: `handleEvent()` called when events occur
6. **Removal**: `removeChild()` or parent destruction

## Best Practices

- Always check `renderer` is not null in `render()`
- Use `getGlobalX()` and `getGlobalY()` for rendering positions
- Return `true` from `handleEventSelf()` to stop event propagation
- Call `invalidateLayout()` after modifying children or properties
- Use local coordinates when positioning children
- Use global coordinates only for rendering and hit testing

## Thread Safety

Not thread-safe. All operations must occur on the main UI thread.

## Related

- [Scene](Scene.md) - Root container
- [ComponentBuilder](ComponentBuilder.md) - Fluent construction API
- [Event](Event.md) - Event structure
- [Panel](../components/Panel.md) - Basic container component
