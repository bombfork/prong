# Prong UI Framework

[![Build Status](https://github.com/bombfork/prong/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/bombfork/prong/actions/workflows/build.yml)

**Sharp, modern C++20 UI framework from BombFork**

Prong is a header-mostly, CRTP-based UI framework designed for high-performance applications requiring flexible layouts, theming support, and renderer-agnostic rendering. Built with modern C++20, it provides zero-cost abstractions while maintaining clean, expressive APIs.

## Features

- 🔱 **CRTP-Based Component System** - Zero-overhead polymorphism with compile-time dispatch
- 📐 **Flexible Layout Managers** - Flex, Grid, Dock, Stack, and Flow layouts
- 🎨 **Advanced Theming** - Hot-reload, semantic colors, multiple themes
- 🖼️ **Renderer-Agnostic** - Works with OpenGL, Vulkan, or custom renderers
- 🪟 **Window-Agnostic** - GLFW, SDL, or native window APIs
- ⚡ **Zero Dependencies** - Header-only where possible, minimal implementation files
- 🚀 **Modern C++20** - Concepts, ranges, and latest language features
- 📦 **CMake Integration** - Easy integration via FetchContent or find_package

## Quick Start

### Installation via CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    prong
    GIT_REPOSITORY https://github.com/bombfork/prong.git
    GIT_TAG        v1.0.0
)

FetchContent_MakeAvailable(prong)

target_link_libraries(your_app PRIVATE bombfork::prong)
```

### Basic Usage

Prong uses a **Scene-based architecture** where the Scene is the root of your UI hierarchy:

```cpp
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>

using namespace bombfork::prong;

// Create window and renderer (use your preferred implementations)
IWindow* window = createMyWindow();
IRenderer* renderer = createMyRenderer();

// Create scene (root of UI hierarchy)
Scene scene(window, renderer);
scene.attach();

// Build UI - components automatically inherit renderer from parent
auto panel = std::make_unique<Panel<>>();
panel->setPosition(50, 50);
panel->setSize(300, 200);

auto button = std::make_unique<Button<>>("Save");
button->setClickCallback([]() {
    std::cout << "Save clicked!" << std::endl;
});
panel->addChild(std::move(button));

scene.addChild(std::move(panel));

// Main loop
while (!window->shouldClose()) {
    // Convert window events to Event structs and pass to scene
    // (in window callbacks: scene.handleEvent(event))

    scene.updateAll(deltaTime);
    scene.renderAll();
    scene.present();
}

scene.detach();
```

## Architecture

### Coordinate System

Prong uses a **relative coordinate system** where child components are positioned relative to their parent's origin. This provides:

- **Intuitive positioning** - Children don't need to know their parent's position
- **Automatic updates** - Moving a parent automatically moves all children
- **Performance** - Global screen coordinates are cached automatically

```cpp
// Position children relative to parent
panel->setPosition(100, 200);    // Panel at screen position (100, 200)
button->setPosition(50, 75);     // Button at (50, 75) relative to panel
panel->addChild(std::move(button));  // Button now at screen position (150, 275)
```

For detailed information, see [docs/coordinate_system.md](docs/coordinate_system.md).

### Core Components

```
bombfork::prong::
├── core/                   # Base component system
│   ├── Scene              # Root scene component (entry point for UI hierarchy)
│   ├── Component           # CRTP base class with relative coordinates
│   ├── CoordinateSystem    # World ↔ Screen transformations (for viewports)
│   └── AsyncCallbackQueue  # Thread-safe callback management
├── components/             # Reusable UI widgets
│   ├── Button             # Clickable button
│   ├── Panel              # Container panel
│   ├── ListBox            # Scrollable list
│   └── TextInput          # Text input field
├── layout/                 # Layout managers (CRTP)
│   ├── FlexLayout         # Flexbox-inspired layout
│   ├── GridLayout         # CSS Grid-inspired layout
│   ├── DockLayout         # Docking panel layout
│   └── StackLayout        # Vertical/horizontal stacking
├── theming/                # Theming system
│   ├── Color              # Color representation
│   ├── ThemeManager       # Global theme management
│   └── AdvancedTheme      # Advanced theme with hot-reload support
├── events/                 # Event handling
│   ├── Event              # Unified event structure
│   ├── IClipboard         # Clipboard abstraction interface
│   ├── IKeyboard          # Keyboard abstraction interface
│   └── IWindow            # Window abstraction interface
├── generic/                # Generic UI components
│   ├── Dialog             # Modal dialogs
│   ├── Toolbar            # Toolbars
│   └── Viewport           # Scrollable viewport
└── rendering/              # Rendering abstractions
    └── IRenderer          # Renderer interface
```

### CRTP Pattern

Prong uses the Curiously Recurring Template Pattern (CRTP) for zero-cost abstraction:

```cpp
template<typename DerivedT>
class FlexLayout : public LayoutManager<DerivedT> {
    // Layout logic with compile-time polymorphism
    void layout(std::vector<Component*>& children) {
        auto* derived = static_cast<DerivedT*>(this);
        // Use derived class methods without virtual dispatch
    }
};

class MyFlexLayout : public FlexLayout<MyFlexLayout> {
    // Custom flex layout behavior
};
```

## Renderer Integration

Prong works with any rendering backend through the `IRenderer` interface:

```cpp
class MyOpenGLRenderer : public bombfork::prong::rendering::IRenderer {
public:
    // Implement required methods
    bool beginFrame() override { /* OpenGL setup */ }
    void endFrame() override { /* OpenGL cleanup */ }
    void drawRect(int x, int y, int w, int h, float r, float g, float b, float a) override {
        // OpenGL rectangle drawing
    }
    // ... other methods
};
```

**Note:** Prong's core is renderer-agnostic. Example renderer implementations (OpenGL) are provided in `examples/adapters/` for demonstration purposes but are not part of the core framework.

## Window Integration

Prong is window-library agnostic through the `IWindow` interface:

```cpp
class MyGLFWAdapter : public bombfork::prong::events::IWindow {
    GLFWwindow* window;
public:
    void getSize(int& w, int& h) const override {
        glfwGetWindowSize(window, &w, &h);
    }
    void* getNativeHandle() override { return window; }
    // ... other methods
};
```

**Note:** Example window adapters (GLFW) are provided in `examples/adapters/` and `examples/common/glfw_adapters/` for demonstration purposes. These are not part of the core framework - you implement the interfaces for your chosen windowing system.

## Theming

Prong provides a flexible theming system through `ThemeManager` and `AdvancedTheme`:

### Built-in Themes

```cpp
using namespace bombfork::prong::theming;

// Register and use built-in themes (Light, Dark, High Contrast)
ThemeManager::getInstance().registerBuiltinThemes();
ThemeManager::getInstance().setCurrentTheme("dark");
ThemeManager::getInstance().setCurrentTheme("light");
ThemeManager::getInstance().setCurrentTheme("high-contrast");
```

### Custom Themes

Create custom themes programmatically:

```cpp
#include <bombfork/prong/theming/advanced_theme.h>
#include <bombfork/prong/theming/theme_manager.h>

// Create a custom theme
auto customTheme = std::make_unique<AdvancedTheme>("custom", "My Custom Theme");

// Define colors
customTheme->setColor("primary", Color(52, 152, 219));      // #3498db
customTheme->setColor("background", Color(44, 62, 80));     // #2c3e50
customTheme->setColor("text", Color(236, 240, 241));        // #ecf0f1

// Register and activate
ThemeManager::getInstance().registerTheme(std::move(customTheme));
ThemeManager::getInstance().setCurrentTheme("custom");

// Access colors
const Color& bg = ThemeManager::getInstance().getCurrentTheme()->getColor("background");
```

### Theme Change Notifications

Register callbacks to respond to theme changes:

```cpp
ThemeManager::getInstance().addChangeCallback([](const ThemeChangeEvent& event) {
    std::cout << "Theme changed from " << event.oldThemeId
              << " to " << event.newThemeId << std::endl;
    // Update UI components as needed
});
```

## Layout System

### Flex Layout

```cpp
using namespace bombfork::prong::layout;

FlexLayout<MyApp> layout;
layout.setDirection(FlexDirection::ROW);
layout.setJustify(FlexJustify::SPACE_BETWEEN);
layout.setAlign(FlexAlign::CENTER);
layout.setGap(10.0f);

layout.layout(components, {800, 600});
```

### Grid Layout

```cpp
GridLayout<MyApp> grid;
grid.setColumns(3);
grid.setRows(2);
grid.setGap(5.0f, 5.0f);
grid.layout(components, {900, 600});
```

### Dock Layout

```cpp
DockLayout<MyApp> dock;
dock.dockTop(toolbar, 40);      // Toolbar at top, 40px height
dock.dockBottom(statusBar, 25); // Status bar at bottom, 25px height
dock.dockLeft(sidebar, 200);    // Sidebar on left, 200px width
dock.fillCenter(mainView);      // Fill remaining space
```

## Event Handling

Prong uses a **hierarchical event system** where events flow through the Scene and down the component tree:

```cpp
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/core/event.h>

// Scene is the entry point for all window events
Scene scene(window, renderer);

// Add components
auto button = std::make_unique<Button<>>("Click Me");
button->setClickCallback([]() {
    std::cout << "Clicked!\n";
});
scene.addChild(std::move(button));

// In your window callbacks, convert platform events to Prong Event structs
void mouseButtonCallback(GLFWwindow* w, int btn, int action, int mods) {
    double mx, my;
    glfwGetCursorPos(w, &mx, &my);

    Event event;
    event.type = (action == GLFW_PRESS) ? Event::Type::MOUSE_PRESS
                                         : Event::Type::MOUSE_RELEASE;
    event.localX = static_cast<int>(mx);
    event.localY = static_cast<int>(my);
    event.button = btn;
    event.modifiers = mods;

    scene.handleEvent(event);  // Scene propagates to children
}
```

**Key Features:**
- Scene is the entry point for all events from the window system
- Events automatically propagate to children with coordinate conversion
- Children rendered last (topmost) receive events first
- Components must be enabled and visible to receive events
- Override `handleEventSelf()` in your components for custom event handling

## Examples

See the `examples/` directory for complete applications:

- **hello_prong** - Minimal button and panel example
- **layout_demo** - Demonstrates all layout managers
- **theming_demo** - Theme switching and customization
- **custom_renderer** - Implementing a custom renderer

## Building from Source

### Native Build

```bash
# Clone repository
git clone https://github.com/bombfork/prong.git
cd prong

# Build with CMake
mkdir build && cd build
cmake .. -DPRONG_BUILD_EXAMPLES=ON -DPRONG_BUILD_TESTS=ON
cmake --build .

# Install (optional)
sudo cmake --install .
```

### Docker Build (Recommended)

For reproducible builds across different platforms, use the containerized build system with mise tasks:

```bash
# Build the Docker image
mise docker-build

# Build the library
mise docker-build-lib

# Build and run tests
mise docker-build-tests
mise docker-test

# Build examples
mise docker-build-examples

# Run clang-format
mise docker-format

# Open interactive shell
mise docker-shell
```

The mise Docker tasks provide a convenient, consistent interface and handle all volume mounts and image management automatically. See [docs/docker.md](docs/docker.md) for complete Docker build system documentation.

## Requirements

### Native Build Requirements

- **C++20** compatible compiler (GCC 10+, Clang 13+, MSVC 2019+)
- **CMake 3.14+**
- **clang-format** (for code formatting)
- **include-what-you-use** (for header dependency checking)
- No runtime dependencies (header-only core)

### Docker Build Requirements

- **Docker 20.10+** with BuildKit support
- **mise** (for running Docker tasks)

## License

MIT License - see LICENSE file for details.

## Contributing

Contributions are welcome! Please see CONTRIBUTING.md for guidelines.

## Roadmap

See our [GitHub Issues](https://github.com/bombfork/prong/issues) for planned features and enhancements. Notable goals include:

- Accessibility support (screen readers, keyboard navigation)
- Animation system
- Drag-and-drop framework
- Virtual scrolling for large lists
- Web backend (WebAssembly + WebGPU)

For the complete list and to suggest new features, visit the [issue tracker](https://github.com/bombfork/prong/issues).

## Acknowledgments

Developed by the BombFork team for high-performance game tools and editors.

---

**Prong** - Sharp UI for sharp developers ⚡🔱
