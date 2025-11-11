# 05 - Dock Layout

Application-style layout with panels docked to edges and a center fill area.

## What This Demonstrates
- Using DockLayout for complex application interfaces
- Docking panels to TOP, BOTTOM, LEFT, RIGHT positions
- Center panel that fills remaining space
- Creating IDE/application-style layouts
- Typical toolbar, sidebar, status bar arrangement

## Key Concepts

**DockLayout**: A layout manager designed for application-style interfaces. It divides the container into 5 regions: TOP, BOTTOM, LEFT, RIGHT, and CENTER. Docked panels take fixed sizes from their edges, and the CENTER panel fills whatever space remains.

**Dock Positions**:
- `TOP`: Docked to top edge, spans full width, uses specified height
- `BOTTOM`: Docked to bottom edge, spans full width, uses specified height
- `LEFT`: Docked to left edge, spans remaining height, uses specified width
- `RIGHT`: Docked to right edge, spans remaining height, uses specified width
- `CENTER`: Fills all remaining space after other panels are placed

**Size Behavior**:
- TOP/BOTTOM panels: Width is ignored (spans full width), height is used
- LEFT/RIGHT panels: Height is ignored (spans remaining height), width is used
- CENTER panel: Both dimensions are ignored (fills remaining space)

**Docking Order**: The order you dock panels matters:
1. TOP and BOTTOM panels are placed first (full width)
2. LEFT and RIGHT panels are placed next (in remaining height)
3. CENTER panel fills what's left

## Building
```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/05_dock_layout/05_dock_layout
```

## Code Walkthrough

1. **Main Panel** (lines 63-66): Create a panel with DockLayout that will contain all docked regions

2. **Toolbar** (lines 71-99): Create a panel with buttons, dock it to TOP with 60px height

3. **Left Sidebar** (lines 102-118): Create a sidebar with tool buttons, dock it to LEFT with 200px width

4. **Right Sidebar** (lines 121-143): Create a properties panel, dock it to RIGHT with 200px width

5. **Status Bar** (lines 146-158): Create a status bar, dock it to BOTTOM with 40px height

6. **Center Panel** (lines 161-174): Create the main content area, dock it to CENTER (fills remaining space)

7. **Docking API**: Use `layout.dock(component.get(), position)` before or after adding the child

## Try This

1. **Change dock sizes**: Modify the widths of sidebars or heights of toolbar/status bar
2. **Reorder docking**: Try docking panels in different orders to see the effect
3. **Remove a dock**: Comment out one of the docked panels to see CENTER grow
4. **Nested layouts**: Use FlexLayout or GridLayout inside the docked panels
5. **Toggle visibility**: Add buttons that show/hide sidebar panels dynamically

**Common Use Cases**:
- IDE interfaces (code editor with toolbars, sidebars, output panel)
- Document editors (toolbar, rulers, document area, status bar)
- Image editors (tools, canvas, properties, layers panel)
- Web browsers (address bar, bookmarks, content, status bar)
- Email clients (folder list, message list, message view, toolbar)

**Pro Tip**: DockLayout is perfect when you need fixed-size panels around the edges and a flexible content area in the middle. For more complex layouts, nest other layout managers within the docked panels.
