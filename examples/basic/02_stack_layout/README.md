# 02 - Stack Layout

Simple layout manager that arranges children in a vertical or horizontal stack.

## What This Demonstrates

- Using Panel with StackLayout template parameter
- Configuring vertical stacking with spacing
- Configuring horizontal stacking with spacing
- Automatic child positioning by the layout manager

## Key Concepts

**StackLayout**: The simplest layout manager in Prong. It arranges children in a single line (vertical or horizontal) with uniform spacing between them. Perfect for simple button groups, toolbars, or menus.

**Layout Configuration**: Access the layout manager via `panel->getLayoutManager()` to configure direction and spacing. The layout automatically repositions children whenever the panel is resized or children are added/removed.

**Direction**:

- `VERTICAL`: Stack children top-to-bottom
- `HORIZONTAL`: Stack children left-to-right

## Building

```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/02_stack_layout/02_stack_layout
```

## Code Walkthrough

1. **Vertical Panel** (lines 60-97): Creates a panel with StackLayout, sets VERTICAL direction and 15px spacing, then adds 4 buttons that stack top-to-bottom
2. **Horizontal Panel** (lines 100-132): Creates another panel with StackLayout, sets HORIZONTAL direction and 20px spacing, then adds 3 buttons that stack left-to-right
3. **Automatic Layout**: When children are added with `addChild()`, the layout manager automatically positions them according to the direction and spacing

## Try This

1. **Change spacing**: Modify `setSpacing(15)` to `setSpacing(40)` to see wider gaps
2. **Add more buttons**: Add a 5th button to the vertical panel
3. **Change direction**: Switch the vertical panel to HORIZONTAL to see how it looks
4. **Different sizes**: Give buttons different widths/heights to see how StackLayout handles them
5. **Nested layouts**: Try creating a vertical panel that contains multiple horizontal panels

**Note**: StackLayout doesn't resize children - it only positions them. Each child keeps its own size. For flexible sizing, see FlexLayout in example 03.
