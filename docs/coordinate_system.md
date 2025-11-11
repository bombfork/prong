# Prong Coordinate System Design

## Overview

Prong uses a **relative coordinate system** with automatic caching for optimal performance and developer ergonomics. Child components are positioned relative to their parent's origin, and global screen coordinates are computed on-demand with intelligent caching.

This document provides a comprehensive explanation of the coordinate system architecture, APIs, and best practices.

## Design Philosophy

The coordinate system was designed with these goals in mind:

1. **Intuitive Developer Experience** - Developers should work with local coordinates by default, not worrying about absolute screen positions
2. **Performance** - Global coordinates should be cached to avoid redundant calculations
3. **Automatic Propagation** - Moving a parent should automatically move all children without manual updates
4. **Layout Manager Simplicity** - Layout managers should work entirely in local space
5. **Clean Separation** - Clear distinction between local (positioning) and global (rendering/hit testing) coordinate spaces

## Coordinate Spaces

Prong distinguishes between two coordinate spaces:

### Local Coordinates (Relative)

**Definition**: Position relative to the parent component's origin.

- Stored in the `localX` and `localY` member variables in `Component`
- Set using `setPosition(x, y)` or `setBounds(x, y, w, h)`
- Retrieved using `getPosition(x, y)` or `getBounds(x, y, w, h)`
- Used for: positioning children, layout calculations, parent-child relationships

**Special case**: For root components (components with no parent), local coordinates ARE the global coordinates.

#### Example

```cpp
// Create a parent panel at screen position (100, 200)
auto panel = std::make_unique<Panel>(renderer);
panel->setPosition(100, 200);
panel->setSize(400, 300);

// Create a child button at position (50, 75) relative to panel
auto button = std::make_unique<Button>(renderer, "Click");
button->setPosition(50, 75);  // Local: (50, 75)
button->setSize(120, 40);

// Add button to panel
panel->addChild(std::move(button));

// The button's global position is now (150, 275) = (100, 200) + (50, 75)
// But the button doesn't need to know or care about this
```

### Global Coordinates (Absolute)

**Definition**: Absolute screen-space position.

- Calculated by walking up the parent chain and summing all local positions
- Cached in `cachedGlobalX` and `cachedGlobalY` member variables
- Retrieved using `getGlobalPosition(x, y)` or `getGlobalBounds(x, y, w, h)`
- Used for: rendering to screen, hit testing, event dispatch

#### Calculation Formula

For any component, its global position is:

```text
globalX = localX + parent.globalX
globalY = localY + parent.globalY
```

This is computed recursively up to the root component.

#### Example

```cpp
// Component hierarchy:
// Root at local (100, 100)
//   └─ Child1 at local (10, 10)
//       └─ Child2 at local (5, 5)

root.setPosition(100, 100);
child1->setPosition(10, 10);
child2->setPosition(5, 5);

// Global positions:
int gx, gy;
root.getGlobalPosition(gx, gy);    // (100, 100) - root has no parent
child1->getGlobalPosition(gx, gy); // (110, 110) = 100 + 10
child2->getGlobalPosition(gx, gy); // (115, 115) = 110 + 5
```

## Caching System

### Why Caching?

Without caching, every call to `getGlobalPosition()` would require traversing the entire parent chain. In a deep component tree rendered at 60 FPS, this becomes expensive quickly.

### How It Works

1. **Lazy Calculation**: Global coordinates are only calculated when first requested
2. **Dirty Flag**: Each component has a `globalCacheDirty` boolean flag
3. **Cache Invalidation**: When a component's position changes, the cache is marked dirty
4. **Automatic Cascade**: Cache invalidation automatically propagates to all descendants

### Cache Invalidation Triggers

The cache is automatically invalidated when:

- `setPosition()` is called
- `setBounds()` is called (which sets position)
- A component is added as a child to a new parent via `addChild()`

### Cache Update

When `getGlobalPosition()` is called on a component with a dirty cache:

```cpp
void updateGlobalCache() const {
    if (parent) {
        int parentGlobalX, parentGlobalY;
        parent->getGlobalPosition(parentGlobalX, parentGlobalY);
        cachedGlobalX = parentGlobalX + localX;
        cachedGlobalY = parentGlobalY + localY;
    } else {
        // Root component: local coordinates are global
        cachedGlobalX = localX;
        cachedGlobalY = localY;
    }
    globalCacheDirty = false;
}
```

Note that this may recursively update parent caches if they are also dirty.

### Performance Characteristics

- **Cache hit**: O(1) - just return cached values
- **Cache miss (single component)**: O(1) - calculate from parent's cached value
- **Cache miss (entire tree dirty)**: O(depth) - walk up to root once
- **Moving a parent**: O(children) - invalidate all descendants

## API Reference

### Setting Position and Size

```cpp
// Set local position only
void setPosition(int x, int y);

// Set local position and size
void setBounds(int x, int y, int width, int height);

// Set size only
void setSize(int width, int height);
```

All of these methods work with **local coordinates** relative to the parent.

### Reading Local Coordinates

```cpp
// Get local position
void getPosition(int& outX, int& outY) const;

// Get local bounds (position + size)
void getBounds(int& outX, int& outY, int& outWidth, int& outHeight) const;

// Get size
void getSize(int& outWidth, int& outHeight) const;
```

### Reading Global Coordinates

```cpp
// Get global (screen-space) position
void getGlobalPosition(int& outX, int& outY) const;

// Get global bounds (position + size)
void getGlobalBounds(int& outX, int& outY, int& outWidth, int& outHeight) const;
```

These methods automatically update the cache if needed.

### Coordinate Conversion

```cpp
// Convert local to global
void localToGlobal(int localX, int localY, int& globalX, int& globalY) const;

// Convert global to local
void globalToLocal(int globalX, int globalY, int& localX, int& localY) const;
```

### Hit Testing

```cpp
// Check if a global screen point is within this component
bool containsGlobal(int globalX, int globalY) const;
```

This is the primary method used by `Component::handleEvent()` for hit testing.

### Protected Rendering Helpers

For use in `render()` implementations:

```cpp
protected:
    int getGlobalX() const;  // Get global X coordinate
    int getGlobalY() const;  // Get global Y coordinate
```

These are convenience methods that call `getGlobalPosition()` internally.

## Usage Patterns

### Positioning Children

Always use **local coordinates** when positioning children:

```cpp
class MyPanel : public Component {
public:
    MyPanel(rendering::IRenderer* renderer) : Component(renderer) {
        // Position children relative to this panel's origin
        auto button1 = std::make_unique<Button>(renderer, "Button 1");
        button1->setPosition(10, 10);  // 10px from panel's top-left
        addChild(std::move(button1));

        auto button2 = std::make_unique<Button>(renderer, "Button 2");
        button2->setPosition(10, 60);  // 10px from left, 60px from top
        addChild(std::move(button2));
    }
};
```

### Rendering Components

Always use **global coordinates** when rendering:

```cpp
void MyComponent::render() override {
    // Get global screen position for rendering
    int x = getGlobalX();
    int y = getGlobalY();

    // Render at absolute screen position
    renderer->drawRect(x, y, width, height, 1.0f, 0.0f, 0.0f, 1.0f);

    // For text or other elements within the component,
    // add offsets to the global position
    renderer->drawText(x + 10, y + 20, "Hello", 1.0f, 1.0f, 1.0f, 1.0f);
}
```

### Layout Managers

Layout managers work entirely in **local coordinates**:

```cpp
template<typename DerivedT>
class MyLayout : public LayoutManager<DerivedT> {
public:
    void layout(std::vector<Component*>& children, const Dimensions& available) {
        int currentX = 0;
        int currentY = 0;

        for (auto* child : children) {
            // Position children using local coordinates
            // These are relative to the parent component
            child->setBounds(currentX, currentY, 100, 50);
            currentX += 110;  // 100px width + 10px gap
        }
    }
};
```

### Event Handling

Event handlers receive **local coordinates**:

```cpp
bool MyComponent::handleClick(int localX, int localY) override {
    // localX and localY are relative to this component
    // (0, 0) is at the top-left corner of this component

    // Check if click is within a sub-region
    if (localX >= 10 && localX < 110 && localY >= 10 && localY < 50) {
        std::cout << "Clicked the button area!" << std::endl;
        return true;
    }

    return false;
}
```

The `Component::handleEvent()` method automatically converts global screen coordinates to local coordinates during event propagation to children.

### Coordinate Conversion

Sometimes you need to convert between spaces:

```cpp
void MyComponent::someMethod() {
    // Convert a local point to global (for tooltip positioning, etc.)
    int tooltipLocalX = 50;
    int tooltipLocalY = 20;
    int tooltipGlobalX, tooltipGlobalY;
    localToGlobal(tooltipLocalX, tooltipLocalY, tooltipGlobalX, tooltipGlobalY);

    // Now you can position a tooltip at the global screen position

    // Convert a global point to local (for custom hit testing)
    int mouseGlobalX = 200;
    int mouseGlobalY = 150;
    int mouseLocalX, mouseLocalY;
    globalToLocal(mouseGlobalX, mouseGlobalY, mouseLocalX, mouseLocalY);

    // Now you can check if mouseLocal is within a specific region
}
```

## Integration with Other Systems

### Event Handling

The hierarchical event system uses global coordinates at entry:

1. **Hit Testing**: `Component::handleEvent()` uses `containsEvent()` to check if events are within bounds
2. **Coordinate Conversion**: Automatically converts global screen coordinates to local during propagation to children
3. **Z-Order**: Tests components in reverse rendering order (topmost first)

### Layout Managers

Layout managers are completely isolated from global coordinates:

- They receive a `Dimensions` struct with available width/height
- They position children using `setBounds()` with local coordinates
- They never need to call `getGlobalPosition()`

### Renderer

The renderer works in screen-space (global coordinates):

- All `IRenderer` drawing methods expect absolute screen coordinates
- Components must use `getGlobalX()` and `getGlobalY()` in `render()`
- The renderer has no knowledge of the component hierarchy or local coordinates

## Common Patterns and Idioms

### Pattern: Centering a Child

```cpp
void centerChild(Component* parent, Component* child) {
    int childW, childH;
    child->getSize(childW, childH);

    int parentW, parentH;
    parent->getSize(parentW, parentH);

    // Calculate local position to center child within parent
    int centerX = (parentW - childW) / 2;
    int centerY = (parentH - childH) / 2;

    child->setPosition(centerX, centerY);  // Local coordinates
}
```

### Pattern: Positioning Relative to Another Child

```cpp
void positionBesideButton(Panel* panel, Button* button1, Button* button2) {
    int b1x, b1y, b1w, b1h;
    button1->getBounds(b1x, b1y, b1w, b1h);

    // Position button2 to the right of button1 with 10px gap
    button2->setPosition(b1x + b1w + 10, b1y);  // All local coordinates
}
```

### Pattern: Custom Hit Testing

```cpp
bool MyComponent::handleClick(int localX, int localY) override {
    // Hit test against custom regions
    struct Region {
        int x, y, w, h;
        std::function<void()> action;
    };

    std::vector<Region> regions = {
        {10, 10, 100, 40, [this]() { onSaveClick(); }},
        {120, 10, 100, 40, [this]() { onCancelClick(); }},
    };

    for (const auto& region : regions) {
        if (localX >= region.x && localX < region.x + region.w &&
            localY >= region.y && localY < region.y + region.h) {
            region.action();
            return true;
        }
    }

    return false;
}
```

### Anti-Pattern: Storing Global Coordinates

**DON'T DO THIS:**

```cpp
class MyComponent : public Component {
    int storedGlobalX;  // BAD: Will become stale!
    int storedGlobalY;

public:
    void render() override {
        // This will use outdated coordinates if parent moved
        renderer->drawRect(storedGlobalX, storedGlobalY, width, height, ...);
    }
};
```

**DO THIS INSTEAD:**

```cpp
class MyComponent : public Component {
public:
    void render() override {
        // Always compute global coordinates on-demand (uses cache)
        int x = getGlobalX();
        int y = getGlobalY();
        renderer->drawRect(x, y, width, height, ...);
    }
};
```

## Performance Considerations

### When Cache is Most Effective

The cache is most effective when:

1. **Deep hierarchies** - More levels = more savings per cache hit
2. **Static layouts** - Components that don't move frequently
3. **Multiple queries per frame** - Rendering, hit testing, etc.

### When to Expect Cache Misses

Cache misses occur when:

1. **Initial render** - First call after component creation
2. **After movement** - Any position change invalidates the cache
3. **After reparenting** - Adding to a new parent invalidates

### Best Practices for Performance

1. **Batch position updates** - If moving multiple components, update all positions before rendering
2. **Avoid unnecessary moves** - Only call `setPosition()` when the position actually changes
3. **Layout once per frame** - Call `performLayout()` once, not continuously
4. **Don't store global coords** - Let the cache do its job

## Comparison with Other UI Frameworks

### Prong (Relative + Cached)

```cpp
panel->setPosition(100, 200);
button->setPosition(50, 75);  // Relative to panel
panel->addChild(std::move(button));
// Button's global position is automatically (150, 275)
```

### Immediate Mode (Dear ImGui style)

```cpp
ImGui::SetCursorPos(ImVec2(100, 200));
ImGui::BeginChild("panel");
    ImGui::SetCursorPos(ImVec2(50, 75));  // Relative
    ImGui::Button("Click");
ImGui::EndChild();
```

### Absolute Positioning (Traditional)

```cpp
panel->setPosition(100, 200);
button->setPosition(150, 275);  // Must manually calculate
panel->addChild(button);
// If panel moves, must manually update button position
```

### Retained + Absolute (Qt Widgets)

```cpp
panel->move(100, 200);
button->move(150, 275);  // Absolute screen coordinates
button->setParent(panel);
// Qt automatically converts to relative internally
```

## CoordinateSystem Class (World Coordinates)

Note: There is a separate `CoordinateSystem` class (`include/bombfork/prong/core/coordinate_system.h`) that handles **world-to-screen transformations** for game/viewport scenarios. This is distinct from the component coordinate system:

- **Component coordinates** (this document): UI element positioning in screen space
- **World coordinates** (`CoordinateSystem` class): Game world to viewport transformations with zoom/pan

The `CoordinateSystem` class is used primarily in viewport components that display game worlds, maps, or other zoomable content. It handles:

- World position ↔ Screen position conversions
- Camera positioning and zoom levels
- Tile-based coordinate systems
- Visible region calculations

See the `CoordinateSystem` class documentation for details on world coordinate transformations.

## Troubleshooting

### Problem: Component renders at wrong position

**Symptoms**: Component appears at (0, 0) or wrong location

**Likely causes**:

1. Using local coordinates for rendering instead of global
2. Not calling `getGlobalX()` and `getGlobalY()`

**Solution**: Always use global coordinates in `render()`:

```cpp
void render() override {
    int x = getGlobalX();  // Not localX!
    int y = getGlobalY();  // Not localY!
    renderer->drawRect(x, y, width, height, ...);
}
```

### Problem: Child doesn't move with parent

**Symptoms**: Child stays in place when parent moves

**Likely causes**:

1. Child is not actually added to parent via `addChild()`
2. Child's position is being set to global coordinates
3. Child is being repositioned after parent moves

**Solution**: Ensure proper parent-child relationship:

```cpp
auto child = std::make_unique<MyComponent>();
child->setPosition(localX, localY);  // Local coordinates
parent->addChild(std::move(child));  // Establish relationship
// Now child will move with parent automatically
```

### Problem: Events not working after component moves

**Symptoms**: Clicks/hover don't work at new position

**Likely causes**:

1. Component not added to the scene hierarchy
2. Custom `containsEvent()` implementation using incorrect coordinate space

**Solution**: Ensure hit testing uses global coordinates:

```cpp
// The default implementation is correct:
bool containsGlobal(int globalX, int globalY) const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    return globalX >= gx && globalX < gx + width &&
           globalY >= gy && globalY < gy + height;
}
```

### Problem: Performance issues with deep hierarchies

**Symptoms**: Slow rendering or event handling

**Likely causes**:

1. Constantly invalidating cache by repeatedly calling `setPosition()`
2. Very deep component nesting (>50 levels)

**Solution**:

- Only update positions when actually changed
- Consider flattening the hierarchy if possible
- Profile to confirm this is actually the bottleneck

## Future Considerations

Potential enhancements to the coordinate system:

1. **Rotation and Scaling** - Currently not supported; would require transformation matrices
2. **Coordinate Space Validation** - Debug-mode checks to catch coordinate space misuse
3. **Dirty Rectangle Optimization** - Track which regions need repainting
4. **Batch Invalidation** - Defer cache invalidation until needed
5. **Relative Sizing** - Similar to relative positioning (e.g., "50% of parent width")

## Summary

The Prong coordinate system provides:

- **Relative positioning** for intuitive parent-child relationships
- **Automatic cache management** for performance
- **Clear separation** between local (positioning) and global (rendering) spaces
- **Simple APIs** that match developer expectations
- **Zero manual maintenance** of global coordinates

By following the patterns in this document, you can build complex UI hierarchies with confidence that coordinates will be handled correctly and efficiently.
