# Prong API Quick Reference

Condensed reference for all Prong APIs. See individual files for detailed documentation.

## Core Classes

### Component

Base class for all UI elements.

```cpp
class Component {
  virtual void update(double deltaTime) = 0;
  virtual void render() = 0;
  void setPosition(int x, int y);
  void setSize(int width, int height);
  void addChild(std::unique_ptr<Component> child);
  bool handleEvent(const Event& event);
  void setResizeBehavior(ResizeBehavior behavior);
};
```

### Scene

Root container and event coordinator.

```cpp
class Scene : public Component {
  Scene(IWindow* window, IRenderer* renderer);
  void attach();  // Start receiving events
  void detach();  // Stop receiving events
  void handleResize(int newWidth, int newHeight);
};
```

### ComponentBuilder

Fluent API for component construction.

```cpp
auto component = create<ComponentType>(/* constructor args */)
                   .withSize(width, height)
                   .withPosition(x, y)
                   .withVisible(bool)
                   .build();
```

### Event

Unified event structure.

```cpp
struct Event {
  enum class Type { MOUSE_PRESS, MOUSE_RELEASE, MOUSE_MOVE, MOUSE_SCROLL, KEY_PRESS, KEY_RELEASE, CHAR_INPUT };
  Type type;
  int localX, localY;  // Mouse position in local coordinates
  int button;          // Mouse button
  int key, mods;       // Keyboard
  unsigned int codepoint;  // Character input
  double scrollX, scrollY; // Scroll delta
};
```

## UI Components

### Button

```cpp
auto button = create<Button>("Label")
                .withSize(200, 60)
                .withClickCallback([]() { /* clicked */ })
                .build();
```

### Panel

```cpp
auto panel = create<Panel>()
               .withSize(400, 300)
               .withBackgroundColor(r, g, b, a)
               .build();
```

### TextInput

```cpp
auto input = create<TextInput>()
               .withPlaceholder("Enter text...")
               .withTextChangeCallback([](const std::string& text) { /* text changed */ })
               .build();
input->setClipboard(clipboardPtr);  // Required for copy/paste
input->setKeyboard(keyboardPtr);    // Required for key handling
```

### ListBox

```cpp
auto listBox = create<ListBox>()
                 .withSize(300, 400)
                 .withSelectionCallback([](int index) { /* item selected */ })
                 .build();
listBox->addItem("Item 1");
listBox->addItem("Item 2");
```

### Dialog

```cpp
auto dialog = Dialog::create(renderer, "Title", "Message", DialogType::OK_CANCEL);
dialog->setButtonCallback([](Dialog::Result result) {
  if (result == Dialog::Result::OK) { /* OK clicked */ }
});
```

### Toolbar

```cpp
auto toolbar = create<Toolbar>()
                 .withSize(800, 50)
                 .build();
toolbar->addItem("File", []() { /* File clicked */ });
toolbar->addItem("Edit", []() { /* Edit clicked */ });
```

### Viewport

```cpp
auto viewport = create<Viewport>()
                  .withSize(600, 400)
                  .build();
// Viewport can contain content larger than its size
// Automatically adds scrolling
```

## Layout Managers

### FlexLayout

Flexbox-inspired layout.

```cpp
auto layout = std::make_unique<FlexLayout<ParentType>>();
layout->setDirection(FlexLayout<ParentType>::Direction::ROW);  // or COLUMN
layout->setJustifyContent(FlexLayout<ParentType>::JustifyContent::START);  // START, CENTER, END, SPACE_BETWEEN, SPACE_AROUND
layout->setAlignItems(FlexLayout<ParentType>::AlignItems::STRETCH);  // STRETCH, START, CENTER, END
layout->setGap(10);
panel->setLayoutManager(std::move(layout));

// Per-item properties
layout->setItemProperties(childPtr, {.grow = 1.0f, .shrink = 1.0f});
```

### GridLayout

CSS Grid-inspired layout.

```cpp
auto layout = std::make_unique<GridLayout<ParentType>>();
layout->setRows(3);
layout->setColumns(4);
layout->setGap(10);
panel->setLayoutManager(std::move(layout));
```

### DockLayout

Docking panel layout.

```cpp
auto layout = std::make_unique<DockLayout<ParentType>>();
layout->setGaps(5, 5, 5, 5);  // top, bottom, left, right
panel->setLayoutManager(std::move(layout));

// Set dock position for children
layout->setItemProperties(topPanel, {.position = DockPosition::TOP});
layout->setItemProperties(centerPanel, {.position = DockPosition::CENTER});
```

Dock positions: `TOP`, `BOTTOM`, `LEFT`, `RIGHT`, `CENTER`

### StackLayout

Simple sequential layout.

```cpp
auto layout = std::make_unique<StackLayout<ParentType>>();
layout->setOrientation(StackLayout<ParentType>::Orientation::VERTICAL);  // or HORIZONTAL
layout->setSpacing(10);
panel->setLayoutManager(std::move(layout));
```

### FlowLayout

Wrapping flow layout.

```cpp
auto layout = std::make_unique<FlowLayout<ParentType>>();
layout->setOrientation(FlowLayout<ParentType>::Orientation::HORIZONTAL);
layout->setSpacing(10);
layout->setLineSpacing(15);
panel->setLayoutManager(std::move(layout));
```

## Event Handling

### Custom Event Handler

```cpp
class MyComponent : public Component {
protected:
  bool handleEventSelf(const Event& event) override {
    switch (event.type) {
      case Event::Type::MOUSE_PRESS:
        // event.localX, event.localY are relative to this component
        // event.button: 0=left, 1=right, 2=middle
        return true;  // Stop propagation

      case Event::Type::KEY_PRESS:
        if (focusState == FocusState::FOCUSED) {
          // event.key, event.mods
          return true;
        }
        break;

      case Event::Type::CHAR_INPUT:
        // event.codepoint
        return true;

      case Event::Type::MOUSE_SCROLL:
        // event.scrollX, event.scrollY
        return true;
    }
    return false;  // Continue propagation
  }
};
```

### Focus Management

```cpp
component->requestFocus();  // Request focus

if (component->getFocusState() == Component::FocusState::FOCUSED) {
  // Handle keyboard input
}
```

## Rendering

### IRenderer Interface

```cpp
class IRenderer {
  virtual bool beginFrame() = 0;
  virtual void endFrame() = 0;
  virtual void present() = 0;
  virtual void clear(float r, float g, float b, float a) = 0;

  virtual void drawRect(int x, int y, int w, int h, float r, float g, float b, float a) = 0;
  virtual void drawText(const char* text, int x, int y, float scale, float r, float g, float b, float a) = 0;

  virtual void enableScissorTest(int x, int y, int w, int h) = 0;
  virtual void disableScissorTest() = 0;

  // Optional: Texture/sprite support
  virtual unsigned int loadTexture(const char* path) = 0;
  virtual void drawSprite(unsigned int textureID, int x, int y, int w, int h, float r, float g, float b, float a) = 0;
};
```

### Custom Renderer Example

```cpp
class MyRenderer : public IRenderer {
  bool beginFrame() override {
    // Start frame, clear buffers, etc.
    return true;
  }

  void endFrame() override {
    // Finish rendering
  }

  void drawRect(int x, int y, int w, int h, float r, float g, float b, float a) override {
    // Draw filled rectangle
  }

  // Implement other methods...
};
```

## Theming

### ThemeManager

Singleton for global theme management.

```cpp
// Get instance
auto& theme = ThemeManager::getInstance();

// Get colors
Color bg = theme.getColor(ColorRole::BACKGROUND);
Color text = theme.getColor(ColorRole::TEXT);

// Set colors
theme.setColor(ColorRole::BUTTON_BACKGROUND, Color(0.3f, 0.5f, 0.8f));

// Callbacks
theme.setThemeChangeCallback([]() {
  // Theme changed, update components
});
```

Color roles: `BACKGROUND`, `TEXT`, `BUTTON_BACKGROUND`, `BUTTON_BACKGROUND_HOVER`, `BUTTON_BACKGROUND_PRESSED`, `BUTTON_TEXT`, `PANEL_BACKGROUND`, `INPUT_BACKGROUND`, `INPUT_TEXT`, etc.

### Color

```cpp
Color color(0.5f, 0.8f, 0.3f, 1.0f);  // RGBA [0.0, 1.0]
color.setRGB(r, g, b);
color.setAlpha(a);
float r, g, b, a;
color.getRGBA(r, g, b, a);
```

## Window Abstraction

### IWindow

```cpp
class IWindow {
  virtual bool shouldClose() const = 0;
  virtual void pollEvents() = 0;
  virtual void getSize(int& width, int& height) const = 0;
  virtual void setEventCallback(EventCallback callback) = 0;
  virtual bool isKeyPressed(int key) const = 0;
};
```

### IClipboard (for TextInput)

```cpp
class IClipboard {
  virtual std::string getString() = 0;
  virtual void setString(const std::string& text) = 0;
};
```

### IKeyboard (for TextInput)

```cpp
class IKeyboard {
  virtual Key mapPlatformKey(int platformKey) = 0;
};
```

Platform-agnostic Key enum: `A`-`Z`, `SPACE`, `ENTER`, `BACKSPACE`, `DELETE`, `LEFT`, `RIGHT`, `HOME`, `END`, etc.

## Resize Behavior

### Unified Behavior

```cpp
component->setResizeBehavior(Component::ResizeBehavior::FIXED);   // Keep original size
component->setResizeBehavior(Component::ResizeBehavior::SCALE);   // Scale proportionally
component->setResizeBehavior(Component::ResizeBehavior::FILL);    // Fill available space
component->setResizeBehavior(Component::ResizeBehavior::MAINTAIN_ASPECT);  // Scale maintaining aspect
```

### Per-Axis Behavior

```cpp
component->setAxisResizeBehavior(
  Component::AxisResizeBehavior::FIXED,  // Horizontal
  Component::AxisResizeBehavior::FILL    // Vertical
);
```

### Constraints

```cpp
Component::ResponsiveConstraints constraints;
constraints.minWidth = 200;
constraints.maxWidth = 600;
constraints.minHeight = 150;
constraints.maxHeight = 450;
component->setConstraints(constraints);
```

## Custom Components and Layouts

### Custom Component

```cpp
class ColorPicker : public Component {
public:
  ColorPicker(IRenderer* renderer) : Component(renderer, "ColorPicker") {}

  void update(double deltaTime) override {
    // Update logic
  }

  void render() override {
    int gx = getGlobalX();
    int gy = getGlobalY();
    // Render using renderer->drawRect(), etc.
  }

protected:
  bool handleEventSelf(const Event& event) override {
    // Handle events
    return false;
  }
};
```

### Custom Layout Manager

```cpp
template<typename ParentT>
class CircularLayout : public LayoutManager<CircularLayout<ParentT>> {
  Dimensions measureLayout(const std::vector<Component*>& components) override {
    // Return required size
    return {diameter, diameter};
  }

  void layout(std::vector<Component*>& components, const Dimensions& available) override {
    // Position components in a circle
    float angleStep = 2.0f * M_PI / components.size();
    for (size_t i = 0; i < components.size(); ++i) {
      float angle = i * angleStep;
      int x = centerX + radius * cos(angle);
      int y = centerY + radius * sin(angle);
      components[i]->setPosition(x, y);
    }
  }
};
```

## Common Patterns

### Application Structure

```cpp
int main() {
  // 1. Initialize window system
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(800, 600, "App", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // 2. Create adapters
  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(window);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();
  renderer->initialize(800, 600);

  // 3. Create scene
  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // 4. Build UI
  auto panel = create<Panel>().withSize(800, 600).build();
  auto button = create<Button>("Click").build();
  panel->addChild(std::move(button));
  scene->addChild(std::move(panel));
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

  // 6. Cleanup
  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
```

### Three-Panel Layout

```cpp
auto root = create<Panel>().withSize(1200, 800).build();
auto layout = std::make_unique<FlexLayout<Panel>>();
layout->setDirection(FlexLayout<Panel>::Direction::ROW);
root->setLayoutManager(std::move(layout));

auto left = create<Panel>().withSize(250, 800).build();
left->setAxisResizeBehavior(AxisResizeBehavior::FIXED, AxisResizeBehavior::FILL);

auto center = create<Panel>().withSize(700, 800).build();
center->setResizeBehavior(ResizeBehavior::FILL);

auto right = create<Panel>().withSize(250, 800).build();
right->setAxisResizeBehavior(AxisResizeBehavior::FIXED, AxisResizeBehavior::FILL);

root->addChild(std::move(left));
root->addChild(std::move(center));
root->addChild(std::move(right));
```

### Dynamic UI

```cpp
// Add components
parent->addChild(createNewComponent());
parent->invalidateLayout();

// Remove components
parent->removeChild(childPtr);
parent->invalidateLayout();

// Change layout
auto newLayout = std::make_unique<GridLayout<Panel>>();
parent->setLayoutManager(std::move(newLayout));
parent->invalidateLayout();
```

## Performance Tips

1. **Batch operations**: Multiple changes, then one `invalidateLayout()`
2. **Use CRTP**: Zero overhead for custom layouts
3. **Cache coordinates**: Global coordinates automatically cached
4. **Stop propagation**: Return `true` from event handlers when appropriate
5. **Visibility over removal**: Hide components instead of removing if they'll return
6. **Choose simple layouts**: Use simplest layout manager that meets needs
7. **Profile first**: Measure before optimizing

## Thread Safety

- **Single-threaded**: All UI operations on main thread
- **ThemeManager**: Thread-safe (only thread-safe component)
- **Scene**: Not thread-safe
- **Components**: Not thread-safe

## Examples

- `examples/basic/` - Simple focused examples (8 examples)
- `examples/intermediate/` - Complex patterns (4 examples)
- `examples/advanced/` - Custom components, layouts, optimization (4 examples)
- `examples/demo_app/` - Full application demo
