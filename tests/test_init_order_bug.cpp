/**
 * @file test_init_order_bug.cpp
 * @brief Test to reproduce the initialization order bug in issue #126
 */

#include <bombfork/prong/core/component.h>

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
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

void testInitializationOrderBug() {
  std::cout << "Testing initialization order bug (issue #126)..." << std::endl;
  std::cout << "Reproducing: setAxisResizeBehavior called BEFORE layout runs" << std::endl << std::endl;

  // Simulate demo app scenario
  auto parent = std::make_unique<TestComponent>("Parent");
  parent->setBounds(0, 0, 800, 600);

  // Create child with width=300, height=0 (simulating withSize(300, 0))
  auto child = std::make_unique<TestComponent>("LeftPanel");
  child->setBounds(0, 0, 300, 0); // Height is 0!

  // Set axis resize behavior IMMEDIATELY (this used to be the bug!)
  child->setAxisResizeBehavior(Component::AxisResizeBehavior::FIXED, Component::AxisResizeBehavior::FILL);

  // Check current dimensions
  std::cout << "After setAxisResizeBehavior (before layout):" << std::endl;
  int w1, h1;
  child->getSize(w1, h1);
  std::cout << "  Current size: " << w1 << " x " << h1 << std::endl;
  std::cout << "  NOTE: With fix, originalHeight is NOT initialized yet (lazy initialization)" << std::endl
            << std::endl;

  // Now add to parent and simulate layout setting proper height
  auto* childPtr = child.get();
  parent->addChild(std::move(child));

  // Simulate parent resize (which triggers onParentResize on children)
  // This is what happens during initial layout when parent's size is set
  std::cout << "Parent's size changes, triggering onParentResize(800, 600) on child..." << std::endl;
  childPtr->onParentResize(800, 600);

  int x2, y2, w2, h2;
  childPtr->getBounds(x2, y2, w2, h2);
  std::cout << "  Child size after first onParentResize: " << w2 << " x " << h2 << std::endl;
  std::cout << "  Expected (FIXED horizontal=300, FILL vertical=600): 300 x 600" << std::endl;

  if (w2 != 300 || h2 != 600) {
    std::cout << "  ✗ FAILED: First resize gave incorrect size!" << std::endl;
    throw std::runtime_error("First resize failed");
  }
  std::cout << "  ✓ First resize correct!" << std::endl << std::endl;

  // Now simulate window resize (second resize)
  std::cout << "Window resizes to 1024x768..." << std::endl;
  childPtr->onParentResize(1024, 768);

  int x3, y3, w3, h3;
  childPtr->getBounds(x3, y3, w3, h3);
  std::cout << "  After second onParentResize: " << w3 << " x " << h3 << std::endl;
  std::cout << "  Expected (FIXED horizontal=300, FILL vertical=768): 300 x 768" << std::endl;

  if (w3 != 300 || h3 != 768) {
    std::cout << "  ✗ FAILED: Second resize gave incorrect size!" << std::endl;
    std::cout << "     This is the 'jumping effect' - layout differs after window resize" << std::endl;
    throw std::runtime_error("Second resize failed - jumping effect detected");
  }
  std::cout << "  ✓ Second resize correct!" << std::endl << std::endl;

  // Resize back smaller
  std::cout << "Window resizes to 640x480..." << std::endl;
  childPtr->onParentResize(640, 480);

  int x4, y4, w4, h4;
  childPtr->getBounds(x4, y4, w4, h4);
  std::cout << "  After third onParentResize: " << w4 << " x " << h4 << std::endl;
  std::cout << "  Expected (FIXED horizontal=300, FILL vertical=480): 300 x 480" << std::endl;

  if (w4 != 300 || h4 != 480) {
    std::cout << "  ✗ FAILED: Third resize gave incorrect size!" << std::endl;
    throw std::runtime_error("Third resize failed");
  }
  std::cout << "  ✓ Third resize correct!" << std::endl << std::endl;

  std::cout << "✓ Bug is FIXED: No jumping effect, all resizes consistent!" << std::endl;
}

int main() {
  std::cout << "=== Issue #126: Initialization Order Bug Test ===" << std::endl << std::endl;

  try {
    testInitializationOrderBug();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "✗ Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
