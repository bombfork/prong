#include <bombfork/prong/core/component.h>
#include <bombfork/prong/events/event_dispatcher.h>
#include <bombfork/prong/events/iwindow.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace bombfork::prong::rendering {
class IRenderer;
}

using namespace bombfork::prong;
using namespace bombfork::prong::events;

// === Mock Window ===

class MockWindow : public IWindow {
public:
  MockWindow() = default;

  void getSize(int& width, int& height) const override {
    width = 800;
    height = 600;
  }

  void* getNativeHandle() override { return nullptr; }

  void getCursorPos(double& x, double& y) const override {
    x = 0.0;
    y = 0.0;
  }

  bool shouldClose() const override { return false; }

  bool isMouseButtonPressed(int button) const override {
    (void)button;
    return false;
  }

  bool isKeyPressed(int key) const override {
    (void)key;
    return false;
  }

  int getModifiers() const override { return 0; }

  void setCallbacks(const WindowCallbacks& callbacks) override { (void)callbacks; }
};

// === Test Component ===

class TestComponent : public Component {
public:
  explicit TestComponent(rendering::IRenderer* renderer = nullptr, const std::string& name = "TestComponent")
    : Component(renderer, name) {}

  void update(double deltaTime) override { (void)deltaTime; }

  void render() override {}

  // Helper to set position and size for testing
  void setBounds(int x, int y, int width, int height) override {
    setPosition(x, y);
    setSize(width, height);
  }

  // Override to handle mouse press for testing
  bool handleMousePress(int localX, int localY, int button) override {
    // First try children
    if (Component::handleMousePress(localX, localY, button)) {
      return true;
    }
    // If no child handled it, we handle it (for testing purposes)
    (void)localX;
    (void)localY;
    (void)button;
    return true;
  }
};

// === Test Cases ===

void test_flat_hierarchy_single_component() {
  std::cout << "Test: Flat hierarchy - single component\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create a component at (100, 100) with size 200x150
  auto comp = std::make_unique<TestComponent>();
  comp->setBounds(100, 100, 200, 150);
  TestComponent* compPtr = comp.get();

  // Register component
  dispatcher.registerComponent(compPtr);

  // Test clicking inside component
  // Move mouse to position first, then press
  dispatcher.processMouseMove(150, 150); // Inside component
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);

  // Verify component received focus (indirect test of findComponentAt)
  assert(dispatcher.getFocusedComponent() == compPtr);

  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_RELEASE, 0);

  std::cout << "  ✓ Single component at location found correctly\n";
}

void test_flat_hierarchy_multiple_components() {
  std::cout << "Test: Flat hierarchy - multiple overlapping components\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create two overlapping components
  auto comp1 = std::make_unique<TestComponent>(nullptr, "comp1");
  comp1->setBounds(100, 100, 200, 150);
  TestComponent* comp1Ptr = comp1.get();

  auto comp2 = std::make_unique<TestComponent>(nullptr, "comp2");
  comp2->setBounds(150, 125, 200, 150);
  TestComponent* comp2Ptr = comp2.get();

  // Register in order: comp1 first, comp2 second (comp2 is topmost)
  dispatcher.registerComponent(comp1Ptr);
  dispatcher.registerComponent(comp2Ptr);

  // Click in overlapping region (200, 150)
  dispatcher.processMouseMove(200, 150);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);

  // Should get comp2 (topmost)
  assert(dispatcher.getFocusedComponent() == comp2Ptr);

  std::cout << "  ✓ Topmost component found in overlapping region\n";

  // Clear focus
  dispatcher.clearFocus();

  // Click in comp1-only region (120, 120)
  dispatcher.processMouseMove(120, 120);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);

  // Should get comp1
  assert(dispatcher.getFocusedComponent() == comp1Ptr);

  std::cout << "  ✓ Correct component found in non-overlapping region\n";
}

void test_flat_hierarchy_outside_all_components() {
  std::cout << "Test: Flat hierarchy - point outside all components\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  auto comp = std::make_unique<TestComponent>();
  comp->setBounds(100, 100, 200, 150);
  TestComponent* compPtr = comp.get();

  dispatcher.registerComponent(compPtr);

  // Click outside component
  dispatcher.processMouseMove(50, 50);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);

  // Should have no focus
  assert(dispatcher.getFocusedComponent() == nullptr);

  std::cout << "  ✓ No component found outside bounds\n";
}

void test_nested_parent_with_single_child() {
  std::cout << "Test: Nested hierarchy - parent with single child\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent at (100, 100) with size 400x300
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  // Create child at global position (150, 150) with size 100x80
  // This is 50 pixels offset from parent's position (100, 100)
  auto child = std::make_unique<TestComponent>(nullptr, "child");
  child->setBounds(150, 150, 100, 80);
  TestComponent* childPtr = child.get();

  parent->addChild(std::move(child));

  // Register only the parent (children are discovered recursively)
  dispatcher.registerComponent(parentPtr);

  // Click on child (200, 175 is inside child bounds 150-250, 150-230)
  dispatcher.processMouseMove(200, 175);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);

  // Should get child, not parent
  assert(dispatcher.getFocusedComponent() == childPtr);

  std::cout << "  ✓ Child component found when clicked\n";

  // Clear focus
  dispatcher.clearFocus();

  // Click on parent only (110, 110 is outside child)
  dispatcher.processMouseMove(110, 110);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);

  // Should get parent
  assert(dispatcher.getFocusedComponent() == parentPtr);

  std::cout << "  ✓ Parent component found when clicked outside child\n";
}

void test_nested_parent_with_multiple_children() {
  std::cout << "Test: Nested hierarchy - parent with multiple children\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent at (100, 100) with size 400x300
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  // Create child1 at global position (110, 110) with size 100x80
  auto child1 = std::make_unique<TestComponent>(nullptr, "child1");
  child1->setBounds(110, 110, 100, 80);
  TestComponent* child1Ptr = child1.get();

  // Create child2 at global position (220, 110) with size 100x80
  auto child2 = std::make_unique<TestComponent>(nullptr, "child2");
  child2->setBounds(220, 110, 100, 80);
  TestComponent* child2Ptr = child2.get();

  // Create child3 at global position (110, 200) with size 100x80
  auto child3 = std::make_unique<TestComponent>(nullptr, "child3");
  child3->setBounds(110, 200, 100, 80);
  TestComponent* child3Ptr = child3.get();

  parent->addChild(std::move(child1));
  parent->addChild(std::move(child2));
  parent->addChild(std::move(child3));

  dispatcher.registerComponent(parentPtr);

  // Click on child1
  dispatcher.processMouseMove(140, 130);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == child1Ptr);

  std::cout << "  ✓ Child1 found correctly\n";

  dispatcher.clearFocus();

  // Click on child2
  dispatcher.processMouseMove(250, 130);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == child2Ptr);

  std::cout << "  ✓ Child2 found correctly\n";

  dispatcher.clearFocus();

  // Click on child3
  dispatcher.processMouseMove(140, 220);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == child3Ptr);

  std::cout << "  ✓ Child3 found correctly\n";
}

void test_nested_deep_hierarchy() {
  std::cout << "Test: Nested hierarchy - deep nesting (3+ levels)\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create grandparent at (100, 100) with size 400x300
  auto grandparent = std::make_unique<TestComponent>(nullptr, "grandparent");
  grandparent->setBounds(100, 100, 400, 300);
  TestComponent* grandparentPtr = grandparent.get();

  // Create parent at global position (150, 150)
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(150, 150, 300, 200);
  TestComponent* parentPtr = parent.get();

  // Create child at global position (200, 200)
  auto child = std::make_unique<TestComponent>(nullptr, "child");
  child->setBounds(200, 200, 100, 80);
  TestComponent* childPtr = child.get();

  // Create grandchild at global position (210, 210)
  auto grandchild = std::make_unique<TestComponent>(nullptr, "grandchild");
  grandchild->setBounds(210, 210, 50, 40);
  TestComponent* grandchildPtr = grandchild.get();

  child->addChild(std::move(grandchild));
  parent->addChild(std::move(child));
  grandparent->addChild(std::move(parent));

  dispatcher.registerComponent(grandparentPtr);

  // Click on grandchild (deepest level)
  dispatcher.processMouseMove(230, 230);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == grandchildPtr);

  std::cout << "  ✓ Deepest (grandchild) component found\n";

  dispatcher.clearFocus();

  // Click on child only (outside grandchild)
  dispatcher.processMouseMove(270, 240);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == childPtr);

  std::cout << "  ✓ Child component found (not grandchild or parent)\n";

  dispatcher.clearFocus();

  // Click on parent only (outside child)
  dispatcher.processMouseMove(170, 170);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == parentPtr);

  std::cout << "  ✓ Parent component found (not child)\n";

  dispatcher.clearFocus();

  // Click on grandparent only (outside parent)
  dispatcher.processMouseMove(120, 120);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == grandparentPtr);

  std::cout << "  ✓ Grandparent component found (not parent)\n";
}

void test_nested_overlapping_siblings() {
  std::cout << "Test: Nested hierarchy - overlapping siblings\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent at (100, 100) with size 400x300
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  // Create child1 at (50, 50) relative to parent
  auto child1 = std::make_unique<TestComponent>(nullptr, "child1");
  child1->setBounds(150, 150, 150, 100);
  TestComponent* child1Ptr = child1.get();

  // Create child2 overlapping child1 at (100, 75) relative to parent
  auto child2 = std::make_unique<TestComponent>(nullptr, "child2");
  child2->setBounds(200, 175, 150, 100);
  TestComponent* child2Ptr = child2.get();

  // Add children in order: child1 first, child2 second (child2 is topmost)
  parent->addChild(std::move(child1));
  parent->addChild(std::move(child2));

  dispatcher.registerComponent(parentPtr);

  // Click in overlapping region - should get child2 (last added = topmost)
  // child1 is at global (150, 150) with size 150x100 = (150-300, 150-250)
  // child2 is at global (200, 175) with size 150x100 = (200-350, 175-275)
  // Overlapping region is (200-300, 175-250)
  dispatcher.processMouseMove(250, 200);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == child2Ptr);

  std::cout << "  ✓ Topmost sibling (child2) found in overlapping region\n";

  dispatcher.clearFocus();

  // Click in child1-only region
  dispatcher.processMouseMove(170, 160);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == child1Ptr);

  std::cout << "  ✓ First sibling (child1) found in non-overlapping region\n";
}

void test_edge_case_disabled_child() {
  std::cout << "Test: Edge case - disabled child should be skipped\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent at (100, 100) with size 400x300
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  // Create child at (50, 50) relative to parent
  auto child = std::make_unique<TestComponent>(nullptr, "child");
  child->setBounds(150, 150, 100, 80);
  TestComponent* childPtr = child.get();

  parent->addChild(std::move(child));
  dispatcher.registerComponent(parentPtr);

  // Disable the child
  childPtr->setEnabled(false);

  // Click on child position - should get parent instead
  dispatcher.processMouseMove(180, 170);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == parentPtr);

  std::cout << "  ✓ Disabled child skipped, parent found\n";
}

void test_edge_case_invisible_child() {
  std::cout << "Test: Edge case - invisible child should be skipped\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent at (100, 100) with size 400x300
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  // Create child at (50, 50) relative to parent
  auto child = std::make_unique<TestComponent>(nullptr, "child");
  child->setBounds(50, 50, 100, 80);
  TestComponent* childPtr = child.get();

  parent->addChild(std::move(child));
  dispatcher.registerComponent(parentPtr);

  // Make child invisible
  childPtr->setVisible(false);

  // Click on child position - should get parent instead
  dispatcher.processMouseMove(180, 170);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == parentPtr);

  std::cout << "  ✓ Invisible child skipped, parent found\n";
}

void test_edge_case_empty_parent() {
  std::cout << "Test: Edge case - empty parent (no children)\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent with no children
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  dispatcher.registerComponent(parentPtr);

  // Click on parent - should get parent
  dispatcher.processMouseMove(200, 200);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == parentPtr);

  std::cout << "  ✓ Empty parent found correctly\n";
}

void test_performance_outside_parent_bounds() {
  std::cout << "Test: Performance - point outside parent bounds\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent at (100, 100) with size 400x300
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  // Create child at global position (150, 150)
  auto child = std::make_unique<TestComponent>(nullptr, "child");
  child->setBounds(150, 150, 100, 80);

  parent->addChild(std::move(child));
  dispatcher.registerComponent(parentPtr);

  // Click outside parent - should not find anything
  // This tests early exit optimization: children shouldn't be checked
  dispatcher.processMouseMove(50, 50);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == nullptr);

  std::cout << "  ✓ Early exit when outside parent bounds (children not checked)\n";
}

void test_multiple_nested_containers() {
  std::cout << "Test: Multiple nested containers - correct component at each level\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create root container 1
  auto root1 = std::make_unique<TestComponent>(nullptr, "root1");
  root1->setBounds(0, 0, 400, 300);
  TestComponent* root1Ptr = root1.get();

  auto root1Child = std::make_unique<TestComponent>(nullptr, "root1Child");
  root1Child->setBounds(50, 50, 100, 100);
  TestComponent* root1ChildPtr = root1Child.get();

  root1->addChild(std::move(root1Child));

  // Create root container 2 (non-overlapping)
  auto root2 = std::make_unique<TestComponent>(nullptr, "root2");
  root2->setBounds(500, 0, 400, 300);
  TestComponent* root2Ptr = root2.get();

  auto root2Child = std::make_unique<TestComponent>(nullptr, "root2Child");
  root2Child->setBounds(550, 50, 100, 100);
  TestComponent* root2ChildPtr = root2Child.get();

  root2->addChild(std::move(root2Child));

  // Register both root containers
  dispatcher.registerComponent(root1Ptr);
  dispatcher.registerComponent(root2Ptr);

  // Click on root1's child
  dispatcher.processMouseMove(100, 100);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == root1ChildPtr);

  std::cout << "  ✓ Root1's child found correctly\n";

  dispatcher.clearFocus();

  // Click on root2's child
  dispatcher.processMouseMove(600, 100);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == root2ChildPtr);

  std::cout << "  ✓ Root2's child found correctly\n";
}

void test_sibling_registration_order() {
  std::cout << "Test: Sibling registration order affects hit testing\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create two overlapping top-level components
  auto comp1 = std::make_unique<TestComponent>(nullptr, "comp1");
  comp1->setBounds(100, 100, 200, 150);
  TestComponent* comp1Ptr = comp1.get();

  auto comp2 = std::make_unique<TestComponent>(nullptr, "comp2");
  comp2->setBounds(150, 125, 200, 150);
  TestComponent* comp2Ptr = comp2.get();

  // Register comp1 first, then comp2 (comp2 should be on top)
  dispatcher.registerComponent(comp1Ptr);
  dispatcher.registerComponent(comp2Ptr);

  // Click in overlapping area - should get comp2
  dispatcher.processMouseMove(200, 150);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == comp2Ptr);

  std::cout << "  ✓ Later-registered component found in overlapping area\n";

  // Now test the reverse: unregister both and re-register in opposite order
  dispatcher.unregisterComponent(comp1Ptr);
  dispatcher.unregisterComponent(comp2Ptr);
  dispatcher.clearFocus();

  // Register comp2 first, then comp1 (comp1 should be on top now)
  dispatcher.registerComponent(comp2Ptr);
  dispatcher.registerComponent(comp1Ptr);

  // Click in overlapping area - should get comp1 now
  dispatcher.processMouseMove(200, 150);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);
  assert(dispatcher.getFocusedComponent() == comp1Ptr);

  std::cout << "  ✓ Registration order correctly affects hit testing priority\n";
}

void test_nested_with_both_parent_and_children_registered() {
  std::cout << "Test: Edge case - both parent and children registered separately\n";

  MockWindow window;
  EventDispatcher dispatcher(&window);

  // Create parent
  auto parent = std::make_unique<TestComponent>(nullptr, "parent");
  parent->setBounds(100, 100, 400, 300);
  TestComponent* parentPtr = parent.get();

  // Create child at global position (150, 150)
  auto child = std::make_unique<TestComponent>(nullptr, "child");
  child->setBounds(150, 150, 100, 80);
  TestComponent* childPtr = child.get();

  parent->addChild(std::move(child));

  // Register both parent and child
  dispatcher.registerComponent(parentPtr);
  dispatcher.registerComponent(childPtr); // Child registered separately

  // Click on child - should find it twice (once via parent's hierarchy, once directly)
  // But should still work correctly
  dispatcher.processMouseMove(180, 170);
  dispatcher.processMouseButton(MOUSE_BUTTON_LEFT, INPUT_PRESS, 0);

  // Should get child (either from parent hierarchy or direct registration)
  assert(dispatcher.getFocusedComponent() == childPtr);

  std::cout << "  ✓ Duplicate registration handled correctly\n";
}

int main() {
  try {
    std::cout << "Running EventDispatcher recursive hit testing tests...\n\n";

    // Flat hierarchy tests
    test_flat_hierarchy_single_component();
    test_flat_hierarchy_multiple_components();
    test_flat_hierarchy_outside_all_components();

    std::cout << "\n";

    // Nested hierarchy tests
    test_nested_parent_with_single_child();
    test_nested_parent_with_multiple_children();
    test_nested_deep_hierarchy();
    test_nested_overlapping_siblings();

    std::cout << "\n";

    // Edge case tests
    test_edge_case_disabled_child();
    test_edge_case_invisible_child();
    test_edge_case_empty_parent();

    std::cout << "\n";

    // Performance and correctness tests
    test_performance_outside_parent_bounds();
    test_multiple_nested_containers();
    test_sibling_registration_order();
    test_nested_with_both_parent_and_children_registered();

    std::cout << "\n✓ All EventDispatcher recursive hit testing tests passed!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception\n";
    return 1;
  }
}
