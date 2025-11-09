/**
 * @file test_axis_resize.cpp
 * @brief Unit tests for per-axis resize behavior
 */

#include <bombfork/prong/core/component.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using namespace bombfork::prong;

// Simple concrete component for testing
class TestComponent : public Component {
public:
  explicit TestComponent(const std::string& name = "TestComponent") : Component(nullptr, name) {}

  void update(double deltaTime) override { (void)deltaTime; }
  void render() override {}
};

void testAxisResizeBehaviorFixedFixed() {
  std::cout << "Testing AxisResizeBehavior::FIXED on both axes..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("FixedChild");
  child->setBounds(100, 100, 200, 150);
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::FIXED, Component::AxisResizeBehavior::FIXED);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // Initial size
  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 100 && y == 100 && w == 200 && h == 150);

  // Resize parent - both axes should stay FIXED
  childPtr->onParentResize(1024, 768);

  childPtr->getBounds(x, y, w, h);
  assert(x == 100 && y == 100 && w == 200 && h == 150);

  std::cout << "✓ AxisResizeBehavior::FIXED/FIXED test passed" << std::endl;
}

void testAxisResizeBehaviorFixedFill() {
  std::cout << "Testing AxisResizeBehavior::FIXED horizontal, FILL vertical..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("FixedFillChild");
  child->setBounds(100, 0, 200, 600);
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::FIXED, Component::AxisResizeBehavior::FILL);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // First call establishes original dimensions
  childPtr->onParentResize(800, 600);

  // Resize parent - horizontal should stay fixed, vertical should fill
  childPtr->onParentResize(1024, 768);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 100 && y == 0 && w == 200 && h == 768); // Width stays 200, height fills 768

  // Resize smaller
  childPtr->onParentResize(640, 480);
  childPtr->getBounds(x, y, w, h);
  assert(x == 100 && y == 0 && w == 200 && h == 480); // Width stays 200, height fills 480

  std::cout << "✓ AxisResizeBehavior::FIXED/FILL test passed" << std::endl;
}

void testAxisResizeBehaviorFillFixed() {
  std::cout << "Testing AxisResizeBehavior::FILL horizontal, FIXED vertical..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("FillFixedChild");
  child->setBounds(0, 100, 800, 150);
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::FILL, Component::AxisResizeBehavior::FIXED);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // First call establishes original dimensions
  childPtr->onParentResize(800, 600);

  // Resize parent - horizontal should fill, vertical should stay fixed
  childPtr->onParentResize(1024, 768);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 0 && y == 100 && w == 1024 && h == 150); // Width fills 1024, height stays 150

  // Resize smaller
  childPtr->onParentResize(640, 480);
  childPtr->getBounds(x, y, w, h);
  assert(x == 0 && y == 100 && w == 640 && h == 150); // Width fills 640, height stays 150

  std::cout << "✓ AxisResizeBehavior::FILL/FIXED test passed" << std::endl;
}

void testAxisResizeBehaviorFillFill() {
  std::cout << "Testing AxisResizeBehavior::FILL on both axes..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("FillFillChild");
  child->setBounds(0, 0, 800, 600);
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::FILL, Component::AxisResizeBehavior::FILL);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // Resize parent - both axes should fill
  childPtr->onParentResize(1024, 768);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 0 && y == 0 && w == 1024 && h == 768);

  // Resize smaller
  childPtr->onParentResize(640, 480);
  childPtr->getBounds(x, y, w, h);
  assert(x == 0 && y == 0 && w == 640 && h == 480);

  std::cout << "✓ AxisResizeBehavior::FILL/FILL test passed" << std::endl;
}

void testAxisResizeBehaviorScaleScale() {
  std::cout << "Testing AxisResizeBehavior::SCALE on both axes..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("ScaleScaleChild");
  child->setBounds(100, 100, 200, 150);
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::SCALE, Component::AxisResizeBehavior::SCALE);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // First call establishes original dimensions
  childPtr->onParentResize(800, 600);

  // Resize to 2x
  childPtr->onParentResize(1600, 1200);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 200 && y == 200 && w == 400 && h == 300); // Everything scaled by 2

  // Resize to 0.5x
  childPtr->onParentResize(400, 300);
  childPtr->getBounds(x, y, w, h);
  assert(x == 50 && y == 50 && w == 100 && h == 75); // Everything scaled by 0.5

  std::cout << "✓ AxisResizeBehavior::SCALE/SCALE test passed" << std::endl;
}

void testAxisResizeBehaviorMixedScale() {
  std::cout << "Testing AxisResizeBehavior::SCALE horizontal, FIXED vertical..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("ScaleFixedChild");
  child->setBounds(100, 100, 200, 150);
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::SCALE, Component::AxisResizeBehavior::FIXED);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // First call establishes original dimensions
  childPtr->onParentResize(800, 600);

  // Resize to 2x width, 1.5x height (but height should stay fixed)
  childPtr->onParentResize(1600, 900);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 200 && y == 100 && w == 400 && h == 150); // Width scaled by 2, height stays 150

  std::cout << "✓ AxisResizeBehavior::SCALE/FIXED test passed" << std::endl;
}

void testUnifiedBehaviorStillWorksAfterAxisBehavior() {
  std::cout << "Testing that unified behavior still works after setting axis behavior..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("UnifiedChild");
  child->setBounds(0, 0, 800, 600);

  // First set axis behavior
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::FILL, Component::AxisResizeBehavior::FILL);

  // Then switch back to unified behavior
  child->setResizeBehavior(Component::ResizeBehavior::FIXED);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // Resize parent - should use FIXED behavior (unified)
  childPtr->onParentResize(1024, 768);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 0 && y == 0 && w == 800 && h == 600); // Should stay at original size

  std::cout << "✓ Unified behavior after axis behavior test passed" << std::endl;
}

int main() {
  std::cout << "=== Per-Axis Resize Behavior Tests ===" << std::endl << std::endl;

  try {
    testAxisResizeBehaviorFixedFixed();
    testAxisResizeBehaviorFixedFill();
    testAxisResizeBehaviorFillFixed();
    testAxisResizeBehaviorFillFill();
    testAxisResizeBehaviorScaleScale();
    testAxisResizeBehaviorMixedScale();
    testUnifiedBehaviorStillWorksAfterAxisBehavior();

    std::cout << std::endl << "✓ All per-axis resize behavior tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "✗ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "✗ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
