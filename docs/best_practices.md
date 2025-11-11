# Prong Framework Best Practices

## Overview

This guide provides practical guidelines for building effective, maintainable, and performant UIs with the Prong framework. Following these practices will help you avoid common pitfalls and make the most of Prong's features.

## Scene Organization

### Use Scene as Root Container

Always use `Scene` as the top-level container for your UI hierarchy. Don't create standalone component trees without a scene.

**Good:**

```cpp
Scene scene(window, renderer);
scene.attach();

auto mainPanel = create<Panel<>>().build();
scene.addChild(std::move(mainPanel));

// Main loop
while (!window->shouldClose()) {
    scene.updateAll(deltaTime);
    scene.renderAll();
    scene.present();
}
```

**Bad:**

```cpp
// Don't manage component trees without Scene
auto mainPanel = create<Panel<>>(renderer).build();
mainPanel->updateAll(deltaTime);  // Missing scene features
mainPanel->renderAll();           // No window integration
```

### Separate Scene Logic into Classes

For complex UIs, create custom Scene subclasses to encapsulate logic:

```cpp
class GameScene : public Scene {
private:
    std::unique_ptr<Panel<FlexLayout>> hud;
    std::unique_ptr<Viewport> gameViewport;
    std::unique_ptr<Dialog> pauseDialog;

public:
    GameScene(IWindow* window, IRenderer* renderer)
        : Scene(window, renderer) {
        buildUI();
    }

    void buildUI() {
        // Create HUD
        hud = create<Panel<FlexLayout>>()
            .withSize(800, 60)
            .build();
        hud->getLayout().configure({
            .direction = FlexDirection::ROW,
            .justify = FlexJustify::SPACE_BETWEEN
        });

        // Create viewport
        gameViewport = create<Viewport>()
            .withSize(800, 600)
            .build();

        addChild(std::move(hud));
        addChild(std::move(gameViewport));
    }

    void showPauseMenu() {
        if (!pauseDialog) {
            pauseDialog = create<Dialog>()
                .withSize(300, 200)
                .build();
            addChild(std::move(pauseDialog));
        }
        pauseDialog->setVisible(true);
    }
};
```

### Use Scene Switching for Different States

Create separate scenes for different application states:

```cpp
class Application {
private:
    std::unique_ptr<Scene> currentScene;
    IWindow* window;
    IRenderer* renderer;

public:
    void switchToMainMenu() {
        if (currentScene) {
            currentScene->detach();
        }
        currentScene = std::make_unique<MainMenuScene>(window, renderer);
        currentScene->attach();
    }

    void switchToGame() {
        if (currentScene) {
            currentScene->detach();
        }
        currentScene = std::make_unique<GameScene>(window, renderer);
        currentScene->attach();
    }

    void run() {
        while (!window->shouldClose()) {
            currentScene->updateAll(deltaTime);
            currentScene->renderAll();
            currentScene->present();
        }
    }
};
```

## Layout Selection Guide

### When to Use Each Layout

**FlexLayout** - Use for:

- Toolbars and navigation bars
- Button groups with flexible spacing
- Responsive lists that adapt to container size
- Complex alignment requirements
- Components that need grow/shrink behavior

**GridLayout** - Use for:

- Icon grids and image galleries
- Form layouts with labels and inputs
- Calculator-style button grids
- Uniform card layouts
- Any regular grid structure

**DockLayout** - Use for:

- Application main windows (IDE-style)
- Dashboards with fixed panels
- Resizable side panels
- Top/bottom toolbars with center content

**StackLayout** - Use for:

- Simple vertical menus
- Stacked cards or panels
- Linear lists with consistent spacing
- When you just need simple stacking (simpler than FlexLayout)

**FlowLayout** - Use for:

- Tag clouds
- Wrapping button groups
- Image galleries with automatic wrapping
- Dynamic content with unknown item count

### Layout Decision Tree

```text
Need multiple regions (top/bottom/sides)?
├─ Yes → DockLayout
└─ No
   │
   └─ Need grid structure?
      ├─ Yes → GridLayout
      └─ No
         │
         └─ Need automatic wrapping?
            ├─ Yes → FlowLayout
            └─ No
               │
               └─ Need flexible grow/shrink/justify?
                  ├─ Yes → FlexLayout
                  └─ No → StackLayout (simplest)
```

### Avoid Layout Overkill

Don't use complex layouts when simple positioning suffices:

```cpp
// BAD: Overkill for 2 buttons
auto panel = create<Panel<FlexLayout>>().build();
panel->getLayout().configure({...});
panel->addChild(create<Button>("OK").build());
panel->addChild(create<Button>("Cancel").build());

// GOOD: Manual positioning for simple cases
auto panel = create<Panel<>>().build();
auto okBtn = create<Button>("OK")
    .withPosition(10, 10)
    .withSize(100, 40)
    .build();
auto cancelBtn = create<Button>("Cancel")
    .withPosition(120, 10)
    .withSize(100, 40)
    .build();
panel->addChild(std::move(okBtn));
panel->addChild(std::move(cancelBtn));
```

## Component Creation Patterns

### Use ComponentBuilder for New Components

The builder pattern provides cleaner code and better defaults:

**Good:**

```cpp
auto button = create<Button>("Submit")
    .withSize(100, 40)
    .withPosition(10, 10)
    .withClickCallback([]() { submit(); })
    .build();
```

**Acceptable (when you need more control):**

```cpp
auto button = std::make_unique<Button>(renderer, "Submit");
button->setSize(100, 40);
button->setPosition(10, 10);
button->setClickCallback([]() { submit(); });
```

### Keep References to Interactive Components

Store pointers/references when components need to interact:

```cpp
class LoginDialog : public Dialog {
private:
    TextInput* usernameInput;  // Raw pointer (owned by Dialog)
    TextInput* passwordInput;
    Button* loginButton;

public:
    void buildUI() {
        auto username = create<TextInput>()
            .withPlaceholder("Username")
            .build();
        usernameInput = username.get();  // Store pointer before moving

        auto password = create<TextInput>()
            .withPlaceholder("Password")
            .build();
        passwordInput = password.get();

        auto login = create<Button>("Login")
            .withClickCallback([this]() { onLoginClick(); })
            .build();
        loginButton = login.get();

        addChild(std::move(username));
        addChild(std::move(password));
        addChild(std::move(login));
    }

    void onLoginClick() {
        std::string user = usernameInput->getText();
        std::string pass = passwordInput->getText();
        // Validate and login
    }
};
```

### Use Shared Pointers for Complex Dependencies

When multiple components need bidirectional references:

```cpp
class SearchPanel {
private:
    std::shared_ptr<TextInput> searchBox;
    std::shared_ptr<ListBox> results;
    std::shared_ptr<Button> clearButton;

public:
    void buildUI() {
        searchBox = std::shared_ptr<TextInput>(
            create<TextInput>()
                .withPlaceholder("Search...")
                .build()
        );

        results = std::shared_ptr<ListBox>(
            create<ListBox>().build()
        );

        clearButton = std::shared_ptr<Button>(
            create<Button>("Clear")
                .build()
        );

        // SearchBox updates results
        searchBox->setOnTextChanged([this, results = results](const std::string& text) {
            results->setItems(search(text));
        });

        // Clear button affects both
        clearButton->setClickCallback([searchBox = searchBox, results = results]() {
            searchBox->setText("");
            results->clear();
        });

        // Add to parent (shared_ptr keeps them alive)
        parent->addChild(std::move(searchBox));
        parent->addChild(std::move(results));
        parent->addChild(std::move(clearButton));
    }
};
```

### Component Factory Functions

Create factory functions for commonly used component configurations:

```cpp
namespace ui {

std::unique_ptr<Button> createPrimaryButton(const std::string& text) {
    return create<Button>(text)
        .withSize(120, 40)
        .build();
}

std::unique_ptr<Button> createSecondaryButton(const std::string& text) {
    return create<Button>(text)
        .withSize(100, 35)
        .build();
}

std::unique_ptr<Panel<FlexLayout>> createToolbar() {
    auto toolbar = create<Panel<FlexLayout>>()
        .withSize(800, 50)
        .build();
    toolbar->getLayout().configure({
        .direction = FlexDirection::ROW,
        .justify = FlexJustify::START,
        .align = FlexAlign::CENTER,
        .gap = 5.0f
    });
    return toolbar;
}

std::unique_ptr<Panel<StackLayout>> createVerticalMenu() {
    auto menu = create<Panel<StackLayout>>()
        .withSize(200, 400)
        .build();
    menu->getLayout().configure({
        .orientation = StackOrientation::VERTICAL,
        .alignment = StackAlignment::STRETCH,
        .spacing = 5.0f
    });
    return menu;
}

}  // namespace ui

// Usage
auto toolbar = ui::createToolbar();
toolbar->addChild(ui::createPrimaryButton("Save"));
toolbar->addChild(ui::createSecondaryButton("Cancel"));
```

## Coordinate System Best Practices

### Always Use Local Coordinates for Positioning

When positioning children, use local coordinates relative to parent:

**Good:**

```cpp
void MyPanel::layoutChildren() {
    // Position relative to this panel's origin
    child1->setPosition(10, 10);     // 10px from panel's top-left
    child2->setPosition(10, 60);     // 10px from left, 60px from top
}
```

**Bad:**

```cpp
void MyPanel::layoutChildren() {
    int globalX = getGlobalX();
    int globalY = getGlobalY();
    // Don't use global coordinates for positioning!
    child1->setPosition(globalX + 10, globalY + 10);  // Wrong!
}
```

### Always Use Global Coordinates for Rendering

In `render()` methods, always use global coordinates:

**Good:**

```cpp
void MyComponent::render() override {
    int x = getGlobalX();  // Screen-space X
    int y = getGlobalY();  // Screen-space Y

    renderer->drawRect(x, y, width, height, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer->drawText(x + 5, y + 15, "Text", 1.0f, 1.0f, 1.0f, 1.0f);
}
```

**Bad:**

```cpp
void MyComponent::render() override {
    // Don't use localX/localY for rendering!
    renderer->drawRect(localX, localY, width, height, ...);  // Wrong!
}
```

### Never Store Global Coordinates

Global coordinates can become stale. Always compute on-demand:

**Bad:**

```cpp
class MyComponent : public Component {
    int storedGlobalX, storedGlobalY;  // Will become stale!

    void someMethod() {
        getGlobalPosition(storedGlobalX, storedGlobalY);
    }

    void render() override {
        // May use outdated coordinates if parent moved
        renderer->drawRect(storedGlobalX, storedGlobalY, ...);
    }
};
```

**Good:**

```cpp
class MyComponent : public Component {
    void render() override {
        // Always compute fresh (uses cache internally)
        int x = getGlobalX();
        int y = getGlobalY();
        renderer->drawRect(x, y, ...);
    }
};
```

### Let Layout Managers Handle Positioning

Don't manually position children if you're using a layout manager:

**Bad:**

```cpp
auto panel = create<Panel<FlexLayout>>().build();
panel->getLayout().configure({...});

// Don't manually position - layout will override!
auto btn = create<Button>("Click")
    .withPosition(50, 50)  // This will be overridden
    .build();
panel->addChild(std::move(btn));
```

**Good:**

```cpp
auto panel = create<Panel<FlexLayout>>().build();
panel->getLayout().configure({...});

// Let layout handle positioning
auto btn = create<Button>("Click")
    .withSize(100, 40)  // Size is respected
    .build();
panel->addChild(std::move(btn));
```

## Event Handling Patterns

### Return True to Consume Events

Return `true` from event handlers to stop propagation:

```cpp
bool MyComponent::handleEventSelf(const Event& event) override {
    if (event.type == EventType::MOUSE_PRESS) {
        // Handle click
        onClick();
        return true;  // Event consumed, stop propagation
    }
    return false;  // Event not handled, continue to other components
}
```

### Use Hit Testing Carefully

The default hit testing is usually sufficient:

```cpp
// Default implementation (in Component) - usually correct
bool containsGlobal(int globalX, int globalY) const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    return globalX >= gx && globalX < gx + width &&
           globalY >= gy && globalY < gy + height;
}
```

Only override for custom shapes:

```cpp
// Custom circular hit testing
bool CircularButton::containsGlobal(int globalX, int globalY) const override {
    int gx, gy;
    getGlobalPosition(gx, gy);

    int centerX = gx + width / 2;
    int centerY = gy + height / 2;
    int radius = width / 2;

    int dx = globalX - centerX;
    int dy = globalY - centerY;
    return (dx * dx + dy * dy) <= (radius * radius);
}
```

### Disable Event Handling When Appropriate

Disable components to prevent event handling:

```cpp
button->setEnabled(false);  // Won't receive events
panel->setVisible(false);   // Won't receive events
```

### Use Focus Callbacks for State Changes

Subscribe to focus changes rather than polling:

```cpp
textInput->setFocusCallback([textInput](bool hasFocus) {
    if (hasFocus) {
        std::cout << "Input focused" << std::endl;
        textInput->selectAll();
    } else {
        std::cout << "Input unfocused" << std::endl;
        textInput->validate();
    }
});
```

### Handle All Event Types You Care About

Don't assume event types - check explicitly:

```cpp
bool MyComponent::handleEventSelf(const Event& event) override {
    switch (event.type) {
        case EventType::MOUSE_PRESS:
            onMouseDown(event.mouseX, event.mouseY);
            return true;

        case EventType::MOUSE_RELEASE:
            onMouseUp(event.mouseX, event.mouseY);
            return true;

        case EventType::MOUSE_MOVE:
            onMouseMove(event.mouseX, event.mouseY);
            return false;  // Don't consume move events

        case EventType::KEY_PRESS:
            if (event.key == Key::ENTER) {
                onEnter();
                return true;
            }
            break;

        default:
            break;
    }
    return false;
}
```

## Resize Behavior Guidelines

### Choose Appropriate Resize Behavior

Match resize behavior to component purpose:

```cpp
// Fixed size - decorative elements, icons
icon->setResizeBehavior(Component::ResizeBehavior::FIXED);

// Fill - main content areas
contentPanel->setResizeBehavior(Component::ResizeBehavior::FILL);

// Maintain aspect - images, videos
imagePanel->setResizeBehavior(Component::ResizeBehavior::MAINTAIN_ASPECT);

// Scale - proportional scaling
scalablePanel->setResizeBehavior(Component::ResizeBehavior::SCALE);
```

### Use Per-Axis Behavior with FlexLayout

For panels in FlexLayout, use per-axis resize behavior:

```cpp
auto layout = std::make_shared<FlexLayout>();
layout->configure({
    .direction = FlexDirection::ROW
});

// Left sidebar - fixed width, fill height
auto leftPanel = create<Panel<>>()
    .withSize(200, 600)
    .build();
leftPanel->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FIXED,   // Keep 200px width
    Component::AxisResizeBehavior::FILL     // Fill available height
);

// Center content - fill both dimensions
auto centerPanel = create<Panel<>>()
    .withSize(600, 600)
    .build();
centerPanel->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FILL,    // Grow with layout
    Component::AxisResizeBehavior::FILL
);

auto container = create<Panel<>>()
    .withLayout(layout)
    .withChildren(
        std::move(leftPanel),
        std::move(centerPanel)
    )
    .build();
```

### Set Constraints for Bounded Resizing

Use responsive constraints to limit component sizes:

```cpp
Component::ResponsiveConstraints constraints;
constraints.minWidth = 200;
constraints.maxWidth = 600;
constraints.minHeight = 150;
constraints.maxHeight = 450;

resizablePanel->setConstraints(constraints);
resizablePanel->setResizeBehavior(Component::ResizeBehavior::SCALE);
// Will scale but never exceed constraints
```

### Handle Window Resize in Scene

Respond to window resize events at the scene level:

```cpp
class MyScene : public Scene {
public:
    void onWindowResize(int newWidth, int newHeight) override {
        // Scene automatically resizes to window
        setBounds(0, 0, newWidth, newHeight);

        // Adjust child layouts if needed
        mainPanel->setSize(newWidth, newHeight - 50);  // Leave space for toolbar
    }
};

// In window callback
window->setResizeCallback([scene](int w, int h) {
    scene->onWindowResize(w, h);
});
```

## Performance Tips

### Minimize Layout Recalculations

Layout calculations can be expensive for large hierarchies:

```cpp
// BAD: Invalidates layout multiple times
for (int i = 0; i < 100; i++) {
    panel->addChild(create<Button>(std::to_string(i)).build());
    // Layout invalidated 100 times
}

// GOOD: Batch children, layout calculated once before render
std::vector<std::unique_ptr<Component>> children;
for (int i = 0; i < 100; i++) {
    children.push_back(create<Button>(std::to_string(i)).build());
}
for (auto& child : children) {
    panel->addChild(std::move(child));
}
```

### Cache Layout References

Don't repeatedly call `getLayout()`:

```cpp
// BAD: Repeated lookups
panel->getLayout().configure({...});
panel->getLayout().setItemProperties({...});
panel->getLayout().performLayout();

// GOOD: Cache reference
auto& layout = panel->getLayout();
layout.configure({...});
layout.setItemProperties({...});
layout.performLayout();
```

### Avoid Unnecessary Position Updates

Only update positions when they actually change:

```cpp
// BAD: Updates every frame even if not moving
void update(float dt) override {
    setPosition(x, y);  // Same position, but invalidates cache!
}

// GOOD: Only update when position changes
void update(float dt) override {
    int newX = calculateX();
    int newY = calculateY();

    int currentX, currentY;
    getPosition(currentX, currentY);

    if (newX != currentX || newY != currentY) {
        setPosition(newX, newY);  // Only update if changed
    }
}
```

### Use Batch Rendering When Possible

For many sprites, use batch rendering:

```cpp
// BAD: Individual sprite calls
for (const auto& sprite : sprites) {
    renderer->drawSprite(sprite.x, sprite.y, sprite.w, sprite.h,
                        sprite.textureId, ...);
}

// GOOD: Single batch call
std::vector<IRenderer::SpriteData> spriteBatch;
for (const auto& sprite : sprites) {
    spriteBatch.push_back({sprite.x, sprite.y, ...});
}
renderer->drawSprites(spriteBatch);
```

### Disable Invisible Components

Components off-screen or hidden don't need updates:

```cpp
void update(float dt) override {
    if (!visible) {
        return;  // Skip update for invisible components
    }

    // Normal update logic
    updateAnimation(dt);
}
```

### Profile Before Optimizing

Use debug names to identify performance bottlenecks:

```cpp
auto heavyPanel = create<Panel<>>()
    .withDebugName("HeavyPanel")
    .build();

// In profiling code
void Component::render() override {
    auto start = std::chrono::high_resolution_clock::now();

    // Render logic

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << debugName << " render: " << duration.count() << "μs" << std::endl;
}
```

## Code Style

### Consistent Naming

Use clear, consistent names for components:

```cpp
// Good naming
auto loginDialog = create<Dialog>().build();
auto submitButton = create<Button>("Submit").build();
auto usernameInput = create<TextInput>().build();

// Avoid generic names when stored
auto dialog = create<Dialog>().build();    // Which dialog?
auto button = create<Button>("OK").build();  // Which button?
```

### Use Descriptive Debug Names

Set debug names for complex hierarchies:

```cpp
auto mainPanel = create<Panel<>>()
    .withDebugName("MainPanel")
    .build();

auto toolbar = create<Panel<FlexLayout>>()
    .withDebugName("TopToolbar")
    .build();

auto sidebar = create<Panel<StackLayout>>()
    .withDebugName("LeftSidebar")
    .build();
```

### Prefer Const Correctness

Use const where appropriate:

```cpp
class MyComponent : public Component {
private:
    const int PADDING = 10;
    const int BUTTON_HEIGHT = 40;

public:
    void render() override {
        const int x = getGlobalX();
        const int y = getGlobalY();
        const float alpha = 1.0f;

        renderer->drawRect(x, y, width, height, 1.0f, 1.0f, 1.0f, alpha);
    }
};
```

### Organize Component Methods

Follow a consistent method order:

```cpp
class MyComponent : public Component {
public:
    // Constructor
    MyComponent(IRenderer* renderer);

    // Public interface
    void setSomeProperty(int value);
    int getSomeProperty() const;

    // Component lifecycle overrides
    void update(float deltaTime) override;
    void render() override;

protected:
    // Event handling
    bool handleEventSelf(const Event& event) override;

private:
    // Private helpers
    void buildUI();
    void layoutChildren();
    void updateState();

    // Member variables
    int someProperty = 0;
    std::unique_ptr<Button> button;
};
```

### Use Designated Initializers

For configuration structs, use designated initializers:

```cpp
// Good - clear what each value means
layout->configure({
    .direction = FlexDirection::ROW,
    .justify = FlexJustify::START,
    .align = FlexAlign::CENTER,
    .gap = 10.0f,
    .wrap = false
});

// Avoid - unclear positional parameters
layout->configure({
    FlexDirection::ROW,
    FlexJustify::START,
    FlexAlign::CENTER,
    10.0f,
    false
});
```

## Common Pitfalls to Avoid

### Don't Mix Manual and Layout Positioning

**Problem:**

```cpp
auto panel = create<Panel<FlexLayout>>().build();
auto btn = create<Button>("Click")
    .withPosition(50, 50)  // This will be overridden by layout!
    .build();
panel->addChild(std::move(btn));
```

**Solution:** Choose one or the other:

```cpp
// Either use layout
auto panel = create<Panel<FlexLayout>>().build();
auto btn = create<Button>("Click").build();  // No manual position
panel->addChild(std::move(btn));

// Or manual positioning
auto panel = create<Panel<>>().build();  // No layout
auto btn = create<Button>("Click")
    .withPosition(50, 50)  // Manual position
    .build();
panel->addChild(std::move(btn));
```

### Don't Forget to Attach Scene

**Problem:**

```cpp
Scene scene(window, renderer);
// scene.attach() not called!

while (!window->shouldClose()) {
    scene.updateAll(deltaTime);
    scene.renderAll();
}
```

**Solution:**

```cpp
Scene scene(window, renderer);
scene.attach();  // Always attach!

while (!window->shouldClose()) {
    scene.updateAll(deltaTime);
    scene.renderAll();
}

scene.detach();  // Clean shutdown
```

### Don't Forget Platform Adapters for TextInput

**Problem:**

```cpp
auto textInput = create<TextInput>().build();
// No clipboard or keyboard set!
// Copy/paste won't work
```

**Solution:**

```cpp
auto adapters = GLFWAdapters::create(window);
auto textInput = create<TextInput>().build();
textInput->setClipboard(adapters.clipboard.get());
textInput->setKeyboard(adapters.keyboard.get());
// Keep adapters alive!
```

### Don't Use Raw Pointers for Ownership

**Problem:**

```cpp
Component* button = new Button(renderer, "Click");
panel->addChild(???);  // Can't transfer ownership
// Memory leak!
```

**Solution:**

```cpp
auto button = std::make_unique<Button>(renderer, "Click");
panel->addChild(std::move(button));  // Proper ownership transfer
```

### Don't Ignore Const Correctness in Event Handlers

**Problem:**

```cpp
bool handleEventSelf(const Event& event) override {
    event.mouseX = 100;  // Compile error: event is const!
    return true;
}
```

**Solution:**

```cpp
bool handleEventSelf(const Event& event) override {
    int localX = event.mouseX;  // Copy if you need to modify
    localX += 10;
    // Use localX
    return true;
}
```

## Testing Best Practices

### Use Mock Renderer for Unit Tests

Create a mock renderer for testing components without graphics:

```cpp
class MockRenderer : public IRenderer {
public:
    bool beginFrame() override { return true; }
    void endFrame() override {}
    void present() override {}
    void clear(float r, float g, float b, float a) override {}

    void drawRect(int x, int y, int w, int h, float r, float g, float b, float a) override {
        // Record for testing
        rects.push_back({x, y, w, h});
    }

    std::vector<Rect> rects;  // For verification
};

// Test
TEST(ButtonTest, RendersAtCorrectPosition) {
    MockRenderer renderer;
    Button button(&renderer, "Test");
    button.setPosition(100, 200);
    button.setSize(120, 40);

    button.render();

    ASSERT_EQ(1, renderer.rects.size());
    EXPECT_EQ(100, renderer.rects[0].x);
    EXPECT_EQ(200, renderer.rects[0].y);
}
```

### Test Event Handling

Verify event handling behavior:

```cpp
TEST(ButtonTest, HandlesClick) {
    MockRenderer renderer;
    bool clicked = false;

    Button button(&renderer, "Test");
    button.setPosition(0, 0);
    button.setSize(100, 40);
    button.setClickCallback([&clicked]() { clicked = true; });

    Event clickEvent;
    clickEvent.type = EventType::MOUSE_PRESS;
    clickEvent.mouseX = 50;  // Inside button
    clickEvent.mouseY = 20;

    bool handled = button.handleEvent(clickEvent);

    EXPECT_TRUE(handled);
    EXPECT_TRUE(clicked);
}
```

### Test Layout Calculations

Verify layout managers position components correctly:

```cpp
TEST(FlexLayoutTest, PositionsChildrenHorizontally) {
    MockRenderer renderer;

    auto layout = std::make_shared<FlexLayout>();
    layout->configure({
        .direction = FlexDirection::ROW,
        .gap = 10.0f
    });

    auto panel = create<Panel<>>()
        .withSize(400, 100)
        .withLayout(layout)
        .build();

    auto child1 = create<Button>("A").withSize(50, 40).build();
    auto child2 = create<Button>("B").withSize(50, 40).build();

    Button* c1 = child1.get();
    Button* c2 = child2.get();

    panel->addChild(std::move(child1));
    panel->addChild(std::move(child2));

    panel->performLayout();

    int x1, y1, x2, y2;
    c1->getPosition(x1, y1);
    c2->getPosition(x2, y2);

    EXPECT_EQ(0, x1);      // First child at start
    EXPECT_EQ(60, x2);     // Second child at 50 + 10 gap
}
```

## See Also

- [Architecture](architecture.md) - Framework architecture overview
- [Layout System](layouts.md) - Comprehensive layout guide
- [Coordinate System](coordinate_system.md) - Coordinate system details
- [ComponentBuilder](component_builder.md) - Fluent component creation API
