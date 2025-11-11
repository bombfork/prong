# Custom Component Example

Demonstrates creating a custom ColorPicker component from scratch, showing all aspects of component development in Prong.

## Key Topics

- **CRTP pattern**: Inheriting from Component base class
- **update() and render()**: Implementing component lifecycle
- **Event handling**: Custom mouse interaction
- **State management**: Internal color state
- **Builder pattern**: Fluent API for construction
- **Callbacks**: Notifying changes to parent

## ColorPicker Component

A simple color picker with:

- 18 color swatches in a grid
- Click to select color
- Visual feedback for selection
- Preview area showing selected color
- Callback for color changes

## Component Structure

```cpp
class ColorPicker : public Component {
private:
  float selectedR, selectedG, selectedB;
  ColorChangeCallback onColorChange;

public:
  void update(double deltaTime) override;
  void render() override;

protected:
  bool handleEventSelf(const Event& event) override;
};
```

## Builder Pattern

```cpp
auto picker = ColorPickerBuilder(renderer)
                 .withPosition(50, 50)
                 .withColor(1.0f, 0.5f, 0.0f)
                 .withCallback([](float r, float g, float b) {
                   // Handle color change
                 })
                 .build();
```

## Building

```bash
mise build-examples
./build/examples/advanced/01_custom_component/01_custom_component
```
