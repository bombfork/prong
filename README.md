# Prong UI Framework

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

```cpp
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/layout/flex_layout.h>

using namespace bombfork::prong;

class MyApp : public Component {
public:
    MyApp(rendering::IRenderer* renderer) : Component(renderer) {
        // Create UI components
        auto saveButton = std::make_unique<Button>(renderer, "Save");
        saveButton->setClickCallback([] { /* save action */ });

        auto panel = std::make_unique<Panel>(renderer);
        panel->addChild(std::move(saveButton));

        addChild(std::move(panel));
    }

    void update(double deltaTime) override {
        // Update logic
    }

    void render() override {
        // Rendering handled by component tree
    }
};
```

## Architecture

### Core Components

```
bombfork::prong::
├── core/                   # Base component system
│   ├── Component           # CRTP base class
│   ├── CoordinateSystem    # World ↔ Screen transformations
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
│   └── ThemeParser        # Load themes from files
├── events/                 # Event handling
│   ├── EventDispatcher    # Central event router
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

### Provided Implementations

- **OpenGL Renderer** - Reference implementation (OpenGL 3.3+)
- **Vulkan Renderer** - High-performance implementation (Vulkan 1.2+)
- **Software Renderer** - CPU-based fallback

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

## Theming

### Built-in Themes

```cpp
using namespace bombfork::prong::theming;

// Use built-in themes
ThemeManager::getInstance().setCurrentTheme("dark");
ThemeManager::getInstance().setCurrentTheme("light");
ThemeManager::getInstance().setCurrentTheme("high-contrast");
```

### Custom Themes

Load themes from YAML, JSON, or properties files:

```yaml
# mytheme.yaml
name: "My Custom Theme"
colors:
  primary: "#3498db"
  background: "#2c3e50"
  text: "#ecf0f1"

components:
  button:
    backgroundColor: "{primary}"
    textColor: "{text}"
    hoverColor: "#4a90e2"
```

```cpp
ThemeParser parser;
auto result = parser.parseFromFile("mytheme.yaml");
if (result.success) {
    ThemeManager::getInstance().registerTheme("custom", std::move(result.theme));
    ThemeManager::getInstance().setCurrentTheme("custom");
}
```

### Hot Reload

```cpp
ThemeHotReloader reloader("mytheme.yaml", [](auto theme) {
    ThemeManager::getInstance().updateCurrentTheme(std::move(theme));
});

reloader.startWatching(1000); // Check every 1000ms
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

```cpp
EventDispatcher dispatcher(windowAdapter);

// Register components for event handling
dispatcher.registerComponent(&button);
dispatcher.registerComponent(&panel);

// Handle mouse click
dispatcher.processMouseButton(BUTTON_LEFT, ACTION_PRESS, 0);

// Handle keyboard input
dispatcher.processKey(KEY_ENTER, 0, ACTION_PRESS, 0);
```

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

- [ ] Accessibility support (screen readers, keyboard navigation)
- [ ] Animation system
- [ ] Drag-and-drop framework
- [ ] Virtual scrolling for large lists
- [ ] Web backend (WebAssembly + WebGPU)

## Acknowledgments

Developed by the BombFork team for high-performance game tools and editors.

---

**Prong** - Sharp UI for sharp developers ⚡🔱
