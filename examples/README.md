# Prong Examples

This directory contains example implementations and applications demonstrating how to use the Prong UI framework.

## Directory Structure

```text
examples/
├── adapters/              # Example adapter implementations
│   ├── glfw_window_adapter.h       # GLFW → IWindow adapter
│   └── simple_opengl_renderer.h    # OpenGL → IRenderer adapter
└── demo_app/              # Comprehensive demo application
    ├── main.cpp           # Main entry point with GLFW setup
    └── scenes/
        └── demo_scene.h   # Demo scene showcasing all components and layouts
```

## Adapters

The adapter examples show how to integrate Prong with specific libraries:

### GLFW Window Adapter (`glfw_window_adapter.h`)

Demonstrates how to implement the `IWindow` interface using GLFW. Key features:

- Wraps a `GLFWwindow*` to provide platform-agnostic window operations
- Converts GLFW callbacks to Prong's `WindowCallbacks` system
- Translates GLFW constants to Prong's input constants
- Handles modifier key state tracking

**Usage:**

```cpp
GLFWwindow* glfwWindow = glfwCreateWindow(1280, 720, "App", nullptr, nullptr);
auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
auto renderer = std::make_unique<SimpleOpenGLRenderer>();
Scene scene(windowAdapter.get(), renderer.get());
scene.attach();
// Components use hierarchical event handling - just add them to the scene
```

### Simple OpenGL Renderer (`simple_opengl_renderer.h`)

Demonstrates how to implement the `IRenderer` interface using OpenGL. This is a minimal implementation focusing on clarity over performance.

**Features:**

- Texture management with OpenGL texture objects
- Basic primitive rendering (rectangles, sprites)
- Orthographic 2D projection setup
- Scissor test support for clipping

**Production Notes:**
A production renderer would include:

- Texture atlas management
- Batch rendering with vertex buffers
- Font rendering with FreeType or similar
- Shader-based rendering pipeline
- Performance optimizations

**Usage:**

```cpp
auto renderer = std::make_unique<SimpleOpenGLRenderer>();
renderer->initialize(1280, 720);
```

## Demo Application

The `demo_app` example demonstrates a comprehensive Prong application showcasing all components and layouts:

### Building and Running

**Using mise (recommended):**

```bash
mise demo
```

**Manual build:**

```bash
mkdir build && cd build
cmake .. -DPRONG_BUILD_EXAMPLES=ON
cmake --build .
./examples/prong_demo_app
```

### What It Demonstrates

1. **Window and Renderer Setup**
   - Creating GLFW window
   - Initializing OpenGL renderer with font support
   - Setting up adapters (window, clipboard, keyboard)

2. **Scene-Based Architecture**
   - Scene as root component managing UI hierarchy
   - Hierarchical event handling with automatic propagation
   - Window callbacks converting to Event structs
   - Event propagation through component tree (children first)

3. **All UI Components**
   - **Panel** - Bordered containers with titles and padding
   - **Button** - Interactive buttons with callbacks and hover states
   - **TextInput** - Text input with clipboard support and change callbacks
   - **ListBox** - Scrollable list with selection support
   - Component styling, theming, and visual feedback

4. **All Layout Managers**
   - **FlexLayout** - Main layout structure with flexible sizing
   - **GridLayout** - 3x3 button grid with equal cell sizes
   - **StackLayout** - Horizontal button stack
   - **FlowLayout** - Tag-like wrapping interface

5. **Application Loop**
   - Frame timing and delta time
   - Hierarchical update (updateAll) and render (renderAll)
   - Font rendering for text overlays
   - Frame presentation

6. **Interactive Features**
   - Add custom items to list via text input
   - Select items from the list
   - Clear the list
   - Click buttons in different layouts
   - Visual feedback and console logging
   - ESC key or Exit button to close

### Dependencies

- GLFW 3.3+
- OpenGL 3.3+
- OpenGL loader (GLAD, GLEW, etc.)

## Creating Your Own Adapters

### Window Adapter

To create a window adapter for a different library (SDL, native APIs, etc.):

1. Inherit from `IWindow`
2. Implement all pure virtual methods
3. Create a `WindowCallbacks` struct in `setCallbacks()`
4. Register these callbacks with your windowing library
5. Convert library-specific constants to Prong constants

See `glfw_window_adapter.h` for a complete example.

### Renderer Adapter

To create a renderer for a different graphics API (Vulkan, DirectX, etc.):

1. Inherit from `IRenderer`
2. Create a custom `TextureHandle` subclass for your API
3. Implement all pure virtual methods
4. Handle coordinate system conversions (Prong uses top-left origin with relative child coordinates)
5. Implement batching for optimal performance

See `simple_opengl_renderer.h` for a complete example.

### Coordinate System

Prong uses a relative coordinate system:

- **Screen origin**: Top-left corner at (0, 0)
- **Child positions**: Relative to their parent's origin, NOT the screen
- **Rendering**: Components use global (screen-space) coordinates internally for rendering
- **API**: `setPosition()` and `setBounds()` use parent-relative coordinates
- **Conversion**: Use `getGlobalPosition()` to convert local to screen coordinates

Example:

```cpp
// Panel at screen position (100, 100)
auto panel = create<Panel<>>()
               .withPosition(100, 100)  // Screen position (if no parent)
               .withSize(400, 300)
               .build();

// Button at position (10, 10) relative to panel
// Will be rendered at screen position (110, 110)
auto button = create<Button>("OK")
                .withPosition(10, 10)  // Relative to parent panel
                .withSize(100, 30)
                .build();
panel->addChild(std::move(button));
```

## Integration with Existing Projects

If you have an existing application with a rendering system:

### Option 1: Adapter Pattern

Create adapters that wrap your existing renderer and window classes (recommended for quick integration).

### Option 2: Direct Implementation

Implement `IWindow` and `IRenderer` directly in your window/renderer classes.

### Example Integration Flow

```cpp
// 1. Wrap your existing window
class MyWindowAdapter : public IWindow {
    MyExistingWindow* window;
    // ... implement interface
};

// 2. Wrap your existing renderer
class MyRendererAdapter : public IRenderer {
    MyExistingRenderer* renderer;
    // ... implement interface
};

// 3. Use with Prong
auto windowAdapter = std::make_unique<MyWindowAdapter>(myWindow);
auto rendererAdapter = std::make_unique<MyRendererAdapter>(myRenderer);
Scene scene(windowAdapter.get(), rendererAdapter.get());
scene.attach();

// 4. Create and add UI components - renderer is inherited from scene
auto button = create<Button>("Click").build();
scene.addChild(std::move(button));
// Events are automatically propagated through the component tree
```

## Notes

- The example renderer uses legacy OpenGL (immediate mode) for simplicity
- Production code should use modern OpenGL with shaders and VBOs
- Text rendering in the examples is a placeholder - use FreeType or similar
- The examples are header-only for easy integration into your projects

## License

These examples are provided under the same license as the Prong framework.
See the main LICENSE file for details.
