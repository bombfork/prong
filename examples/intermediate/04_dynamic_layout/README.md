# Dynamic Layout Example

This example demonstrates how to manipulate UI at runtime, including adding/removing components, switching layout managers, and updating properties dynamically.

## What This Example Shows

- **Adding components** at runtime with `addChild()`
- **Removing components** with `removeChild()`
- **Switching layout managers** dynamically
- **Invalidating layouts** to trigger recalculation
- **Random component generation** for testing
- **Interactive UI manipulation** through control panel

## Runtime UI Manipulation

### Adding Components

```cpp
void addPanel() {
  // Create new panel
  auto panel = create<Panel>()
                   .withSize(120, 100)
                   .withBackgroundColor(r, g, b, 1.0f)
                   .build();

  // Add to parent
  g_contentPanel->addChild(std::move(panel));

  // Trigger layout recalculation
  g_contentPanel->invalidateLayout();
}
```

Key points:
- Create component with builder pattern
- Transfer ownership with `std::move()`
- Call `invalidateLayout()` to update positions

### Removing Components

```cpp
void removePanel() {
  auto& children = g_contentPanel->getChildren();
  if (!children.empty()) {
    // Remove last child
    g_contentPanel->removeChild(children.back());
    g_contentPanel->invalidateLayout();
  }
}
```

Key points:
- Get reference to children vector
- Pass pointer to child you want removed
- Layout is automatically recalculated

### Clearing All Components

```cpp
void clearPanels() {
  auto& children = g_contentPanel->getChildren();
  while (!children.empty()) {
    g_contentPanel->removeChild(children.back());
  }
  g_contentPanel->invalidateLayout();
}
```

## Dynamic Layout Switching

### Switching Layout Managers

You can change the layout manager at runtime:

```cpp
void useFlexLayoutRow() {
  auto layout = std::make_unique<FlexLayout<Panel>>();
  layout->setDirection(FlexLayout<Panel>::Direction::ROW);
  layout->setGap(10);

  g_contentPanel->setLayoutManager(std::move(layout));
  g_contentPanel->invalidateLayout();
}

void useGridLayout() {
  auto layout = std::make_unique<GridLayout<Panel>>();
  layout->setColumns(3);
  layout->setGap(10);

  g_contentPanel->setLayoutManager(std::move(layout));
  g_contentPanel->invalidateLayout();
}
```

The same children are automatically repositioned according to the new layout manager.

### When to Invalidate Layout

Call `invalidateLayout()` when:
- Adding or removing children
- Changing layout manager
- Modifying layout properties (gap, direction, etc.)
- Changing component sizes that affect layout

The layout will be recalculated on the next update cycle.

## Component Lifecycle

### Creating Components at Runtime

```cpp
std::unique_ptr<Panel> createRandomPanel() {
  // Use builder pattern
  auto panel = create<Panel>()
                   .withSize(120, 100)
                   .withBackgroundColor(r, g, b, 1.0f)
                   .build();

  // Can add children before adding to parent
  auto label = create<Button>("Label")
                   .withSize(100, 60)
                   .withPosition(10, 20)
                   .build();

  panel->addChild(std::move(label));

  return panel; // Return unique_ptr
}
```

### Ownership Model

Prong uses `std::unique_ptr` for ownership:

```cpp
// Create component (you own it)
auto component = create<Button>("Click").build();

// Transfer ownership to parent
parent->addChild(std::move(component));
// component is now nullptr, parent owns it

// Parent owns children until:
// 1. Parent is destroyed (children destroyed automatically)
// 2. Child is explicitly removed with removeChild()
```

## Interactive Control Panel Pattern

This example uses a common pattern: control panel + content area

```cpp
// Root with horizontal FlexLayout
// ├── Control Panel (left)
// │   ├── Add Button
// │   ├── Remove Button
// │   ├── Clear Button
// │   ├── Layout Buttons...
// │   └── ...
// └── Content Panel (right)
//     └── Dynamic children
```

Benefits:
- Separate controls from content
- Easy to add new operations
- Clear visual organization
- Testable manipulation logic

## Building and Running

```bash
# Build all examples
mise build-examples

# Run this example
./build/examples/intermediate/04_dynamic_layout/04_dynamic_layout
```

Experiment with different layouts and panel counts!

## Common Patterns

### Adding Multiple Components

```cpp
void addMultiple(int count) {
  for (int i = 0; i < count; ++i) {
    contentPanel->addChild(createComponent());
  }
  // Invalidate once after all additions
  contentPanel->invalidateLayout();
}
```

### Conditional Component Display

```cpp
void updateVisibility(bool showAdvanced) {
  for (auto& child : panel->getChildren()) {
    if (isAdvancedComponent(child.get())) {
      child->setVisible(showAdvanced);
    }
  }
}
```

### Updating Component Properties

```cpp
void updateTheme(const Color& color) {
  for (auto& child : panel->getChildren()) {
    if (auto* button = dynamic_cast<Button*>(child.get())) {
      button->setBackgroundColor(color);
    }
  }
}
```

### Animated Addition/Removal

```cpp
void animateAdd(std::unique_ptr<Component> component) {
  // Start with size 0
  component->setSize(0, 0);
  auto* compPtr = component.get();

  parent->addChild(std::move(component));

  // Animate size over time in update()
  // (Would need proper animation system for this)
}
```

## Use Cases

This pattern is useful for:

1. **Dynamic Lists**: Adding items as they're created
2. **Tab Panels**: Switching content based on active tab
3. **Wizards**: Different steps with different layouts
4. **Responsive Layouts**: Changing layout based on screen size
5. **Plugin Systems**: Loading UI components dynamically
6. **Game Menus**: Building menus from data
7. **Settings Panels**: Showing/hiding options conditionally

## Performance Considerations

- **Batch operations**: Invalidate layout once after multiple changes
- **Reuse components**: Keep pool of components instead of creating/destroying
- **Lazy creation**: Create components only when needed
- **Visibility over removal**: Hide components instead of removing if they'll be needed again

## Extending This Example

Try these modifications:

1. **Add animation**: Fade in new panels
2. **Drag to reorder**: Let users drag panels to reorder them
3. **Save/load layout**: Serialize layout configuration
4. **Undo/redo**: Track operations for undo
5. **Different component types**: Mix buttons, text inputs, etc.
6. **Layout transitions**: Animate between layout changes
7. **Templates**: Save and load layout templates

## Related Examples

- **basic/06_flex_layout** - FlexLayout introduction
- **basic/07_grid_layout** - GridLayout introduction
- **intermediate/01_nested_panels** - Static nested layouts
- **intermediate/02_responsive_ui** - Resize behavior
- **demo_app** - Full application with dynamic UI
