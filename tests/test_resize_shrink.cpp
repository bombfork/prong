/**
 * @file test_resize_shrink.cpp
 * @brief Test component resize shrinking behavior
 */

#include <bombfork/prong/core/component.h>

#include <cassert>
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

int main() {
  std::cout << "=== Testing Resize SHRINKING Behavior ===" << std::endl << std::endl;

  // Test FILL behavior with shrinking
  {
    std::cout << "Test 1: FILL behavior - grow then shrink" << std::endl;
    TestComponent parent;
    parent.setBounds(0, 0, 800, 600);

    auto child = std::make_unique<TestComponent>("FillChild");
    child->setBounds(0, 0, 800, 600);
    child->setResizeBehavior(Component::ResizeBehavior::FILL);

    auto* childPtr = child.get();
    parent.addChild(std::move(child));

    // Grow
    std::cout << "  Growing to 1024x768..." << std::endl;
    childPtr->onParentResize(1024, 768);
    int x, y, w, h;
    childPtr->getBounds(x, y, w, h);
    std::cout << "  After grow: " << w << "x" << h << std::endl;
    assert(w == 1024 && h == 768);

    // Shrink
    std::cout << "  Shrinking to 640x480..." << std::endl;
    childPtr->onParentResize(640, 480);
    childPtr->getBounds(x, y, w, h);
    std::cout << "  After shrink: " << w << "x" << h << std::endl;
    assert(w == 640 && h == 480);

    std::cout << "  ✓ FILL shrink test PASSED" << std::endl << std::endl;
  }

  // Test SCALE behavior with shrinking
  {
    std::cout << "Test 2: SCALE behavior - grow then shrink" << std::endl;
    TestComponent parent;
    parent.setBounds(0, 0, 800, 600);

    auto child = std::make_unique<TestComponent>("ScaleChild");
    child->setBounds(100, 100, 200, 150);
    child->setResizeBehavior(Component::ResizeBehavior::SCALE);

    auto* childPtr = child.get();
    parent.addChild(std::move(child));

    // Establish original parent size
    std::cout << "  Establishing original parent size 800x600..." << std::endl;
    childPtr->onParentResize(800, 600);

    // Grow to 1600x1200 (2x)
    std::cout << "  Growing to 1600x1200 (2x)..." << std::endl;
    childPtr->onParentResize(1600, 1200);
    int x, y, w, h;
    childPtr->getBounds(x, y, w, h);
    std::cout << "  After grow: pos=(" << x << "," << y << ") size=" << w << "x" << h << std::endl;
    std::cout << "  Expected: pos=(200,200) size=400x300" << std::endl;
    assert(x == 200 && y == 200 && w == 400 && h == 300);

    // Shrink to 400x300 (0.5x of original)
    std::cout << "  Shrinking to 400x300 (0.5x)..." << std::endl;
    childPtr->onParentResize(400, 300);
    childPtr->getBounds(x, y, w, h);
    std::cout << "  After shrink: pos=(" << x << "," << y << ") size=" << w << "x" << h << std::endl;
    std::cout << "  Expected: pos=(50,50) size=100x75" << std::endl;

    if (x == 50 && y == 50 && w == 100 && h == 75) {
      std::cout << "  ✓ SCALE shrink test PASSED" << std::endl << std::endl;
    } else {
      std::cout << "  ✗ SCALE shrink test FAILED - sizes don't match!" << std::endl;
      std::cout << "  This confirms the bug: SCALE doesn't work properly when shrinking!" << std::endl << std::endl;
      return 1;
    }
  }

  std::cout << "✓ All shrinking tests passed!" << std::endl;
  return 0;
}
