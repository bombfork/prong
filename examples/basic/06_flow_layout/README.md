# 06 - Flow Layout

Automatic wrapping layout that flows children like text, wrapping to new lines as needed.

## What This Demonstrates
- Using FlowLayout for tag clouds and chip lists
- Automatic line wrapping when content doesn't fit
- Configuring horizontal and vertical gaps
- Responsive behavior that adapts to container size
- Handling variable-width children

## Key Concepts

**FlowLayout**: A layout manager that arranges children left-to-right (like text flow), automatically wrapping to the next line when the next child doesn't fit. Perfect for tag clouds, chip lists, filter buttons, and any interface where items should flow naturally.

**Automatic Wrapping**: When a child component doesn't fit on the current line (based on its width + gaps), FlowLayout automatically moves it to the next line. This makes the layout highly responsive to container size changes.

**Gap Configuration**:
- `setHorizontalGap(px)`: Space between items on the same line
- `setVerticalGap(px)`: Space between lines

**Child Sizing**: FlowLayout respects each child's individual size - it doesn't resize children. This allows variable-width buttons to coexist naturally (like tags of different lengths).

## Building
```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/06_flow_layout/06_flow_layout
```

## Code Walkthrough

1. **First Panel** (lines 63-92): Programming language tags with variable widths (based on text length) and 10px gaps. Watch how longer tags wrap to new lines.

2. **Second Panel** (lines 95-113): Uniform-sized tags with larger gaps (20px horizontal, 15px vertical) for a more spaced-out appearance.

3. **Third Panel** (lines 116-145): Compact skills cloud with small 5px gaps, demonstrating a dense tag cloud layout.

4. **Variable Widths**: Notice how button widths are calculated based on text length (`60 + length * 8`) to simulate real tag sizing.

## Try This

1. **Resize the window**: Drag the window narrower to see tags wrap to more lines, or wider to see them spread out
2. **Change gaps**: Try `setHorizontalGap(30)` for more breathing room
3. **Add more tags**: Add additional buttons to see how FlowLayout handles them
4. **Uniform sizes**: Give all buttons the same size for a grid-like appearance with wrapping
5. **Very wide container**: Make the panel wider so all tags fit on one line

**Common Use Cases**:
- Tag clouds (categories, labels, keywords)
- Filter chips (active filters in a search interface)
- Skill lists (resume, profile pages)
- Icon toolbars that wrap at narrow widths
- Badge collections (achievements, awards)
- Language/region selectors
- Color palette swatches

**Comparison with Other Layouts**:
- **vs StackLayout**: StackLayout only goes in one direction, no wrapping
- **vs FlexLayout**: FlexLayout can grow/shrink children, FlowLayout keeps original sizes
- **vs GridLayout**: GridLayout enforces regular grid, FlowLayout allows irregular wrapping

**Pro Tip**: FlowLayout is perfect when you don't know ahead of time how many items you'll have, or when items have variable sizes. It automatically adapts to the content.
