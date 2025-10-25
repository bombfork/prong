#include "mocks/mock_renderer.h"
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/layout/flow_layout.h>
#include <bombfork/prong/layout/grid_layout.h>
#include <bombfork/prong/layout/stack_layout.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bombfork::prong;
using namespace bombfork::prong::tests;

// Test component with configurable minimum size
class TestComponent : public Component {
private:
  int minWidth = 0;
  int minHeight = 0;

public:
  explicit TestComponent(const std::string& name = "TestComponent", int minW = 0, int minH = 0)
    : Component(nullptr, name), minWidth(minW), minHeight(minH) {}

  void setMinimumSize(int w, int h) {
    minWidth = w;
    minHeight = h;
  }

  int getMinimumWidth() const override { return minWidth; }

  int getMinimumHeight() const override { return minHeight; }

  void update(double deltaTime) override { (void)deltaTime; }

  void render() override {}
};

// ============================================================================
// GridLayout Minimum Size Tests
// ============================================================================

void test_grid_respects_minimum_width() {
  std::cout << "Testing GridLayout respects minimum width..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(200, 200);

  auto layout = std::make_shared<layout::GridLayout>();
  layout->configure({.columns = 2, .rows = 2}); // 2x2 grid
  panel.setLayout(layout);

  // Add components with minimum sizes
  auto child1 = std::make_unique<TestComponent>("Child1", 80, 50);
  child1->setSize(0, 0);
  auto child2 = std::make_unique<TestComponent>("Child2", 60, 50);
  child2->setSize(0, 0);
  auto child3 = std::make_unique<TestComponent>("Child3", 70, 50);
  child3->setSize(0, 0);
  auto child4 = std::make_unique<TestComponent>("Child4", 90, 50);
  child4->setSize(0, 0);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));
  panel.addChild(std::move(child4));

  panel.performLayout();

  // Each column should be at least as wide as its widest minimum
  // Column 1: max(80, 70) = 80
  // Column 2: max(60, 90) = 90
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();
  auto* layoutChild3 = panel.getChildren()[2].get();
  auto* layoutChild4 = panel.getChildren()[3].get();

  int w1, h1, w2, h2, w3, h3, w4, h4;
  layoutChild1->getSize(w1, h1);
  layoutChild2->getSize(w2, h2);
  layoutChild3->getSize(w3, h3);
  layoutChild4->getSize(w4, h4);

  assert(w1 >= 80);
  assert(w2 >= 60);
  assert(w3 >= 70);
  assert(w4 >= 90);

  std::cout << "✓ GridLayout respects minimum width test passed" << std::endl;
}

void test_grid_respects_minimum_height() {
  std::cout << "Testing GridLayout respects minimum height..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(200, 200);

  auto layout = std::make_shared<layout::GridLayout>();
  layout->configure({.columns = 2, .rows = 2}); // 2x2 grid
  panel.setLayout(layout);

  // Add components with different minimum heights
  auto child1 = std::make_unique<TestComponent>("Child1", 50, 60);
  child1->setSize(0, 0);
  auto child2 = std::make_unique<TestComponent>("Child2", 50, 80);
  child2->setSize(0, 0);
  auto child3 = std::make_unique<TestComponent>("Child3", 50, 70);
  child3->setSize(0, 0);
  auto child4 = std::make_unique<TestComponent>("Child4", 50, 90);
  child4->setSize(0, 0);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));
  panel.addChild(std::move(child4));

  panel.performLayout();

  // Each row should be at least as tall as its tallest minimum
  // Row 1: max(60, 80) = 80
  // Row 2: max(70, 90) = 90
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();
  auto* layoutChild3 = panel.getChildren()[2].get();
  auto* layoutChild4 = panel.getChildren()[3].get();

  int w1, h1, w2, h2, w3, h3, w4, h4;
  layoutChild1->getSize(w1, h1);
  layoutChild2->getSize(w2, h2);
  layoutChild3->getSize(w3, h3);
  layoutChild4->getSize(w4, h4);

  assert(h1 >= 60);
  assert(h2 >= 80);
  assert(h3 >= 70);
  assert(h4 >= 90);

  std::cout << "✓ GridLayout respects minimum height test passed" << std::endl;
}

void test_grid_with_gap() {
  std::cout << "Testing GridLayout with gap..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(400, 400);

  auto layout = std::make_shared<layout::GridLayout>();
  layout->configure(
    {.columns = 2, .rows = 2, .horizontalSpacing = 10.0f, .verticalSpacing = 10.0f}); // 2x2 grid with 10px gap
  panel.setLayout(layout);

  // Add four equal components
  for (int i = 0; i < 4; i++) {
    auto child = std::make_unique<TestComponent>("Child" + std::to_string(i), 50, 50);
    child->setSize(0, 0);
    panel.addChild(std::move(child));
  }

  panel.performLayout();

  // With 400x400 space and 10px gap:
  // Available width per column: (400 - 10) / 2 = 195
  // Available height per row: (400 - 10) / 2 = 195
  auto* layoutChild1 = panel.getChildren()[0].get();

  int w, h;
  layoutChild1->getSize(w, h);

  assert(w >= 50); // At least minimum
  assert(h >= 50); // At least minimum

  std::cout << "✓ GridLayout with gap test passed" << std::endl;
}

void test_grid_fixed_size_components() {
  std::cout << "Testing GridLayout with fixed size components..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(400, 400);

  auto layout = std::make_shared<layout::GridLayout>();
  layout->configure({.columns = 2, .rows = 2}); // 2x2 grid
  panel.setLayout(layout);

  // Add components with explicit sizes
  auto child1 = std::make_unique<TestComponent>("Child1", 50, 50);
  child1->setSize(100, 80);
  auto child2 = std::make_unique<TestComponent>("Child2", 50, 50);
  child2->setSize(120, 80);
  auto child3 = std::make_unique<TestComponent>("Child3", 50, 50);
  child3->setSize(100, 90);
  auto child4 = std::make_unique<TestComponent>("Child4", 50, 50);
  child4->setSize(120, 90);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));
  panel.addChild(std::move(child4));

  panel.performLayout();

  // Fixed sizes should be maintained or distributed based on available space
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();

  int w1, h1, w2, h2;
  layoutChild1->getSize(w1, h1);
  layoutChild2->getSize(w2, h2);

  // Should maintain their sizes or get proportional space
  assert(w1 > 0);
  assert(w2 > 0);

  std::cout << "✓ GridLayout with fixed size components test passed" << std::endl;
}

// ============================================================================
// StackLayout Minimum Size Tests
// ============================================================================

void test_stack_horizontal_respects_minimums() {
  std::cout << "Testing StackLayout horizontal respects minimums..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(400, 200);

  auto layout = std::make_shared<layout::StackLayout>();
  layout->configure({.orientation = layout::StackOrientation::HORIZONTAL});
  panel.setLayout(layout);

  // Add components with different minimum widths
  auto child1 = std::make_unique<TestComponent>("Child1", 100, 50);
  child1->setSize(0, 0);
  auto child2 = std::make_unique<TestComponent>("Child2", 80, 50);
  child2->setSize(0, 0);
  auto child3 = std::make_unique<TestComponent>("Child3", 120, 50);
  child3->setSize(0, 0);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));

  panel.performLayout();

  // Each component should be at least its minimum width
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();
  auto* layoutChild3 = panel.getChildren()[2].get();

  int w1, h1, w2, h2, w3, h3;
  layoutChild1->getSize(w1, h1);
  layoutChild2->getSize(w2, h2);
  layoutChild3->getSize(w3, h3);

  assert(w1 >= 100);
  assert(w2 >= 80);
  assert(w3 >= 120);

  std::cout << "✓ StackLayout horizontal respects minimums test passed" << std::endl;
}

void test_stack_vertical_respects_minimums() {
  std::cout << "Testing StackLayout vertical respects minimums..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(200, 400);

  auto layout = std::make_shared<layout::StackLayout>();
  layout->configure({.orientation = layout::StackOrientation::VERTICAL});
  panel.setLayout(layout);

  // Add components with different minimum heights
  auto child1 = std::make_unique<TestComponent>("Child1", 50, 80);
  child1->setSize(0, 0);
  auto child2 = std::make_unique<TestComponent>("Child2", 50, 100);
  child2->setSize(0, 0);
  auto child3 = std::make_unique<TestComponent>("Child3", 50, 90);
  child3->setSize(0, 0);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));

  panel.performLayout();

  // Each component should be at least its minimum height
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();
  auto* layoutChild3 = panel.getChildren()[2].get();

  int w1, h1, w2, h2, w3, h3;
  layoutChild1->getSize(w1, h1);
  layoutChild2->getSize(w2, h2);
  layoutChild3->getSize(w3, h3);

  assert(h1 >= 80);
  assert(h2 >= 100);
  assert(h3 >= 90);

  std::cout << "✓ StackLayout vertical respects minimums test passed" << std::endl;
}

void test_stack_with_gap() {
  std::cout << "Testing StackLayout with gap..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(400, 200);

  auto layout = std::make_shared<layout::StackLayout>();
  layout->configure({.orientation = layout::StackOrientation::HORIZONTAL, .spacing = 15.0f}); // 15px gap
  panel.setLayout(layout);

  // Add three components
  for (int i = 0; i < 3; i++) {
    auto child = std::make_unique<TestComponent>("Child" + std::to_string(i), 50, 50);
    child->setSize(0, 0);
    panel.addChild(std::move(child));
  }

  panel.performLayout();

  // With 400px width and 15px gap between 3 items = 30px total gap
  // Available width: (400 - 30) / 3 = ~123px per item
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();

  int w1, h1;
  layoutChild1->getSize(w1, h1);
  int x1, y1, x2, y2;
  layoutChild1->getPosition(x1, y1);
  layoutChild2->getPosition(x2, y2);

  // Check that gaps are applied (second child should start after first + gap)
  int expectedGap = x2 - (x1 + w1);
  assert(expectedGap == 15);

  std::cout << "✓ StackLayout with gap test passed" << std::endl;
}

void test_stack_insufficient_space() {
  std::cout << "Testing StackLayout with insufficient space..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(100, 200); // Small width

  auto layout = std::make_shared<layout::StackLayout>();
  layout->configure({.orientation = layout::StackOrientation::HORIZONTAL});
  panel.setLayout(layout);

  // Add components with large minimums
  auto child1 = std::make_unique<TestComponent>("Child1", 80, 50);
  child1->setSize(0, 0);
  auto child2 = std::make_unique<TestComponent>("Child2", 80, 50);
  child2->setSize(0, 0);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));

  panel.performLayout();

  // Components should still respect minimums (will overflow)
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();

  int w1, h1, w2, h2;
  layoutChild1->getSize(w1, h1);
  layoutChild2->getSize(w2, h2);

  assert(w1 >= 80);
  assert(w2 >= 80);

  std::cout << "✓ StackLayout with insufficient space test passed" << std::endl;
}

// ============================================================================
// FlowLayout Minimum Size Tests
// ============================================================================

void test_flow_respects_minimum_sizes() {
  std::cout << "Testing FlowLayout respects minimum sizes..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(400, 300);

  auto layout = std::make_shared<layout::FlowLayout>();
  panel.setLayout(layout);

  // Add components with different minimum sizes
  auto child1 = std::make_unique<TestComponent>("Child1", 100, 60);
  child1->setSize(0, 0);
  auto child2 = std::make_unique<TestComponent>("Child2", 80, 50);
  child2->setSize(0, 0);
  auto child3 = std::make_unique<TestComponent>("Child3", 120, 70);
  child3->setSize(0, 0);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));

  panel.performLayout();

  // Each component should be at least its minimum size
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();
  auto* layoutChild3 = panel.getChildren()[2].get();

  int w1, h1, w2, h2, w3, h3;
  layoutChild1->getSize(w1, h1);
  layoutChild2->getSize(w2, h2);
  layoutChild3->getSize(w3, h3);

  assert(w1 >= 100);
  assert(h1 >= 60);
  assert(w2 >= 80);
  assert(h2 >= 50);
  assert(w3 >= 120);
  assert(h3 >= 70);

  std::cout << "✓ FlowLayout respects minimum sizes test passed" << std::endl;
}

void test_flow_wrapping() {
  std::cout << "Testing FlowLayout wrapping behavior..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(250, 300); // Width forces wrapping

  auto layout = std::make_shared<layout::FlowLayout>();
  layout->configure({.horizontal = true, .maxItemsPerLine = 2}); // Horizontal flow, max 2 items per row
  panel.setLayout(layout);

  // Add components that should wrap
  auto child1 = std::make_unique<TestComponent>("Child1", 100, 50);
  child1->setSize(100, 50);
  auto child2 = std::make_unique<TestComponent>("Child2", 100, 50);
  child2->setSize(100, 50);
  auto child3 = std::make_unique<TestComponent>("Child3", 100, 50);
  child3->setSize(100, 50);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));

  panel.performLayout();

  // First two should be on same row, third should wrap
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();
  auto* layoutChild3 = panel.getChildren()[2].get();

  int x1, y1, x2, y2, x3, y3;
  layoutChild1->getPosition(x1, y1);
  layoutChild2->getPosition(x2, y2);
  layoutChild3->getPosition(x3, y3);

  // Child1 and Child2 should be on same row (same y)
  assert(y1 == y2);

  // Child3 should be on next row (different y)
  assert(y3 > y1);

  std::cout << "✓ FlowLayout wrapping behavior test passed" << std::endl;
}

void test_flow_with_gap() {
  std::cout << "Testing FlowLayout with gap..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(400, 300);

  auto layout = std::make_shared<layout::FlowLayout>();
  layout->configure({.spacing = 10.0f, .crossSpacing = 10.0f}); // 10px gap
  panel.setLayout(layout);

  // Add three components
  for (int i = 0; i < 3; i++) {
    auto child = std::make_unique<TestComponent>("Child" + std::to_string(i), 80, 50);
    child->setSize(80, 50);
    panel.addChild(std::move(child));
  }

  panel.performLayout();

  // Check that gaps are applied between components on same row
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();

  int w1, h1;
  layoutChild1->getSize(w1, h1);
  int x1, y1, x2, y2;
  layoutChild1->getPosition(x1, y1);
  layoutChild2->getPosition(x2, y2);

  int horizontalGap = x2 - (x1 + w1);
  assert(horizontalGap == 10);

  std::cout << "✓ FlowLayout with gap test passed" << std::endl;
}

void test_flow_vertical_direction() {
  std::cout << "Testing FlowLayout with row wrapping..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(300, 250); // Height forces wrapping

  auto layout = std::make_shared<layout::FlowLayout>();
  // FlowLayout with horizontal=false flows items horizontally into rows (wraps vertically)
  // With maxItemsPerLine=2, we get 2 items per row, then wrap to next row
  layout->configure({.horizontal = false, .maxItemsPerLine = 2});
  panel.setLayout(layout);

  // Add components
  auto child1 = std::make_unique<TestComponent>("Child1", 80, 100);
  child1->setSize(80, 100);
  auto child2 = std::make_unique<TestComponent>("Child2", 80, 100);
  child2->setSize(80, 100);
  auto child3 = std::make_unique<TestComponent>("Child3", 80, 100);
  child3->setSize(80, 100);

  panel.addChild(std::move(child1));
  panel.addChild(std::move(child2));
  panel.addChild(std::move(child3));

  panel.performLayout();

  // With horizontal flow and maxItemsPerLine=2:
  // First two are on same row (different x, same y), third wraps to next row
  auto* layoutChild1 = panel.getChildren()[0].get();
  auto* layoutChild2 = panel.getChildren()[1].get();
  auto* layoutChild3 = panel.getChildren()[2].get();

  int x1, y1, x2, y2, x3, y3;
  layoutChild1->getPosition(x1, y1);
  layoutChild2->getPosition(x2, y2);
  layoutChild3->getPosition(x3, y3);

  // Child1 and Child2 should be on same row (same y, different x)
  assert(y1 == y2);
  assert(x2 > x1);

  // Child3 should be on next row (different y)
  assert(y3 > y1);

  std::cout << "✓ FlowLayout with row wrapping test passed" << std::endl;
}

void test_flow_empty_layout() {
  std::cout << "Testing FlowLayout with no children..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);
  panel.setSize(400, 300);

  auto layout = std::make_shared<layout::FlowLayout>();
  panel.setLayout(layout);

  // No children added
  panel.performLayout(); // Should not crash

  std::cout << "✓ FlowLayout with no children test passed" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
  try {
    std::cout << "Running Layout Minimum Size tests...\n" << std::endl;

    // GridLayout tests
    std::cout << "=== GridLayout Tests ===" << std::endl;
    test_grid_respects_minimum_width();
    test_grid_respects_minimum_height();
    test_grid_with_gap();
    test_grid_fixed_size_components();

    // StackLayout tests
    std::cout << "\n=== StackLayout Tests ===" << std::endl;
    test_stack_horizontal_respects_minimums();
    test_stack_vertical_respects_minimums();
    test_stack_with_gap();
    test_stack_insufficient_space();

    // FlowLayout tests
    std::cout << "\n=== FlowLayout Tests ===" << std::endl;
    test_flow_respects_minimum_sizes();
    test_flow_wrapping();
    test_flow_with_gap();
    test_flow_vertical_direction();
    test_flow_empty_layout();

    std::cout << "\n✓ All Layout Minimum Size tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception" << std::endl;
    return 1;
  }
}
