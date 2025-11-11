# 03 - Flex Layout

Flexbox-inspired layout manager for responsive, flexible UI designs.

## What This Demonstrates
- Using FlexLayout in ROW and COLUMN directions
- Setting grow factors for flexible sizing
- Configuring justify content (START, SPACE_BETWEEN, etc.)
- Configuring align items (CENTER, STRETCH, etc.)
- How FlexLayout responds to window resizing

## Key Concepts

**FlexLayout**: Prong's most powerful layout manager, inspired by CSS Flexbox. Children can be set to grow/shrink dynamically to fill available space while maintaining proportions.

**Grow Factor**: Determines how much a component grows relative to siblings. If one child has grow=1 and another has grow=2, the second child gets twice as much of the remaining space.

**Justify Content**: Controls spacing along the main axis (horizontal for ROW, vertical for COLUMN)
- `START`: Pack items at the start
- `CENTER`: Center items
- `END`: Pack items at the end
- `SPACE_BETWEEN`: Distribute items with space between them
- `SPACE_AROUND`: Distribute items with space around them
- `SPACE_EVENLY`: Distribute items with equal space

**Align Items**: Controls alignment along the cross axis
- `START`: Align items at the start
- `CENTER`: Center items
- `END`: Align items at the end
- `STRETCH`: Stretch items to fill cross axis

## Building
```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/03_flex_layout/03_flex_layout
```

## Code Walkthrough

1. **First Panel** (lines 60-108): Horizontal FlexLayout with mixed grow factors. The "Fixed" button stays at 100px, while others grow proportionally (1x, 2x, 1x shares).

2. **Second Panel** (lines 111-135): Demonstrates SPACE_BETWEEN justify content, which distributes buttons evenly with space between them (no gaps needed).

3. **Third Panel** (lines 138-161): Vertical COLUMN layout with STRETCH alignment, making all buttons fill the panel width.

4. **Setting Grow**: Use `layout.setGrow(component.get(), factor)` before adding the component to the panel.

## Try This

1. **Resize the window**: Drag the window edges to see how buttons with grow factors expand/contract
2. **Change grow factors**: Try different values like 3.0f or 0.5f
3. **Change justify content**: Try `SPACE_EVENLY` or `CENTER` in the second panel
4. **Add shrink factors**: Use `layout.setShrink(component.get(), 1.0f)` to allow shrinking
5. **Mix fixed and flexible**: Create more complex layouts with some fixed-size elements

**Pro Tip**: FlexLayout is ideal for toolbars, navigation bars, and responsive content areas that need to adapt to different screen sizes.
