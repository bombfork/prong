# ComponentBuilder Pattern

The `ComponentBuilder` provides a fluent interface for creating UI components in the Prong framework. It eliminates the need to pass renderers manually and provides a clean, chainable API for setting properties and callbacks.

## Overview

Components created with the builder automatically inherit their renderer from their parent when added to the component hierarchy. This follows the principle of "tell, don't configure" - you describe what you want, and the framework handles the details.

## Basic Usage

### Including the Header

```cpp
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
```

### Creating a Simple Button

```cpp
using namespace bombfork::prong;

auto button = create<Button>("Click Me")
    .withSize(120, 40)
    .withPosition(10, 10)
    .withClickCallback([]() {
        std::cout << "Button clicked!\n";
    })
    .build();
```

## Factory Functions

Two factory functions are provided:

### `create<ComponentT>()`
Creates a component using its default constructor.

```cpp
auto button = create<Button>()
    .withText("OK")
    .build();
```

### `create<ComponentT>(args...)`
Creates a component by forwarding arguments to its constructor.

```cpp
auto button = create<Button>("OK")  // Passes "OK" to Button constructor
    .withSize(100, 30)
    .build();
```

## Common Methods

All component types support these methods:

### Position and Size
```cpp
builder.withPosition(int x, int y)
builder.withSize(int width, int height)
builder.withBounds(int x, int y, int width, int height)
```

### State
```cpp
builder.withVisible(bool visible)
builder.withEnabled(bool enabled)
builder.withDebugName(const std::string& name)
```

### Callbacks
```cpp
builder.withFocusCallback(Component::FocusCallback callback)
```

### Layout
```cpp
builder.withLayout(std::shared_ptr<LayoutT> layout)
```

### Children
```cpp
builder.withChild(std::unique_ptr<Component> child)
builder.withChildren(std::unique_ptr<Component>... children)
```

## Component-Specific Methods

### Button

```cpp
auto button = create<Button>("Click Me")
    .withText("New Text")                    // Set button text
    .withClickCallback([]() { /* ... */ })   // Set click callback
    .build();
```

### TextInput

```cpp
auto textInput = create<TextInput>()
    .withPlaceholder("Enter text...")
    .withTextChangedCallback([](const std::string& text) {
        std::cout << "New text: " << text << "\n";
    })
    .build();
```

### ListBox

```cpp
std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};

auto listBox = create<ListBox>()
    .withItems(items)
    .withSelectionCallback([](int index, const std::string& item) {
        std::cout << "Selected: " << item << "\n";
    })
    .build();
```

## Building Component Hierarchies

### Method 1: Inline Creation

Create and add children in a single expression:

```cpp
auto panel = create<Panel<>>()
    .withSize(400, 300)
    .withChildren(
        create<Button>("OK")
            .withSize(100, 30)
            .withPosition(10, 260)
            .withClickCallback([]() { /* ... */ })
            .build(),
        create<Button>("Cancel")
            .withSize(100, 30)
            .withPosition(120, 260)
            .withClickCallback([]() { /* ... */ })
            .build()
    )
    .build();
```

### Method 2: Keep References

When you need to access components after creation:

```cpp
// Create and store
auto submitBtn = create<Button>("Submit")
    .withSize(100, 30)
    .build();

auto cancelBtn = create<Button>("Cancel")
    .withSize(100, 30)
    .build();

// Configure with access to other components
submitBtn->setClickCallback([submitBtn = submitBtn.get()]() {
    submitBtn->setEnabled(false);
    // Process...
});

// Add to parent
auto panel = create<Panel<>>()
    .withChildren(
        std::move(submitBtn),
        std::move(cancelBtn)
    )
    .build();
```

### Method 3: Shared Pointers for Complex Interactions

When multiple components need to reference each other:

```cpp
std::shared_ptr<TextInput> nameInput;
std::shared_ptr<TextInput> emailInput;
std::shared_ptr<ListBox> resultList;

// Create components
nameInput = std::shared_ptr<TextInput>(
    create<TextInput>()
        .withPlaceholder("Name...")
        .build()
);

emailInput = std::shared_ptr<TextInput>(
    create<TextInput>()
        .withPlaceholder("Email...")
        .build()
);

resultList = std::shared_ptr<ListBox>(
    create<ListBox>().build()
);

// Create button with access to all inputs
auto submitButton = create<Button>("Add")
    .withClickCallback([nameInput, emailInput, resultList]() {
        std::string entry = nameInput->getText() + " <" + emailInput->getText() + ">";
        // Add to list...
    })
    .build();
```

## Advanced Usage

### Direct Access with operator->

Use the arrow operator to access the component directly while still in the builder:

```cpp
auto builder = create<Button>("Process");

// Direct access to component
builder->setSize(150, 40);
builder->setPosition(10, 10);

// Continue with fluent interface
builder.withClickCallback([]() { /* ... */ });

// Finally build
auto button = builder.build();
```

### Conditional Configuration

```cpp
auto buttonBuilder = create<Button>("Action");

// Configure based on runtime conditions
if (needsConfirmation) {
    buttonBuilder.withClickCallback([]() {
        showConfirmDialog();
    });
} else {
    buttonBuilder.withClickCallback([]() {
        executeDirectly();
    });
}

auto button = buttonBuilder.build();
```

## Type Safety

The builder uses `std::enable_if_t` to ensure component-specific methods are only available for the correct types:

```cpp
// This compiles - Button has setClickCallback
auto button = create<Button>()
    .withClickCallback([]() {})  // ✓ OK
    .build();

// This won't compile - Panel doesn't have withClickCallback
auto panel = create<Panel<>>()
    .withClickCallback([]() {})  // ✗ Compiler error
    .build();
```

## Best Practices

### 1. Use Inline Creation for Simple UIs

For simple, static UIs without complex interactions:

```cpp
auto toolbar = create<Panel<FlexLayout>>()
    .withChildren(
        create<Button>("File").build(),
        create<Button>("Edit").build(),
        create<Button>("View").build()
    )
    .build();
```

### 2. Keep References for Interactive Components

When components need to interact:

```cpp
class MyForm {
private:
    std::unique_ptr<TextInput> nameField;
    std::unique_ptr<Button> submitButton;

public:
    void buildUI() {
        nameField = create<TextInput>()
            .withPlaceholder("Name...")
            .build();

        submitButton = create<Button>("Submit")
            .withClickCallback([this]() {
                processForm();
            })
            .build();
    }

    void processForm() {
        std::string name = nameField->getText();
        // ...
    }
};
```

### 3. Use Shared Pointers for Complex Dependencies

When multiple components need bidirectional references:

```cpp
std::shared_ptr<TextInput> input = /* ... */;
std::shared_ptr<Button> clearBtn = /* ... */;

// Button can clear input
clearBtn->setClickCallback([input]() {
    input->clear();
});

// Input can enable/disable button
input->setOnTextChanged([clearBtn](const std::string& text) {
    clearBtn->setEnabled(!text.empty());
});
```

### 4. Prefer Composition Over Deep Nesting

Instead of deeply nested builders:

```cpp
// Better - compose separately
auto topBar = buildTopBar();
auto mainContent = buildMainContent();
auto bottomBar = buildBottomBar();

auto mainWindow = create<Panel<>>()
    .withChildren(
        std::move(topBar),
        std::move(mainContent),
        std::move(bottomBar)
    )
    .build();
```

## Integration with Layout Managers

The builder works seamlessly with layout managers:

```cpp
#include <bombfork/prong/layout/flex_layout.h>

auto flexLayout = std::make_shared<layout::FlexLayout>();
flexLayout->configure({
    .direction = layout::FlexDirection::ROW,
    .justify = layout::FlexJustify::SPACE_BETWEEN,
    .gap = 10.0f
});

auto panel = create<Panel<>>()
    .withLayout(flexLayout)
    .withChildren(
        create<Button>("One").build(),
        create<Button>("Two").build(),
        create<Button>("Three").build()
    )
    .build();
```

## Complete Example

See `examples/component_builder_example.cpp` for a comprehensive example demonstrating all features of the ComponentBuilder pattern.

## Notes

- Components created with the builder don't require a renderer parameter
- The renderer is automatically inherited from the parent when `addChild()` is called
- The builder uses move semantics, so `build()` transfers ownership
- After calling `build()`, the builder is empty and cannot be reused
- Template type deduction works with `create<>()`, no need to specify template parameters explicitly

## See Also

- [Component Base Class](component.md)
- [Layout Managers](layout_managers.md)
- [Event Handling](events.md)
