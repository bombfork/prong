# Prong Layout System Guide

## Overview

Prong's layout system provides automatic positioning and sizing of components within containers. Layout managers handle the complex math of arranging UI elements, allowing you to focus on the structure and behavior of your interface.

**Key Features:**
- **Automatic arrangement** - Components positioned automatically based on layout rules
- **Responsive sizing** - Layouts adapt to container size changes
- **Composable** - Nest layouts within layouts for complex UIs
- **Type-safe** - CRTP-based design with compile-time checking
- **Zero overhead** - No virtual function calls in hot paths

## Layout Manager Architecture

### How Layouts Work

Layout managers implement two key methods:

```cpp
template<typename DerivedT>
class LayoutManager {
public:
    // Calculate required space for components
    virtual Dimensions measureLayout(
        const std::vector<Component*>& components) = 0;

    // Position and size components within available space
    virtual void layout(
        std::vector<Component*>& components,
        const Dimensions& availableSpace) = 0;
};
```

**Measurement Phase:**
- Calculate minimum space needed for all children
- Consider preferred sizes, minimum sizes, grow/shrink factors
- Return total dimensions required

**Layout Phase:**
- Position components within available space
- Apply alignment, spacing, and distribution rules
- Set component bounds using local coordinates

### Attaching Layouts to Components

Layouts can be attached to any container component:

```cpp
// Method 1: Template parameter (compile-time)
auto panel = create<Panel<FlexLayout>>().build();
panel->getLayout().configure({
    .direction = FlexDirection::ROW,
    .gap = 10.0f
});

// Method 2: Runtime attachment
auto flexLayout = std::make_shared<FlexLayout>();
flexLayout->configure({
    .direction = FlexDirection::ROW,
    .gap = 10.0f
});

auto panel = create<Panel<>>()
    .withLayout(flexLayout)
    .build();
```

### Layout Invalidation

Layouts are recalculated when:
- A child component is added or removed
- A child's size changes
- The container is resized
- `invalidateLayout()` is called manually

Layouts are calculated lazily before rendering, not immediately on invalidation.

## FlexLayout

Flexbox-inspired layout with flexible sizing and distribution along a main axis.

### Configuration

```cpp
struct FlexLayoutManager::Configuration {
    FlexDirection direction = FlexDirection::ROW;
    FlexJustify justify = FlexJustify::START;
    FlexAlign align = FlexAlign::STRETCH;
    float gap = 0.0f;
    bool wrap = false;
};
```

### Direction

Controls the main axis direction:

```cpp
enum class FlexDirection {
    ROW,           // Left to right (→)
    ROW_REVERSE,   // Right to left (←)
    COLUMN,        // Top to bottom (↓)
    COLUMN_REVERSE // Bottom to top (↑)
};
```

**Example:**

```cpp
auto layout = std::make_shared<FlexLayout>();
layout->configure({
    .direction = FlexDirection::ROW
});

auto panel = create<Panel<>>()
    .withLayout(layout)
    .withChildren(
        create<Button>("A").withSize(100, 50).build(),
        create<Button>("B").withSize(100, 50).build(),
        create<Button>("C").withSize(100, 50).build()
    )
    .build();
```

**Result (ROW):**
```
┌─────────────────────────────┐
│ ┌───┐ ┌───┐ ┌───┐          │
│ │ A │ │ B │ │ C │          │
│ └───┘ └───┘ └───┘          │
└─────────────────────────────┘
  →     →     →
```

**Result (COLUMN):**
```
┌──────┐
│ ┌──┐ │
│ │A │ │
│ └──┘ │
│  ↓   │
│ ┌──┐ │
│ │B │ │
│ └──┘ │
│  ↓   │
│ ┌──┐ │
│ │C │ │
│ └──┘ │
└──────┘
```

### Justify (Main Axis)

Controls distribution along the main axis:

```cpp
enum class FlexJustify {
    START,         // Pack from start
    END,           // Pack from end
    CENTER,        // Center pack
    SPACE_BETWEEN, // Space between items
    SPACE_AROUND,  // Space around items
    SPACE_EVENLY   // Equal space everywhere
};
```

**Visual Guide (ROW direction):**

```
START:
┌─────────────────────────────┐
│ [A][B][C]                   │
└─────────────────────────────┘

END:
┌─────────────────────────────┐
│                   [A][B][C] │
└─────────────────────────────┘

CENTER:
┌─────────────────────────────┐
│         [A][B][C]           │
└─────────────────────────────┘

SPACE_BETWEEN:
┌─────────────────────────────┐
│ [A]      [B]      [C]       │
└─────────────────────────────┘

SPACE_AROUND:
┌─────────────────────────────┐
│   [A]    [B]    [C]         │
└─────────────────────────────┘

SPACE_EVENLY:
┌─────────────────────────────┐
│  [A]   [B]   [C]            │
└─────────────────────────────┘
```

### Align (Cross Axis)

Controls alignment perpendicular to the main axis:

```cpp
enum class FlexAlign {
    STRETCH,  // Stretch to fill cross axis
    START,    // Align to start of cross axis
    END,      // Align to end of cross axis
    CENTER,   // Center on cross axis
    BASELINE  // Align text baselines
};
```

**Visual Guide (ROW direction, varying heights):**

```
STRETCH:
┌────────────────┐
│ ┌──┐ ┌──┐ ┌──┐│
│ │A │ │B │ │C ││
│ │  │ │  │ │  ││
│ └──┘ └──┘ └──┘│
└────────────────┘

START:
┌────────────────┐
│ ┌──┐ ┌──┐ ┌──┐│
│ │A │ │B │ │C ││
│ └──┘ └──┘ └──┘│
│                │
└────────────────┘

END:
┌────────────────┐
│                │
│ ┌──┐ ┌──┐ ┌──┐│
│ │A │ │B │ │C ││
│ └──┘ └──┘ └──┘│
└────────────────┘

CENTER:
┌────────────────┐
│     ┌──┐       │
│ ┌──┐│B │┌──┐  │
│ │A ││  ││C │  │
│ └──┘└──┘└──┘  │
└────────────────┘
```

### Flex Item Properties

Individual items can have grow, shrink, and basis properties:

```cpp
struct FlexItemProperties {
    float grow = 0.0f;   // How much to grow
    float shrink = 1.0f; // How much to shrink
    float basis = 0.0f;  // Initial size
};
```

**Example with grow factors:**

```cpp
auto layout = std::make_shared<FlexLayout>();
layout->configure({
    .direction = FlexDirection::ROW,
    .gap = 10.0f
});

// Set grow factors: Button B grows twice as much as A and C
layout->setItemProperties({
    {.grow = 1.0f},  // Button A
    {.grow = 2.0f},  // Button B (grows 2x)
    {.grow = 1.0f}   // Button C
});

auto panel = create<Panel<>>()
    .withLayout(layout)
    .withChildren(
        create<Button>("A").build(),  // Will fill available space
        create<Button>("B").build(),  // Will fill 2x more than A/C
        create<Button>("C").build()
    )
    .build();
```

**Result (with extra space):**
```
┌─────────────────────────────────────┐
│ ┌──────┐ ┌──────────────┐ ┌──────┐ │
│ │  A   │ │      B       │ │  C   │ │
│ └──────┘ └──────────────┘ └──────┘ │
└─────────────────────────────────────┘
   grow=1     grow=2          grow=1
```

### Common Use Cases

**Toolbar:**
```cpp
auto toolbar = create<Panel<FlexLayout>>()
    .withSize(800, 50)
    .build();

toolbar->getLayout().configure({
    .direction = FlexDirection::ROW,
    .justify = FlexJustify::START,
    .align = FlexAlign::CENTER,
    .gap = 5.0f
});

toolbar->addChild(create<Button>("File").build());
toolbar->addChild(create<Button>("Edit").build());
toolbar->addChild(create<Button>("View").build());
```

**Sidebar Layout:**
```cpp
auto sidebar = create<Panel<FlexLayout>>()
    .withSize(200, 600)
    .build();

sidebar->getLayout().configure({
    .direction = FlexDirection::COLUMN,
    .justify = FlexJustify::START,
    .align = FlexAlign::STRETCH,
    .gap = 10.0f
});

sidebar->addChild(create<Button>("Dashboard").build());
sidebar->addChild(create<Button>("Settings").build());
sidebar->addChild(create<Button>("Help").build());
```

**Form Row:**
```cpp
auto formRow = create<Panel<FlexLayout>>()
    .withSize(400, 40)
    .build();

formRow->getLayout().configure({
    .direction = FlexDirection::ROW,
    .justify = FlexJustify::SPACE_BETWEEN,
    .align = FlexAlign::CENTER
});

formRow->addChild(create<Panel<>>().withSize(100, 30).build());  // Label
formRow->addChild(create<TextInput>().withSize(250, 30).build()); // Input
```

## GridLayout

CSS Grid-inspired layout for arranging components in rows and columns.

### Configuration

```cpp
struct GridLayoutManager::Configuration {
    size_t columns = 1;
    size_t rows = 0;  // Auto-calculate if 0
    float horizontalSpacing = 0.0f;
    float verticalSpacing = 0.0f;
    GridAlignment cellAlignment = GridAlignment::STRETCH;
    bool equalCellSize = false;
};
```

### Alignment

```cpp
enum class GridAlignment {
    START,   // Align to start of cell
    CENTER,  // Center within cell
    END,     // Align to end of cell
    STRETCH  // Stretch to fill cell
};
```

### Basic Example

```cpp
auto grid = std::make_shared<GridLayout>();
grid->configure({
    .columns = 3,
    .horizontalSpacing = 10.0f,
    .verticalSpacing = 10.0f,
    .cellAlignment = GridAlignment::STRETCH
});

auto panel = create<Panel<>>()
    .withLayout(grid)
    .withChildren(
        create<Button>("1").build(),
        create<Button>("2").build(),
        create<Button>("3").build(),
        create<Button>("4").build(),
        create<Button>("5").build(),
        create<Button>("6").build()
    )
    .build();
```

**Result:**
```
┌───────────────────────────┐
│ ┌───┐  ┌───┐  ┌───┐      │
│ │ 1 │  │ 2 │  │ 3 │      │
│ └───┘  └───┘  └───┘      │
│                           │
│ ┌───┐  ┌───┐  ┌───┐      │
│ │ 4 │  │ 5 │  │ 6 │      │
│ └───┘  └───┘  └───┘      │
└───────────────────────────┘
```

### Equal Cell Sizes

```cpp
grid->configure({
    .columns = 3,
    .equalCellSize = true
});
```

Forces all cells to be the same size (determined by largest component).

### Common Use Cases

**Icon Grid:**
```cpp
auto iconGrid = std::make_shared<GridLayout>();
iconGrid->configure({
    .columns = 4,
    .horizontalSpacing = 15.0f,
    .verticalSpacing = 15.0f,
    .cellAlignment = GridAlignment::CENTER,
    .equalCellSize = true
});

auto panel = create<Panel<>>()
    .withLayout(iconGrid)
    .build();

// Add icon buttons
for (int i = 0; i < 12; i++) {
    panel->addChild(create<Button>(std::to_string(i + 1))
        .withSize(64, 64)
        .build());
}
```

**Form Grid:**
```cpp
auto formGrid = std::make_shared<GridLayout>();
formGrid->configure({
    .columns = 2,  // Label + Input columns
    .horizontalSpacing = 10.0f,
    .verticalSpacing = 5.0f,
    .cellAlignment = GridAlignment::START
});

auto form = create<Panel<>>()
    .withLayout(formGrid)
    .withChildren(
        create<Panel<>>().withSize(100, 30).build(),  // "Name" label
        create<TextInput>().withSize(200, 30).build(),
        create<Panel<>>().withSize(100, 30).build(),  // "Email" label
        create<TextInput>().withSize(200, 30).build()
    )
    .build();
```

**Calculator Layout:**
```cpp
auto calcGrid = std::make_shared<GridLayout>();
calcGrid->configure({
    .columns = 4,
    .horizontalSpacing = 5.0f,
    .verticalSpacing = 5.0f,
    .equalCellSize = true
});

// Add calculator buttons
std::vector<std::string> buttons = {
    "7", "8", "9", "/",
    "4", "5", "6", "*",
    "1", "2", "3", "-",
    "0", ".", "=", "+"
};

auto calc = create<Panel<>>().withLayout(calcGrid).build();
for (const auto& label : buttons) {
    calc->addChild(create<Button>(label).withSize(60, 60).build());
}
```

## DockLayout

Docking panels at edges with a center fill area.

### Configuration

```cpp
struct DockLayoutManager::DockConfiguration {
    bool allowFloating = true;
    bool showTabs = true;
    float splitterThickness = 5.0f;
};

enum class DockArea {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    CENTER,
    FLOATING
};

struct DockSplitterConfig {
    float minSize = 50.0f;
    float maxSize = 500.0f;
    bool resizable = true;
    float initialRatio = 0.2f;  // 20% of parent
};
```

### Basic Example

```cpp
auto dockLayout = std::make_shared<DockLayout>();
dockLayout->configure({
    .allowFloating = false,
    .splitterThickness = 5.0f
});

// Add dock regions
dockLayout->addRegion({
    .area = DockArea::LEFT,
    .components = {leftPanel.get()},
    .splitterConfig = {.initialRatio = 0.2f}
});

dockLayout->addRegion({
    .area = DockArea::TOP,
    .components = {toolbar.get()},
    .splitterConfig = {.initialRatio = 0.1f}
});

dockLayout->addRegion({
    .area = DockArea::CENTER,
    .components = {mainContent.get()}
});

auto mainWindow = create<Panel<>>()
    .withLayout(dockLayout)
    .build();
```

**Result:**
```
┌─────────────────────────────────┐
│         TOP (Toolbar)           │ ← 10% height
├──────┬──────────────────────────┤
│      │                          │
│ LEFT │       CENTER             │
│ 20%  │     (Main Content)       │
│      │                          │
├──────┴──────────────────────────┤
│       BOTTOM (Status)           │ ← Fixed height
└─────────────────────────────────┘
```

### Common Use Cases

**IDE Layout:**
```cpp
auto ideLayout = std::make_shared<DockLayout>();

// File browser on left
ideLayout->addRegion({
    .area = DockArea::LEFT,
    .components = {fileBrowser.get()},
    .splitterConfig = {
        .minSize = 150.0f,
        .maxSize = 400.0f,
        .resizable = true,
        .initialRatio = 0.2f
    }
});

// Properties panel on right
ideLayout->addRegion({
    .area = DockArea::RIGHT,
    .components = {propertiesPanel.get()},
    .splitterConfig = {
        .minSize = 200.0f,
        .maxSize = 500.0f,
        .resizable = true,
        .initialRatio = 0.25f
    }
});

// Console at bottom
ideLayout->addRegion({
    .area = DockArea::BOTTOM,
    .components = {console.get()},
    .splitterConfig = {
        .minSize = 100.0f,
        .maxSize = 400.0f,
        .resizable = true,
        .initialRatio = 0.2f
    }
});

// Code editor in center
ideLayout->addRegion({
    .area = DockArea::CENTER,
    .components = {codeEditor.get()}
});
```

**Dashboard:**
```cpp
auto dashboardLayout = std::make_shared<DockLayout>();

// Navigation bar at top
dashboardLayout->addRegion({
    .area = DockArea::TOP,
    .components = {navbar.get()},
    .splitterConfig = {.initialRatio = 0.08f}
});

// Sidebar on left
dashboardLayout->addRegion({
    .area = DockArea::LEFT,
    .components = {sidebar.get()},
    .splitterConfig = {.initialRatio = 0.15f}
});

// Main dashboard content in center
dashboardLayout->addRegion({
    .area = DockArea::CENTER,
    .components = {dashboardContent.get()}
});
```

## StackLayout

Simple vertical or horizontal stacking with spacing.

### Configuration

```cpp
struct StackLayoutManager::Configuration {
    StackOrientation orientation = StackOrientation::VERTICAL;
    StackAlignment alignment = StackAlignment::START;
    float spacing = 0.0f;
    bool expandMain = false;   // Expand along main axis
    bool expandCross = false;  // Expand along cross axis
};

enum class StackOrientation {
    VERTICAL,   // Top to bottom
    HORIZONTAL  // Left to right
};

enum class StackAlignment {
    START,   // Top/left
    CENTER,  // Center
    END,     // Bottom/right
    STRETCH  // Fill available space
};
```

### Basic Example

```cpp
auto stackLayout = std::make_shared<StackLayout>();
stackLayout->configure({
    .orientation = StackOrientation::VERTICAL,
    .alignment = StackAlignment::START,
    .spacing = 10.0f
});

auto panel = create<Panel<>>()
    .withLayout(stackLayout)
    .withChildren(
        create<Button>("Button 1").withSize(200, 40).build(),
        create<Button>("Button 2").withSize(200, 40).build(),
        create<Button>("Button 3").withSize(200, 40).build()
    )
    .build();
```

**Result (VERTICAL):**
```
┌────────────┐
│ ┌────────┐ │
│ │Button 1│ │
│ └────────┘ │
│     ↕ 10px │
│ ┌────────┐ │
│ │Button 2│ │
│ └────────┘ │
│     ↕ 10px │
│ ┌────────┐ │
│ │Button 3│ │
│ └────────┘ │
└────────────┘
```

### Alignment Examples

**STRETCH (vertical stack):**
```
┌──────────────────┐
│ ┌──────────────┐ │
│ │   Button 1   │ │
│ └──────────────┘ │
│ ┌──────────────┐ │
│ │   Button 2   │ │
│ └──────────────┘ │
└──────────────────┘
```

**CENTER (vertical stack):**
```
┌──────────────────┐
│    ┌────────┐    │
│    │Button 1│    │
│    └────────┘    │
│    ┌────────┐    │
│    │Button 2│    │
│    └────────┘    │
└──────────────────┘
```

### Common Use Cases

**Vertical Menu:**
```cpp
auto menuLayout = std::make_shared<StackLayout>();
menuLayout->configure({
    .orientation = StackOrientation::VERTICAL,
    .alignment = StackAlignment::STRETCH,
    .spacing = 2.0f
});

auto menu = create<Panel<>>()
    .withLayout(menuLayout)
    .withChildren(
        create<Button>("Dashboard").build(),
        create<Button>("Analytics").build(),
        create<Button>("Settings").build(),
        create<Button>("Logout").build()
    )
    .build();
```

**Horizontal Button Group:**
```cpp
auto buttonGroup = std::make_shared<StackLayout>();
buttonGroup->configure({
    .orientation = StackOrientation::HORIZONTAL,
    .alignment = StackAlignment::CENTER,
    .spacing = 5.0f
});

auto group = create<Panel<>>()
    .withLayout(buttonGroup)
    .withChildren(
        create<Button>("Yes").withSize(80, 30).build(),
        create<Button>("No").withSize(80, 30).build(),
        create<Button>("Cancel").withSize(80, 30).build()
    )
    .build();
```

## FlowLayout

Automatic wrapping layout that flows components like text.

### Configuration

```cpp
struct FlowLayoutManager::Configuration {
    bool horizontal = false;  // Flow direction
    FlowAlignment mainAlignment = FlowAlignment::START;
    FlowAlignment crossAlignment = FlowAlignment::START;
    FlowOverflow overflowBehavior = FlowOverflow::WRAP;
    float spacing = 4.0f;
    float crossSpacing = 4.0f;
    size_t maxItemsPerLine = 0;  // 0 = auto
};

enum class FlowAlignment {
    START,
    CENTER,
    END,
    SPACE_BETWEEN,
    SPACE_AROUND
};

enum class FlowOverflow {
    WRAP,    // Wrap to next line
    SCROLL,  // Enable scrolling
    CLIP,    // Clip excess
    RESIZE   // Resize container
};
```

### Basic Example

```cpp
auto flowLayout = std::make_shared<FlowLayout>();
flowLayout->configure({
    .horizontal = true,
    .mainAlignment = FlowAlignment::START,
    .spacing = 10.0f,
    .crossSpacing = 10.0f,
    .overflowBehavior = FlowOverflow::WRAP
});

auto panel = create<Panel<>>()
    .withSize(300, 200)  // Fixed width
    .withLayout(flowLayout)
    .build();

// Add varying size buttons
for (int i = 1; i <= 10; i++) {
    panel->addChild(create<Button>(std::to_string(i))
        .withSize(60, 40)
        .build());
}
```

**Result (wraps automatically):**
```
┌───────────────────────────────┐
│ [1]  [2]  [3]  [4]            │
│                               │
│ [5]  [6]  [7]                 │
│                               │
│ [8]  [9]  [10]                │
└───────────────────────────────┘
```

### Common Use Cases

**Tag Cloud:**
```cpp
auto tagFlow = std::make_shared<FlowLayout>();
tagFlow->configure({
    .horizontal = true,
    .mainAlignment = FlowAlignment::START,
    .spacing = 5.0f,
    .crossSpacing = 5.0f,
    .overflowBehavior = FlowOverflow::WRAP
});

auto tagPanel = create<Panel<>>()
    .withSize(400, 200)
    .withLayout(tagFlow)
    .build();

std::vector<std::string> tags = {
    "C++", "Python", "UI", "Framework", "OpenGL",
    "GLFW", "Graphics", "Layout", "Components"
};

for (const auto& tag : tags) {
    tagPanel->addChild(create<Button>(tag).build());
}
```

**Image Gallery:**
```cpp
auto galleryFlow = std::make_shared<FlowLayout>();
galleryFlow->configure({
    .horizontal = true,
    .mainAlignment = FlowAlignment::START,
    .spacing = 10.0f,
    .crossSpacing = 10.0f,
    .overflowBehavior = FlowOverflow::WRAP
});

auto gallery = create<Panel<>>()
    .withSize(800, 600)
    .withLayout(galleryFlow)
    .build();

// Add thumbnail panels
for (int i = 0; i < 20; i++) {
    gallery->addChild(create<Panel<>>()
        .withSize(150, 150)  // Square thumbnails
        .build());
}
```

## Nested Layouts

Layouts can be nested within each other to create complex UIs.

### Example: Application Window

```cpp
// Main window with DockLayout
auto mainLayout = std::make_shared<DockLayout>();

// Top toolbar with FlexLayout
auto topToolbar = create<Panel<FlexLayout>>()
    .withSize(800, 50)
    .build();
topToolbar->getLayout().configure({
    .direction = FlexDirection::ROW,
    .justify = FlexJustify::START,
    .align = FlexAlign::CENTER,
    .gap = 5.0f
});
topToolbar->addChild(create<Button>("File").build());
topToolbar->addChild(create<Button>("Edit").build());

// Left sidebar with StackLayout
auto leftSidebar = create<Panel<StackLayout>>()
    .withSize(200, 600)
    .build();
leftSidebar->getLayout().configure({
    .orientation = StackOrientation::VERTICAL,
    .alignment = StackAlignment::STRETCH,
    .spacing = 5.0f
});
leftSidebar->addChild(create<Button>("Home").build());
leftSidebar->addChild(create<Button>("Settings").build());

// Center content with GridLayout
auto centerContent = create<Panel<GridLayout>>()
    .withSize(600, 600)
    .build();
centerContent->getLayout().configure({
    .columns = 2,
    .horizontalSpacing = 10.0f,
    .verticalSpacing = 10.0f
});

// Configure dock layout
mainLayout->addRegion({
    .area = DockArea::TOP,
    .components = {topToolbar.get()},
    .splitterConfig = {.initialRatio = 0.08f}
});
mainLayout->addRegion({
    .area = DockArea::LEFT,
    .components = {leftSidebar.get()},
    .splitterConfig = {.initialRatio = 0.2f}
});
mainLayout->addRegion({
    .area = DockArea::CENTER,
    .components = {centerContent.get()}
});

// Create main window
auto mainWindow = create<Panel<>>()
    .withSize(800, 600)
    .withLayout(mainLayout)
    .withChildren(
        std::move(topToolbar),
        std::move(leftSidebar),
        std::move(centerContent)
    )
    .build();
```

**Visual Structure:**
```
┌─────────────────────────────────────────┐
│  ┌───┐ ┌────┐ ┌────┐  (FlexLayout)     │
│  │File│ │Edit│ │View│                   │
│  └───┘ └────┘ └────┘                    │
├──────┬──────────────────────────────────┤
│ ┌──┐ │  ┌────────┐ ┌────────┐          │
│ │H │ │  │  Cell  │ │  Cell  │          │
│ └──┘ │  └────────┘ └────────┘          │
│ ┌──┐ │  ┌────────┐ ┌────────┐          │
│ │S │ │  │  Cell  │ │  Cell  │          │
│ └──┘ │  └────────┘ └────────┘          │
│Stack │      GridLayout                  │
└──────┴──────────────────────────────────┘
```

### Example: Form with Sections

```cpp
// Main form with StackLayout
auto form = create<Panel<StackLayout>>()
    .withSize(400, 600)
    .build();
form->getLayout().configure({
    .orientation = StackOrientation::VERTICAL,
    .alignment = StackAlignment::STRETCH,
    .spacing = 20.0f
});

// Section 1: Personal Info (GridLayout)
auto personalSection = create<Panel<GridLayout>>()
    .withSize(400, 150)
    .build();
personalSection->getLayout().configure({
    .columns = 2,
    .horizontalSpacing = 10.0f,
    .verticalSpacing = 10.0f
});
personalSection->addChild(create<Panel<>>().build());  // Label
personalSection->addChild(create<TextInput>().build());
// ... more fields

// Section 2: Address (GridLayout)
auto addressSection = create<Panel<GridLayout>>()
    .withSize(400, 150)
    .build();
// ... configure similar to above

// Section 3: Action Buttons (FlexLayout)
auto buttonRow = create<Panel<FlexLayout>>()
    .withSize(400, 50)
    .build();
buttonRow->getLayout().configure({
    .direction = FlexDirection::ROW,
    .justify = FlexJustify::END,
    .gap = 10.0f
});
buttonRow->addChild(create<Button>("Submit").build());
buttonRow->addChild(create<Button>("Cancel").build());

// Add sections to form
form->addChild(std::move(personalSection));
form->addChild(std::move(addressSection));
form->addChild(std::move(buttonRow));
```

## Layout Performance Tips

### 1. Minimize Layout Invalidations

```cpp
// BAD: Causes layout recalculation for each child
for (int i = 0; i < 100; i++) {
    panel->addChild(create<Button>(std::to_string(i)).build());
    // Layout invalidated 100 times
}

// GOOD: Batch children before adding
std::vector<std::unique_ptr<Component>> children;
for (int i = 0; i < 100; i++) {
    children.push_back(create<Button>(std::to_string(i)).build());
}
// Add all at once (or use withChildren)
for (auto& child : children) {
    panel->addChild(std::move(child));
}
// Layout invalidated once before render
```

### 2. Use Appropriate Layout Complexity

```cpp
// For simple stacking, StackLayout is faster than FlexLayout
auto simpleList = create<Panel<StackLayout>>()  // ✓ Simpler
    .build();

// vs
auto complexList = create<Panel<FlexLayout>>()  // More overhead
    .build();
```

### 3. Cache Layout References

```cpp
// BAD: Reconfigures layout repeatedly
for (int i = 0; i < 10; i++) {
    panel->getLayout().configure({...});  // Lookup overhead
}

// GOOD: Cache reference
auto& layout = panel->getLayout();
for (int i = 0; i < 10; i++) {
    layout.configure({...});
}
```

### 4. Use Fixed Sizes When Possible

```cpp
// Layouts can skip measurement if children have fixed sizes
auto button = create<Button>("OK")
    .withSize(100, 40)  // Fixed size, no measurement needed
    .build();
```

## Troubleshooting Layouts

### Components Not Visible

**Symptom:** Components added but not showing

**Causes:**
- Parent size too small (0x0)
- Components positioned outside parent bounds
- Layout not configured properly

**Solution:**
```cpp
// Check parent size
int w, h;
panel->getSize(w, h);
std::cout << "Panel size: " << w << "x" << h << std::endl;

// Check if layout is set
if (!panel->hasLayout()) {
    std::cout << "No layout attached!" << std::endl;
}

// Manually trigger layout
panel->performLayout();
```

### Components Overlapping

**Symptom:** Components draw on top of each other

**Causes:**
- No layout attached (all at 0,0)
- Layout spacing = 0 and children same size

**Solution:**
```cpp
// Ensure layout is attached
auto layout = std::make_shared<StackLayout>();
layout->configure({
    .spacing = 10.0f  // Add spacing
});
panel->setLayout(layout);
```

### Components Too Large/Small

**Symptom:** Components don't fit properly

**Causes:**
- Grow/shrink factors incorrect
- Cell alignment wrong
- Missing size constraints

**Solution:**
```cpp
// For FlexLayout, check grow factors
layout->setItemProperties({
    {.grow = 1.0f, .shrink = 1.0f},
    {.grow = 1.0f, .shrink = 1.0f}
});

// For GridLayout, check alignment
layout->configure({
    .cellAlignment = GridAlignment::STRETCH  // vs START/CENTER
});
```

### Layout Not Updating

**Symptom:** Changes don't reflect visually

**Causes:**
- Layout invalidation not triggered
- Layout calculations happen lazily

**Solution:**
```cpp
// Force layout recalculation
panel->invalidateLayout();

// Or trigger via size change
int w, h;
panel->getSize(w, h);
panel->setSize(w + 1, h);  // Triggers layout
panel->setSize(w, h);      // Restore size
```

## See Also

- [Architecture](architecture.md) - Framework architecture and component system
- [Coordinate System](coordinate_system.md) - Understanding coordinate spaces
- [ComponentBuilder](component_builder.md) - Fluent API for creating layouts
- [Best Practices](best_practices.md) - Guidelines for effective layouts
