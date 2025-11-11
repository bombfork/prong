# Responsive UI Example

This example demonstrates how Prong components adapt to window resizing using resize behaviors and constraints. **Try resizing the window** to see the different behaviors in action!

## What This Example Shows

- **ResizeBehavior** enum for unified resize handling
- **AxisResizeBehavior** for independent horizontal/vertical control
- **ResponsiveConstraints** for min/max bounds
- **Real-time window resize handling** with immediate feedback

## Resize Behaviors Demonstrated

### 1. FIXED (Left Panel - Purple)

```cpp
fixedPanel->setResizeBehavior(Component::ResizeBehavior::FIXED);
```

**Behavior**: Component maintains its original size regardless of window resizing.

**Use cases**:

- Fixed-width sidebars
- Toolbars with specific dimensions
- Components that should never change size

### 2. SCALE (Second Panel - Green)

```cpp
scalePanel->setResizeBehavior(Component::ResizeBehavior::SCALE);
```

**Behavior**: Component scales proportionally with the window. If the window doubles in size, the component doubles in size.

**Use cases**:

- Maintaining relative sizes across different screen resolutions
- Scaling UI elements uniformly
- Responsive designs that preserve proportions

### 3. FILL (Third Panel - Blue)

```cpp
fillPanel->setResizeBehavior(Component::ResizeBehavior::FILL);
```

**Behavior**: Component expands to fill all available space in its parent.

**Use cases**:

- Main content areas
- Flexible panels that should use extra space
- Dynamic layouts where size is unknown

### 4. PER-AXIS with CONSTRAINTS (Right Panel - Orange)

```cpp
// Fixed width (stays 250px), fill height (stretches vertically)
constrainedPanel->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FIXED,
    Component::AxisResizeBehavior::FILL
);

// Prevent extremes with constraints
Component::ResponsiveConstraints constraints;
constraints.minWidth = 200;
constraints.maxWidth = 300;
constraints.minHeight = 300;
constraints.maxHeight = 800;
constrainedPanel->setConstraints(constraints);
```

**Behavior**: Independent control per axis plus min/max boundaries.

**Use cases**:

- Sidebars that keep width but fill height
- Headers that fill width but keep height
- Components that need bounded flexibility

## Window Resize Integration

To enable window resizing, you need to:

1. **Set up the resize callback**:

```cpp
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
  if (g_scene) {
    g_scene->handleResize(width, height);
  }
  glViewport(0, 0, width, height);
}

glfwSetFramebufferSizeCallback(glfwWindow, framebufferSizeCallback);
```

1. **Make root component responsive**:

```cpp
rootPanel->setResizeBehavior(Component::ResizeBehavior::FILL);
```

The `Scene::handleResize()` method automatically:

- Updates the scene dimensions
- Triggers resize events for all children
- Recalculates layouts
- Invalidates coordinate caches

## Choosing the Right Resize Behavior

### ResizeBehavior (Unified)

Best when both axes should behave the same way:

- `FIXED`: Static components
- `SCALE`: Proportional scaling
- `FILL`: Maximum flexibility
- `MAINTAIN_ASPECT`: Preserve aspect ratio while scaling

### AxisResizeBehavior (Per-Axis)

Best when axes need different behaviors:

```cpp
component->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FIXED,   // Horizontal
    Component::AxisResizeBehavior::FILL     // Vertical
);
```

Common patterns:

- **Sidebar**: FIXED horizontal, FILL vertical
- **Header**: FILL horizontal, FIXED vertical
- **Corner panel**: FIXED both (same as ResizeBehavior::FIXED)
- **Content area**: FILL both (same as ResizeBehavior::FILL)

## Constraints

Add constraints to any resize behavior:

```cpp
Component::ResponsiveConstraints constraints;
constraints.minWidth = 200;   // Won't shrink below 200px wide
constraints.maxWidth = 600;   // Won't grow beyond 600px wide
constraints.minHeight = 150;  // Won't shrink below 150px tall
constraints.maxHeight = 450;  // Won't grow beyond 450px tall
component->setConstraints(constraints);
```

Constraints work with all resize behaviors and prevent components from becoming too small (unreadable) or too large (wasted space).

## Building and Running

```bash
# Build all examples
mise build-examples

# Run this example
./build/examples/intermediate/02_responsive_ui/02_responsive_ui
```

**Important**: Resize the window to see the behaviors in action!

## Extending This Example

Try these modifications:

1. **Change resize behaviors**: Swap FIXED with SCALE, or FILL with FIXED
2. **Adjust constraints**: Make them tighter or looser
3. **Add nested panels**: See how resize behavior cascades
4. **Mix with layouts**: Combine with FlexLayout grow/shrink factors
5. **Try MAINTAIN_ASPECT**: Add a panel that keeps its aspect ratio

## Common Patterns

### Responsive Three-Panel Layout

```cpp
// Left sidebar: fixed width, fills height
leftPanel->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FIXED,
    Component::AxisResizeBehavior::FILL
);

// Center content: fills both dimensions
centerPanel->setResizeBehavior(Component::ResizeBehavior::FILL);

// Right sidebar: fixed width, fills height
rightPanel->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FIXED,
    Component::AxisResizeBehavior::FILL
);
```

### Header-Content-Footer

```cpp
// Header: fills width, fixed height
header->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FILL,
    Component::AxisResizeBehavior::FIXED
);

// Content: fills everything
content->setResizeBehavior(Component::ResizeBehavior::FILL);

// Footer: fills width, fixed height
footer->setAxisResizeBehavior(
    Component::AxisResizeBehavior::FILL,
    Component::AxisResizeBehavior::FIXED
);
```

## Related Examples

- **basic/01_hello_button** - Basic window setup
- **intermediate/01_nested_panels** - Complex layouts
- **intermediate/04_dynamic_layout** - Runtime layout changes
- **demo_app** - Full application with responsive panels
