# Custom Layout Example - CircularLayout

Demonstrates creating a custom layout manager that positions children in a circle.

## CircularLayout Implementation

```cpp
template <typename ParentT>
class CircularLayout : public LayoutManager<CircularLayout<ParentT>> {
  Dimensions measureLayout(const std::vector<Component*>& components) override;
  void layout(std::vector<Component*>& components, const Dimensions& available) override;
};
```

## Key Concepts

- **CRTP pattern**: Inherit from LayoutManager<YourLayout>
- **measureLayout()**: Return dimensions needed for all children
- **layout()**: Position children in available space
- **Trigonometry**: Use sin/cos to calculate circular positions

## Building

```bash
mise build-examples
./build/examples/advanced/02_custom_layout/02_custom_layout
```
