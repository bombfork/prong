/**
 * @file test_component_resize.cpp
 * @brief Unit tests for component resize handling
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

void testResizeBehaviorFixed() {
  std::cout << "Testing ResizeBehavior::FIXED..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("FixedChild");
  child->setBounds(100, 100, 200, 150);
  child->setResizeBehavior(Component::ResizeBehavior::FIXED);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // Initial size
  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 100 && y == 100 && w == 200 && h == 150);

  // Resize parent - FIXED should not change
  childPtr->onParentResize(1024, 768);

  childPtr->getBounds(x, y, w, h);
  assert(x == 100 && y == 100 && w == 200 && h == 150);

  std::cout << "✓ ResizeBehavior::FIXED test passed" << std::endl;
}

void testResizeBehaviorFill() {
  std::cout << "Testing ResizeBehavior::FILL..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("FillChild");
  child->setBounds(0, 0, 800, 600);
  child->setResizeBehavior(Component::ResizeBehavior::FILL);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // Resize parent larger - FILL should match parent size (grow)
  childPtr->onParentResize(1024, 768);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 0 && y == 0 && w == 1024 && h == 768);

  // Resize parent smaller - FILL should match parent size (shrink)
  childPtr->onParentResize(640, 480);
  childPtr->getBounds(x, y, w, h);
  assert(x == 0 && y == 0 && w == 640 && h == 480);

  std::cout << "✓ ResizeBehavior::FILL test passed (grow and shrink)" << std::endl;
}

void testResizeBehaviorScale() {
  std::cout << "Testing ResizeBehavior::SCALE..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("ScaleChild");
  child->setBounds(100, 100, 200, 150);
  child->setResizeBehavior(Component::ResizeBehavior::SCALE);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // First call to onParentResize establishes original parent size
  childPtr->onParentResize(800, 600);

  // Resize to 2x - should scale proportionally (grow)
  childPtr->onParentResize(1600, 1200);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(x == 200 && y == 200 && w == 400 && h == 300);

  // Resize to 0.5x - should scale proportionally (shrink)
  childPtr->onParentResize(400, 300);
  childPtr->getBounds(x, y, w, h);
  assert(x == 50 && y == 50 && w == 100 && h == 75);

  std::cout << "✓ ResizeBehavior::SCALE test passed (grow and shrink)" << std::endl;
}

void testResizeBehaviorMaintainAspect() {
  std::cout << "Testing ResizeBehavior::MAINTAIN_ASPECT..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("AspectChild");
  child->setBounds(0, 0, 400, 300); // 4:3 aspect ratio
  child->setResizeBehavior(Component::ResizeBehavior::MAINTAIN_ASPECT);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // First call establishes original parent size
  childPtr->onParentResize(800, 600);

  // Resize to different aspect ratio - child should maintain its aspect
  childPtr->onParentResize(1200, 600); // Wide parent

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);

  // Should maintain 4:3 aspect ratio and be centered
  assert(w == 400 && h == 300); // Same scale (1.0x) since height limited it

  std::cout << "✓ ResizeBehavior::MAINTAIN_ASPECT test passed" << std::endl;
}

void testResponsiveConstraints() {
  std::cout << "Testing ResponsiveConstraints..." << std::endl;

  TestComponent parent;
  parent.setBounds(0, 0, 800, 600);

  auto child = std::make_unique<TestComponent>("ConstrainedChild");
  child->setBounds(0, 0, 500, 400);
  child->setResizeBehavior(Component::ResizeBehavior::FILL);

  // Set constraints
  Component::ResponsiveConstraints constraints;
  constraints.minWidth = 200;
  constraints.minHeight = 150;
  constraints.maxWidth = 600;
  constraints.maxHeight = 450;
  child->setConstraints(constraints);

  auto* childPtr = child.get();
  parent.addChild(std::move(child));

  // Try to resize beyond max - should be clamped
  childPtr->onParentResize(1000, 1000);

  int x, y, w, h;
  childPtr->getBounds(x, y, w, h);
  assert(w == 600 && h == 450); // Clamped to max

  std::cout << "✓ ResponsiveConstraints test passed" << std::endl;
}

void testResizePropagation() {
  std::cout << "Testing resize propagation through hierarchy..." << std::endl;

  TestComponent root;
  root.setBounds(0, 0, 800, 600);

  auto parent = std::make_unique<TestComponent>("Parent");
  parent->setBounds(0, 0, 800, 600);
  parent->setResizeBehavior(Component::ResizeBehavior::FILL);

  auto child = std::make_unique<TestComponent>("Child");
  child->setBounds(0, 0, 800, 600);
  child->setResizeBehavior(Component::ResizeBehavior::FILL);

  auto* childPtr = child.get();
  parent->addChild(std::move(child));

  auto* parentPtr = parent.get();
  root.addChild(std::move(parent));

  // Resize root - should propagate to children
  parentPtr->onParentResize(1024, 768);

  int px, py, pw, ph;
  parentPtr->getBounds(px, py, pw, ph);
  assert(pw == 1024 && ph == 768);

  int cx, cy, cw, ch;
  childPtr->getBounds(cx, cy, cw, ch);
  assert(cw == 1024 && ch == 768);

  std::cout << "✓ Resize propagation test passed" << std::endl;
}

int main() {
  std::cout << "=== Component Resize Handling Tests ===" << std::endl << std::endl;

  try {
    testResizeBehaviorFixed();
    testResizeBehaviorFill();
    testResizeBehaviorScale();
    testResizeBehaviorMaintainAspect();
    testResponsiveConstraints();
    testResizePropagation();

    std::cout << std::endl << "✓ All resize handling tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "✗ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "✗ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
