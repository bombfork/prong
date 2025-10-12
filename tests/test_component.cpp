#include <bombfork/prong/core/component.h>
#include <bombfork/prong/layout/layout_manager.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bombfork::prong::rendering {
class IRenderer;
}

using namespace bombfork::prong;

// Simple concrete component for testing (Component is abstract)
class TestComponent : public Component {
public:
  explicit TestComponent(rendering::IRenderer* renderer = nullptr, const std::string& name = "TestComponent")
    : Component(renderer, name) {}

  void update(double deltaTime) override {
    // No-op for tests
    (void)deltaTime;
  }

  void render() override {
    // No-op for tests
  }
};

// Simple test layout manager
template <typename DerivedT>
class TestLayoutManager : public layout::LayoutManager<DerivedT> {
public:
  layout::Dimensions measureLayout(const std::vector<Component*>& components) override {
    (void)components;
    return {100, 50};
  }

  void layout(std::vector<Component*>& components, const layout::Dimensions& availableSpace) override {
    (void)components;
    (void)availableSpace;
  }
};

class ConcreteTestLayout : public TestLayoutManager<ConcreteTestLayout> {};

void test_layout_setter_and_getter() {
  TestComponent component;

  // Initially no layout
  assert(!component.hasLayout());

  // Set a layout
  auto layout = std::make_shared<ConcreteTestLayout>();
  component.setLayout(layout);
  assert(component.hasLayout());

  // Clear layout
  component.clearLayout();
  assert(!component.hasLayout());

  std::cout << "✓ Layout setter and getter tests passed\n";
}

void test_layout_invalidation() {
  TestComponent parent;
  auto layout = std::make_shared<ConcreteTestLayout>();
  parent.setLayout(layout);

  // Adding a child should invalidate layout
  auto child = std::make_unique<TestComponent>();
  parent.addChild(std::move(child));

  // Layout should be marked as invalid (we can't directly test this,
  // but performLayout() should work without errors)
  parent.performLayout();

  std::cout << "✓ Layout invalidation tests passed\n";
}

void test_preferred_size() {
  TestComponent component;

  // Set some dimensions
  component.setSize(200, 150);

  // Get preferred size
  auto preferredSize = component.getPreferredSize();
  assert(preferredSize.width == 200);
  assert(preferredSize.height == 150);

  std::cout << "✓ Preferred size tests passed\n";
}

void test_perform_layout() {
  TestComponent parent;

  // Set a layout manager
  auto layout = std::make_shared<ConcreteTestLayout>();
  parent.setLayout(layout);

  // Add some children
  parent.addChild(std::make_unique<TestComponent>());
  parent.addChild(std::make_unique<TestComponent>());
  parent.addChild(std::make_unique<TestComponent>());

  // Perform layout should not crash
  parent.performLayout();

  std::cout << "✓ Perform layout tests passed\n";
}

void test_renderer_inheritance() {
  TestComponent parent(nullptr, "parent");
  auto child1 = std::make_unique<TestComponent>(nullptr, "child1");
  auto child2 = std::make_unique<TestComponent>(nullptr, "child2");

  // Store raw pointers for checking (before moving)
  TestComponent* child1Ptr = child1.get();
  TestComponent* child2Ptr = child2.get();

  parent.addChild(std::move(child1));
  parent.addChild(std::move(child2));

  // Create a mock renderer (just using nullptr since we don't have a real implementation)
  rendering::IRenderer* mockRenderer = reinterpret_cast<rendering::IRenderer*>(0x1234);

  // Set renderer on parent
  parent.setRenderer(mockRenderer);

  // Children should inherit the renderer
  assert(child1Ptr->getRenderer() == mockRenderer);
  assert(child2Ptr->getRenderer() == mockRenderer);

  std::cout << "✓ Renderer inheritance tests passed\n";
}

void test_addchild_triggers_invalidation() {
  TestComponent parent;

  // Set a layout
  auto layout = std::make_shared<ConcreteTestLayout>();
  parent.setLayout(layout);

  // Perform layout to mark it as valid
  parent.performLayout();

  // Add a child (should invalidate layout)
  parent.addChild(std::make_unique<TestComponent>());

  // Perform layout again - should work without issues
  parent.performLayout();

  std::cout << "✓ AddChild triggers invalidation tests passed\n";
}

int main() {
  try {
    std::cout << "Running Component layout tests...\n\n";

    test_layout_setter_and_getter();
    test_layout_invalidation();
    test_preferred_size();
    test_perform_layout();
    test_renderer_inheritance();
    test_addchild_triggers_invalidation();

    std::cout << "\n✓ All Component layout tests passed!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception\n";
    return 1;
  }
}
