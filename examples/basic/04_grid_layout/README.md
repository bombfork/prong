# 04 - Grid Layout

2D grid layout manager for arranging components in rows and columns.

## What This Demonstrates

- Using GridLayout for 2D arrangements
- Configuring rows and columns
- Setting row and column gaps
- Creating calculator-style interfaces
- Handling different grid aspect ratios (3x3, 4x4, 2x5)

## Key Concepts

**GridLayout**: A CSS Grid-inspired layout manager that divides available space into a 2D grid of equal-sized cells. Perfect for calculators, keypads, icon grids, and any interface requiring regular 2D arrangement.

**Equal Distribution**: GridLayout automatically calculates cell size based on available space, rows, columns, and gaps. All cells are the same size within a grid.

**Automatic Placement**: Children are placed automatically in grid cells, filling left-to-right, top-to-bottom. Just add children in the order you want them to appear.

**Configuration**:

- `setRows(n)`: Number of rows in the grid
- `setColumns(n)`: Number of columns in the grid
- `setRowGap(px)`: Vertical spacing between cells
- `setColumnGap(px)`: Horizontal spacing between cells

## Building

```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/04_grid_layout/04_grid_layout
```

## Code Walkthrough

1. **3x3 Grid** (lines 63-85): Simple numbered grid demonstrating basic GridLayout with 10px gaps

2. **Calculator Grid** (lines 88-113): 4x4 grid styled like a calculator with operation buttons, showing practical use case

3. **Rectangular Grid** (lines 116-137): 2x5 grid demonstrating how GridLayout handles non-square arrangements with different gaps

4. **Automatic Sizing**: Notice how we set initial button sizes, but GridLayout overrides them to fit the grid cells perfectly

## Try This

1. **Change grid dimensions**: Try `setRows(4)` and `setColumns(4)` for the first panel
2. **Adjust gaps**: Experiment with different row/column gap values
3. **Add more cells**: Add more buttons to see how GridLayout wraps to new rows
4. **Non-square grids**: Try a 1x6 grid for a toolbar-like layout
5. **Nested grids**: Create a panel with GridLayout that contains other panels with GridLayout

**Common Use Cases**:

- Calculator interfaces
- Numeric keypads
- Icon/app launchers
- Image/photo galleries
- Dashboard tiles
- Game boards (chess, tic-tac-toe, etc.)

**Note**: GridLayout always creates equal-sized cells. For variable-sized grid cells, use FlexLayout with multiple rows, or combine multiple GridLayouts.
