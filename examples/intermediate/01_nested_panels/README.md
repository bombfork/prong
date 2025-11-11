# Nested Panels Example

This example demonstrates complex nested layout composition in Prong, showing how multiple layout managers can be combined in a hierarchy to create sophisticated UI structures.

## What This Example Shows

- **Multiple layout manager types** working together in harmony
- **Three levels of nesting** with different layout strategies
- **FlexLayout** for flexible horizontal and vertical distribution
- **GridLayout** for structured 2D grid arrangement
- **StackLayout** for simple sequential positioning

## Layout Structure

```
Root Panel (FlexLayout ROW)
├── Left Panel (StackLayout VERTICAL)
│   ├── Action 1 Button
│   ├── Action 2 Button
│   ├── Action 3 Button
│   ├── Action 4 Button
│   └── Action 5 Button
│
├── Center Panel (GridLayout 2x2)
│   ├── Red Panel (with button)
│   ├── Green Panel (with button)
│   ├── Blue Panel (with button)
│   └── Yellow Panel (with button)
│
└── Right Panel (FlexLayout COLUMN)
    ├── Top Panel (StackLayout HORIZONTAL)
    │   ├── H1 Button
    │   └── H2 Button
    └── Bottom Panel (Panel with manual positioning)
        └── Centered Action Button
```

## Key Concepts

### Layout Manager Composition

Each panel can have its own layout manager, which controls how its children are positioned:

```cpp
// Root uses FlexLayout for flexible horizontal arrangement
auto rootLayout = std::make_unique<FlexLayout<Panel>>();
rootLayout->setDirection(FlexLayout<Panel>::Direction::ROW);
rootPanel->setLayoutManager(std::move(rootLayout));

// Left panel uses StackLayout for vertical button stack
auto leftLayout = std::make_unique<StackLayout<Panel>>();
leftLayout->setOrientation(StackLayout<Panel>::Orientation::VERTICAL);
leftPanel->setLayoutManager(std::move(leftLayout));

// Center panel uses GridLayout for 2x2 grid
auto gridLayout = std::make_unique<GridLayout<Panel>>();
gridLayout->setRows(2);
gridLayout->setColumns(2);
centerPanel->setLayoutManager(std::move(gridLayout));
```

### When to Use Each Layout Manager

**FlexLayout**: Use when you need:
- Flexible space distribution
- Alignment control (justify, align)
- Grow/shrink behavior
- Gaps between items

**GridLayout**: Use when you need:
- Fixed rows and columns
- Uniform cell sizes
- Regular grid patterns
- Equal spacing

**StackLayout**: Use when you need:
- Simple sequential arrangement
- Horizontal or vertical stacking
- Consistent spacing
- No complex alignment needs

### Mixed Positioning

Notice that some panels use layout managers while others position children manually:

```cpp
// Manual positioning still works alongside layout managers
auto centerButton = create<Button>("Centered\nAction")
                        .withSize(200, 100)
                        .withPosition(55, 180)  // Manual position
                        .build();
```

This is useful when:
- You need precise control over a few elements
- Layout managers would be overkill
- You're creating custom component internals

## Building and Running

```bash
# Build all examples
mise build-examples

# Run this example
./build/examples/intermediate/01_nested_panels/01_nested_panels
```

## Extending This Example

Try these modifications to learn more:

1. **Change layout directions**: Switch FlexLayout between ROW and COLUMN
2. **Adjust grid size**: Try a 3x3 or 4x2 grid in the center
3. **Add more nesting levels**: Add another layer of panels within the grid cells
4. **Experiment with gaps**: Change `setGap()` and `setSpacing()` values
5. **Add resize behavior**: Make panels responsive to window resizing

## Related Examples

- **basic/06_flex_layout** - Introduction to FlexLayout
- **basic/07_grid_layout** - Introduction to GridLayout
- **intermediate/02_responsive_ui** - Making layouts responsive
- **intermediate/04_dynamic_layout** - Changing layouts at runtime
