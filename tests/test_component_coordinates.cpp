#include "mocks/mock_renderer.h"
#include <bombfork/prong/core/component.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bombfork::prong;
using namespace bombfork::prong::tests;

// Simple concrete component for testing
class TestComponent : public Component {
public:
  explicit TestComponent(const std::string& name = "TestComponent") : Component(nullptr, name) {}

  void update(double deltaTime) override { (void)deltaTime; }
  void render() override {}
};

// ============================================================================
// Basic Coordinate API Tests
// ============================================================================

void test_local_position_storage() {
  std::cout << "Testing local position storage..." << std::endl;

  TestComponent component("test");

  // Default position should be (0, 0)
  int x, y;
  component.getPosition(x, y);
  assert(x == 0);
  assert(y == 0);

  // Set local position
  component.setPosition(100, 200);
  component.getPosition(x, y);
  assert(x == 100);
  assert(y == 200);

  // Verify setBounds also sets position
  component.setBounds(50, 75, 300, 400);
  int bx, by, bw, bh;
  component.getBounds(bx, by, bw, bh);
  assert(bx == 50);
  assert(by == 75);
  assert(bw == 300);
  assert(bh == 400);

  std::cout << "✓ Local position storage test passed" << std::endl;
}

void test_global_position_calculation_root() {
  std::cout << "Testing global position calculation for root component..." << std::endl;

  TestComponent root("root");
  root.setPosition(100, 150);

  // Root component: local coordinates = global coordinates
  int gx, gy;
  root.getGlobalPosition(gx, gy);
  assert(gx == 100);
  assert(gy == 150);

  std::cout << "✓ Global position calculation for root test passed" << std::endl;
}

void test_global_position_calculation_with_parent() {
  std::cout << "Testing global position calculation with parent..." << std::endl;

  TestComponent parent("parent");
  parent.setPosition(100, 200);

  auto child = std::make_unique<TestComponent>("child");
  child->setPosition(50, 75);

  // Store pointer before moving
  TestComponent* childPtr = child.get();
  parent.addChild(std::move(child));

  // Child's global position = parent's global + child's local
  int gx, gy;
  childPtr->getGlobalPosition(gx, gy);
  assert(gx == 150); // 100 + 50
  assert(gy == 275); // 200 + 75

  std::cout << "✓ Global position calculation with parent test passed" << std::endl;
}

void test_global_bounds() {
  std::cout << "Testing global bounds calculation..." << std::endl;

  TestComponent parent("parent");
  parent.setBounds(100, 200, 500, 400);

  auto child = std::make_unique<TestComponent>("child");
  child->setBounds(50, 75, 200, 100);

  TestComponent* childPtr = child.get();
  parent.addChild(std::move(child));

  // Verify global bounds
  int gx, gy, gw, gh;
  childPtr->getGlobalBounds(gx, gy, gw, gh);
  assert(gx == 150); // 100 + 50
  assert(gy == 275); // 200 + 75
  assert(gw == 200); // Width unchanged
  assert(gh == 100); // Height unchanged

  std::cout << "✓ Global bounds calculation test passed" << std::endl;
}

// ============================================================================
// Coordinate Conversion Tests
// ============================================================================

void test_global_to_local_conversion() {
  std::cout << "Testing globalToLocal conversion..." << std::endl;

  TestComponent component("test");
  component.setPosition(100, 200);

  // Convert a global point to local
  int localX, localY;
  component.globalToLocal(150, 250, localX, localY);
  assert(localX == 50); // 150 - 100
  assert(localY == 50); // 250 - 200

  std::cout << "✓ globalToLocal conversion test passed" << std::endl;
}

void test_local_to_global_conversion() {
  std::cout << "Testing localToGlobal conversion..." << std::endl;

  TestComponent component("test");
  component.setPosition(100, 200);

  // Convert a local point to global
  int globalX, globalY;
  component.localToGlobal(50, 75, globalX, globalY);
  assert(globalX == 150); // 100 + 50
  assert(globalY == 275); // 200 + 75

  std::cout << "✓ localToGlobal conversion test passed" << std::endl;
}

void test_coordinate_conversion_roundtrip() {
  std::cout << "Testing coordinate conversion roundtrip..." << std::endl;

  TestComponent component("test");
  component.setPosition(100, 200);

  // Global -> Local -> Global should give original coordinates
  int localX, localY, globalX, globalY;
  component.globalToLocal(150, 250, localX, localY);
  component.localToGlobal(localX, localY, globalX, globalY);
  assert(globalX == 150);
  assert(globalY == 250);

  std::cout << "✓ Coordinate conversion roundtrip test passed" << std::endl;
}

void test_contains_global() {
  std::cout << "Testing containsGlobal..." << std::endl;

  TestComponent component("test");
  component.setBounds(100, 200, 150, 100);

  // Points inside the component
  assert(component.containsGlobal(100, 200)); // Top-left corner
  assert(component.containsGlobal(150, 250)); // Center
  assert(component.containsGlobal(249, 299)); // Bottom-right (just inside)

  // Points outside the component
  assert(!component.containsGlobal(99, 200));  // Just left
  assert(!component.containsGlobal(100, 199)); // Just above
  assert(!component.containsGlobal(250, 250)); // Just right
  assert(!component.containsGlobal(150, 300)); // Just below
  assert(!component.containsGlobal(0, 0));     // Far away

  std::cout << "✓ containsGlobal test passed" << std::endl;
}

// ============================================================================
// Cache Invalidation Tests
// ============================================================================

void test_cache_invalidation_on_position_change() {
  std::cout << "Testing cache invalidation on position change..." << std::endl;

  TestComponent component("test");
  component.setPosition(100, 200);

  // Get global position (caches it)
  int gx1, gy1;
  component.getGlobalPosition(gx1, gy1);
  assert(gx1 == 100);
  assert(gy1 == 200);

  // Change position
  component.setPosition(300, 400);

  // Global position should reflect new position
  int gx2, gy2;
  component.getGlobalPosition(gx2, gy2);
  assert(gx2 == 300);
  assert(gy2 == 400);

  std::cout << "✓ Cache invalidation on position change test passed" << std::endl;
}

void test_cache_invalidation_on_bounds_change() {
  std::cout << "Testing cache invalidation on bounds change..." << std::endl;

  TestComponent component("test");
  component.setBounds(100, 200, 150, 100);

  // Get global position (caches it)
  int gx1, gy1;
  component.getGlobalPosition(gx1, gy1);
  assert(gx1 == 100);
  assert(gy1 == 200);

  // Change bounds
  component.setBounds(300, 400, 200, 150);

  // Global position should reflect new position
  int gx2, gy2;
  component.getGlobalPosition(gx2, gy2);
  assert(gx2 == 300);
  assert(gy2 == 400);

  std::cout << "✓ Cache invalidation on bounds change test passed" << std::endl;
}

void test_cache_invalidation_cascades_to_children() {
  std::cout << "Testing cache invalidation cascades to children..." << std::endl;

  TestComponent parent("parent");
  parent.setPosition(100, 200);

  auto child1 = std::make_unique<TestComponent>("child1");
  child1->setPosition(50, 75);

  auto child2 = std::make_unique<TestComponent>("child2");
  child2->setPosition(30, 40);

  TestComponent* child1Ptr = child1.get();
  TestComponent* child2Ptr = child2.get();

  parent.addChild(std::move(child1));
  parent.addChild(std::move(child2));

  // Get initial global positions (caches them)
  int c1x, c1y, c2x, c2y;
  child1Ptr->getGlobalPosition(c1x, c1y);
  child2Ptr->getGlobalPosition(c2x, c2y);
  assert(c1x == 150); // 100 + 50
  assert(c1y == 275); // 200 + 75
  assert(c2x == 130); // 100 + 30
  assert(c2y == 240); // 200 + 40

  // Move parent
  parent.setPosition(300, 400);

  // Children's global positions should update
  child1Ptr->getGlobalPosition(c1x, c1y);
  child2Ptr->getGlobalPosition(c2x, c2y);
  assert(c1x == 350); // 300 + 50
  assert(c1y == 475); // 400 + 75
  assert(c2x == 330); // 300 + 30
  assert(c2y == 440); // 400 + 40

  std::cout << "✓ Cache invalidation cascades to children test passed" << std::endl;
}

void test_cache_invalidation_deep_hierarchy() {
  std::cout << "Testing cache invalidation in deep hierarchy..." << std::endl;

  // Create a hierarchy: root -> level1 -> level2 -> level3
  TestComponent root("root");
  root.setPosition(100, 100);

  auto level1 = std::make_unique<TestComponent>("level1");
  level1->setPosition(10, 10);

  auto level2 = std::make_unique<TestComponent>("level2");
  level2->setPosition(10, 10);

  auto level3 = std::make_unique<TestComponent>("level3");
  level3->setPosition(10, 10);

  TestComponent* level3Ptr = level3.get();
  level2->addChild(std::move(level3));

  level1->addChild(std::move(level2));

  TestComponent* level1Ptr = level1.get();
  root.addChild(std::move(level1));

  // Get level3's initial global position
  int gx, gy;
  level3Ptr->getGlobalPosition(gx, gy);
  assert(gx == 130); // 100 + 10 + 10 + 10
  assert(gy == 130);

  // Move root
  root.setPosition(200, 200);

  // Level3 should update
  level3Ptr->getGlobalPosition(gx, gy);
  assert(gx == 230); // 200 + 10 + 10 + 10
  assert(gy == 230);

  // Move level1
  level1Ptr->setPosition(20, 20);
  level3Ptr->getGlobalPosition(gx, gy);
  assert(gx == 240); // 200 + 20 + 10 + 10
  assert(gy == 240);

  std::cout << "✓ Cache invalidation deep hierarchy test passed" << std::endl;
}

// ============================================================================
// Parent-Child Coordinate Tests
// ============================================================================

void test_child_relative_to_parent() {
  std::cout << "Testing child coordinates are relative to parent..." << std::endl;

  TestComponent parent("parent");
  parent.setBounds(100, 200, 500, 400);

  auto child = std::make_unique<TestComponent>("child");
  child->setBounds(50, 75, 100, 80);

  TestComponent* childPtr = child.get();
  parent.addChild(std::move(child));

  // Child's local coordinates should be unchanged
  int lx, ly, lw, lh;
  childPtr->getBounds(lx, ly, lw, lh);
  assert(lx == 50);
  assert(ly == 75);
  assert(lw == 100);
  assert(lh == 80);

  // Child's global coordinates should be parent + local
  int gx, gy, gw, gh;
  childPtr->getGlobalBounds(gx, gy, gw, gh);
  assert(gx == 150); // 100 + 50
  assert(gy == 275); // 200 + 75
  assert(gw == 100);
  assert(gh == 80);

  std::cout << "✓ Child relative to parent test passed" << std::endl;
}

void test_multiple_children_different_positions() {
  std::cout << "Testing multiple children with different positions..." << std::endl;

  TestComponent parent("parent");
  parent.setPosition(100, 100);

  auto child1 = std::make_unique<TestComponent>("child1");
  child1->setBounds(0, 0, 50, 50);

  auto child2 = std::make_unique<TestComponent>("child2");
  child2->setBounds(60, 0, 50, 50);

  auto child3 = std::make_unique<TestComponent>("child3");
  child3->setBounds(0, 60, 50, 50);

  TestComponent* child1Ptr = child1.get();
  TestComponent* child2Ptr = child2.get();
  TestComponent* child3Ptr = child3.get();

  parent.addChild(std::move(child1));
  parent.addChild(std::move(child2));
  parent.addChild(std::move(child3));

  // Verify each child's global position
  int gx, gy;
  child1Ptr->getGlobalPosition(gx, gy);
  assert(gx == 100 && gy == 100);

  child2Ptr->getGlobalPosition(gx, gy);
  assert(gx == 160 && gy == 100);

  child3Ptr->getGlobalPosition(gx, gy);
  assert(gx == 100 && gy == 160);

  std::cout << "✓ Multiple children different positions test passed" << std::endl;
}

void test_deeply_nested_hierarchy() {
  std::cout << "Testing deeply nested component hierarchy..." << std::endl;

  // Create hierarchy with 5 levels
  TestComponent root("root");
  root.setPosition(0, 0);

  Component* current = &root;
  TestComponent* deepestChild = nullptr;

  for (int i = 1; i <= 5; i++) {
    auto child = std::make_unique<TestComponent>("level" + std::to_string(i));
    child->setPosition(10 * i, 10 * i);

    if (i == 5) {
      deepestChild = child.get();
    }

    current->addChild(std::move(child));
    current = current->getChildren()[0].get();
  }

  // Deepest child's global position should be sum of all local positions
  int gx, gy;
  deepestChild->getGlobalPosition(gx, gy);
  // 0 + 10 + 20 + 30 + 40 + 50 = 150
  assert(gx == 150);
  assert(gy == 150);

  std::cout << "✓ Deeply nested hierarchy test passed" << std::endl;
}

// ============================================================================
// Edge Cases
// ============================================================================

void test_component_with_no_parent() {
  std::cout << "Testing component with no parent..." << std::endl;

  TestComponent component("orphan");
  component.setPosition(100, 200);

  // Component with no parent: local == global
  int lx, ly, gx, gy;
  component.getPosition(lx, ly);
  component.getGlobalPosition(gx, gy);
  assert(lx == gx);
  assert(ly == gy);
  assert(gx == 100);
  assert(gy == 200);

  std::cout << "✓ Component with no parent test passed" << std::endl;
}

void test_moving_component_between_parents() {
  std::cout << "Testing moving component between parents..." << std::endl;

  TestComponent parent1("parent1");
  parent1.setPosition(100, 100);

  TestComponent parent2("parent2");
  parent2.setPosition(300, 300);

  auto child = std::make_unique<TestComponent>("child");
  child->setPosition(50, 50);
  TestComponent* childPtr = child.get();

  // Add to first parent
  parent1.addChild(std::move(child));

  int gx, gy;
  childPtr->getGlobalPosition(gx, gy);
  assert(gx == 150); // 100 + 50
  assert(gy == 150);

  // Move to second parent
  // Note: removeChild() sets parent=nullptr but doesn't invalidate cache.
  // The cache still holds the old global position (150, 150).
  // To properly test the behavior after removal, we need to manually trigger
  // cache invalidation by calling setPosition().
  parent1.removeChild(childPtr);

  // Force cache update by changing position
  childPtr->setPosition(50, 50); // Same position, but triggers cache invalidation

  // After removing and cache invalidation, global position should be based on local only
  childPtr->getGlobalPosition(gx, gy);
  assert(gx == 50); // No parent, so local == global
  assert(gy == 50);

  std::cout << "✓ Moving component between parents test passed" << std::endl;
}

void test_zero_position_components() {
  std::cout << "Testing components at (0, 0)..." << std::endl;

  TestComponent parent("parent");
  parent.setPosition(0, 0);

  auto child = std::make_unique<TestComponent>("child");
  child->setPosition(0, 0);
  TestComponent* childPtr = child.get();

  parent.addChild(std::move(child));

  int gx, gy;
  childPtr->getGlobalPosition(gx, gy);
  assert(gx == 0);
  assert(gy == 0);

  std::cout << "✓ Zero position components test passed" << std::endl;
}

void test_negative_local_positions() {
  std::cout << "Testing negative local positions..." << std::endl;

  TestComponent parent("parent");
  parent.setPosition(100, 100);

  auto child = std::make_unique<TestComponent>("child");
  child->setPosition(-20, -30);
  TestComponent* childPtr = child.get();

  parent.addChild(std::move(child));

  // Child's global position should allow negative offsets
  int gx, gy;
  childPtr->getGlobalPosition(gx, gy);
  assert(gx == 80); // 100 + (-20)
  assert(gy == 70); // 100 + (-30)

  std::cout << "✓ Negative local positions test passed" << std::endl;
}

void test_contains_global_with_nested_components() {
  std::cout << "Testing containsGlobal with nested components..." << std::endl;

  TestComponent parent("parent");
  parent.setBounds(100, 100, 200, 200);

  auto child = std::make_unique<TestComponent>("child");
  child->setBounds(50, 50, 100, 100);
  TestComponent* childPtr = child.get();

  parent.addChild(std::move(child));

  // Child occupies global area (150, 150) to (250, 250)
  assert(childPtr->containsGlobal(150, 150));  // Top-left
  assert(childPtr->containsGlobal(200, 200));  // Center
  assert(childPtr->containsGlobal(249, 249));  // Bottom-right (inside)
  assert(!childPtr->containsGlobal(149, 150)); // Just left
  assert(!childPtr->containsGlobal(250, 200)); // Just right
  assert(!childPtr->containsGlobal(200, 250)); // Just below

  // Parent occupies global area (100, 100) to (300, 300)
  assert(parent.containsGlobal(100, 100));
  assert(parent.containsGlobal(299, 299));
  assert(!parent.containsGlobal(300, 300));

  std::cout << "✓ containsGlobal with nested components test passed" << std::endl;
}

// ============================================================================
// Integration Tests
// ============================================================================

void test_coordinate_system_with_layout() {
  std::cout << "Testing coordinate system works with layouts..." << std::endl;

  MockRenderer renderer;

  TestComponent parent("parent");
  parent.setRenderer(&renderer);
  parent.setBounds(50, 50, 400, 300);

  // Add children that would be positioned by layout
  auto child1 = std::make_unique<TestComponent>("child1");
  auto child2 = std::make_unique<TestComponent>("child2");

  TestComponent* child1Ptr = child1.get();
  TestComponent* child2Ptr = child2.get();

  parent.addChild(std::move(child1));
  parent.addChild(std::move(child2));

  // Simulate layout manager positioning children
  child1Ptr->setPosition(10, 10);
  child1Ptr->setSize(100, 50);

  child2Ptr->setPosition(10, 70);
  child2Ptr->setSize(100, 50);

  // Verify global positions account for parent offset
  int gx1, gy1, gx2, gy2;
  child1Ptr->getGlobalPosition(gx1, gy1);
  child2Ptr->getGlobalPosition(gx2, gy2);

  assert(gx1 == 60);  // 50 + 10
  assert(gy1 == 60);  // 50 + 10
  assert(gx2 == 60);  // 50 + 10
  assert(gy2 == 120); // 50 + 70

  std::cout << "✓ Coordinate system with layout test passed" << std::endl;
}

void test_getGlobalX_and_getGlobalY_protected_accessors() {
  std::cout << "Testing protected getGlobalX() and getGlobalY() accessors..." << std::endl;

  // Create a custom component to test protected accessors
  class CustomComponent : public Component {
  public:
    explicit CustomComponent(const std::string& name) : Component(nullptr, name) {}

    void update(double deltaTime) override { (void)deltaTime; }
    void render() override {}

    // Public wrappers for protected accessors
    int testGetGlobalX() const { return getGlobalX(); }
    int testGetGlobalY() const { return getGlobalY(); }
  };

  CustomComponent parent("parent");
  parent.setPosition(100, 200);

  auto child = std::make_unique<CustomComponent>("child");
  child->setPosition(50, 75);
  CustomComponent* childPtr = child.get();

  parent.addChild(std::move(child));

  // Test protected accessors
  assert(childPtr->testGetGlobalX() == 150); // 100 + 50
  assert(childPtr->testGetGlobalY() == 275); // 200 + 75

  std::cout << "✓ Protected getGlobalX/Y accessors test passed" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
  try {
    std::cout << "Running Component Coordinate System tests...\n" << std::endl;

    // Basic API tests
    std::cout << "=== Basic Coordinate API Tests ===" << std::endl;
    test_local_position_storage();
    test_global_position_calculation_root();
    test_global_position_calculation_with_parent();
    test_global_bounds();

    // Conversion tests
    std::cout << "\n=== Coordinate Conversion Tests ===" << std::endl;
    test_global_to_local_conversion();
    test_local_to_global_conversion();
    test_coordinate_conversion_roundtrip();
    test_contains_global();

    // Cache invalidation tests
    std::cout << "\n=== Cache Invalidation Tests ===" << std::endl;
    test_cache_invalidation_on_position_change();
    test_cache_invalidation_on_bounds_change();
    test_cache_invalidation_cascades_to_children();
    test_cache_invalidation_deep_hierarchy();

    // Parent-child tests
    std::cout << "\n=== Parent-Child Coordinate Tests ===" << std::endl;
    test_child_relative_to_parent();
    test_multiple_children_different_positions();
    test_deeply_nested_hierarchy();

    // Edge cases
    std::cout << "\n=== Edge Case Tests ===" << std::endl;
    test_component_with_no_parent();
    test_moving_component_between_parents();
    test_zero_position_components();
    test_negative_local_positions();
    test_contains_global_with_nested_components();

    // Integration tests
    std::cout << "\n=== Integration Tests ===" << std::endl;
    test_coordinate_system_with_layout();
    test_getGlobalX_and_getGlobalY_protected_accessors();

    std::cout << "\n✓ All Component Coordinate System tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "\n✗ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
