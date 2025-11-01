#include "mocks/mock_renderer.h"
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/event.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bombfork::prong;
using namespace bombfork::prong::core;
using namespace bombfork::prong::tests;

// ============================================================================
// Test Components
// ============================================================================

// Simple component that tracks which events it received
class EventTrackingComponent : public Component {
public:
  std::vector<Event::Type> receivedEvents;
  bool shouldHandleEvent = false;

  explicit EventTrackingComponent(const std::string& name = "EventTracking") : Component(nullptr, name) {}

  void update(double deltaTime) override { (void)deltaTime; }
  void render() override {}

  bool handleEventSelf(const Event& event) override {
    receivedEvents.push_back(event.type);
    return shouldHandleEvent;
  }

  void clearReceivedEvents() { receivedEvents.clear(); }
};

// Component that handles events at specific positions
class PositionalHandlerComponent : public Component {
public:
  int handleX = 0, handleY = 0;
  int handleRadius = 10;
  bool wasHandled = false;

  explicit PositionalHandlerComponent(const std::string& name = "PositionalHandler") : Component(nullptr, name) {}

  void update(double deltaTime) override { (void)deltaTime; }
  void render() override {}

  bool handleEventSelf(const Event& event) override {
    // Check if event is within radius of handle point
    int dx = event.localX - handleX;
    int dy = event.localY - handleY;
    if (dx * dx + dy * dy <= handleRadius * handleRadius) {
      wasHandled = true;
      return true;
    }
    return false;
  }

  void reset() { wasHandled = false; }
};

// ============================================================================
// Event Struct Tests
// ============================================================================

void test_event_struct_creation() {
  std::cout << "Testing Event struct creation..." << std::endl;

  // Test mouse press event
  Event mousePress{.type = Event::Type::MOUSE_PRESS, .localX = 10, .localY = 20, .button = 0};
  assert(mousePress.type == Event::Type::MOUSE_PRESS);
  assert(mousePress.localX == 10);
  assert(mousePress.localY == 20);
  assert(mousePress.button == 0);

  // Test mouse scroll event
  Event mouseScroll{.type = Event::Type::MOUSE_SCROLL, .localX = 5, .localY = 10, .scrollX = 1.5, .scrollY = -2.0};
  assert(mouseScroll.type == Event::Type::MOUSE_SCROLL);
  assert(mouseScroll.localX == 5);
  assert(mouseScroll.localY == 10);
  assert(mouseScroll.scrollX == 1.5);
  assert(mouseScroll.scrollY == -2.0);

  // Test key press event
  Event keyPress{.type = Event::Type::KEY_PRESS, .key = 65, .mods = 1};
  assert(keyPress.type == Event::Type::KEY_PRESS);
  assert(keyPress.key == 65);
  assert(keyPress.mods == 1);

  // Test char input event
  Event charInput{.type = Event::Type::CHAR_INPUT, .codepoint = 'A'};
  assert(charInput.type == Event::Type::CHAR_INPUT);
  assert(charInput.codepoint == 'A');

  std::cout << "✓ Event struct creation test passed" << std::endl;
}

void test_event_type_enum() {
  std::cout << "Testing Event::Type enum..." << std::endl;

  // Ensure all event types are distinct
  assert(Event::Type::MOUSE_PRESS != Event::Type::MOUSE_RELEASE);
  assert(Event::Type::MOUSE_MOVE != Event::Type::MOUSE_SCROLL);
  assert(Event::Type::KEY_PRESS != Event::Type::KEY_RELEASE);
  assert(Event::Type::KEY_PRESS != Event::Type::CHAR_INPUT);

  std::cout << "✓ Event::Type enum test passed" << std::endl;
}

// ============================================================================
// Component::isPositionalEvent Tests
// ============================================================================

void test_is_positional_event() {
  std::cout << "Testing isPositionalEvent() helper..." << std::endl;

  EventTrackingComponent component;

  // Mouse events should be positional
  assert(component.isPositionalEvent(Event::Type::MOUSE_PRESS) == true);
  assert(component.isPositionalEvent(Event::Type::MOUSE_RELEASE) == true);
  assert(component.isPositionalEvent(Event::Type::MOUSE_MOVE) == true);
  assert(component.isPositionalEvent(Event::Type::MOUSE_SCROLL) == true);

  // Keyboard events should not be positional
  assert(component.isPositionalEvent(Event::Type::KEY_PRESS) == false);
  assert(component.isPositionalEvent(Event::Type::KEY_RELEASE) == false);
  assert(component.isPositionalEvent(Event::Type::CHAR_INPUT) == false);

  std::cout << "✓ isPositionalEvent() test passed" << std::endl;
}

// ============================================================================
// Component::containsEvent Tests
// ============================================================================

void test_contains_event_default() {
  std::cout << "Testing default containsEvent() implementation..." << std::endl;

  EventTrackingComponent component;
  component.setBounds(0, 0, 100, 100);

  // Test events inside bounds
  Event inside{.type = Event::Type::MOUSE_PRESS, .localX = 50, .localY = 50};
  assert(component.containsEvent(inside) == true);

  Event topLeft{.type = Event::Type::MOUSE_PRESS, .localX = 0, .localY = 0};
  assert(component.containsEvent(topLeft) == true);

  Event bottomRight{.type = Event::Type::MOUSE_PRESS, .localX = 99, .localY = 99};
  assert(component.containsEvent(bottomRight) == true);

  // Test events outside bounds
  Event outside{.type = Event::Type::MOUSE_PRESS, .localX = 150, .localY = 50};
  assert(component.containsEvent(outside) == false);

  Event negative{.type = Event::Type::MOUSE_PRESS, .localX = -1, .localY = 50};
  assert(component.containsEvent(negative) == false);

  Event beyondWidth{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 50};
  assert(component.containsEvent(beyondWidth) == false);

  Event beyondHeight{.type = Event::Type::MOUSE_PRESS, .localX = 50, .localY = 100};
  assert(component.containsEvent(beyondHeight) == false);

  std::cout << "✓ Default containsEvent() test passed" << std::endl;
}

// ============================================================================
// Component::handleEventSelf Tests
// ============================================================================

void test_handle_event_self_default() {
  std::cout << "Testing default handleEventSelf() implementation..." << std::endl;

  EventTrackingComponent component;
  component.setBounds(0, 0, 100, 100);

  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 50, .localY = 50};

  // Default implementation should return false (not handled)
  component.shouldHandleEvent = false;
  bool handled = component.handleEventSelf(event);
  assert(handled == false);
  assert(component.receivedEvents.size() == 1);
  assert(component.receivedEvents[0] == Event::Type::MOUSE_PRESS);

  std::cout << "✓ Default handleEventSelf() test passed" << std::endl;
}

void test_handle_event_self_custom() {
  std::cout << "Testing custom handleEventSelf() implementation..." << std::endl;

  EventTrackingComponent component;
  component.setBounds(0, 0, 100, 100);
  component.shouldHandleEvent = true;

  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 50, .localY = 50};

  // Custom implementation should handle event
  bool handled = component.handleEventSelf(event);
  assert(handled == true);
  assert(component.receivedEvents.size() == 1);

  std::cout << "✓ Custom handleEventSelf() test passed" << std::endl;
}

// ============================================================================
// Component::handleEvent Propagation Tests
// ============================================================================

void test_handle_event_disabled_component() {
  std::cout << "Testing handleEvent() with disabled component..." << std::endl;

  EventTrackingComponent component;
  component.setBounds(0, 0, 100, 100);
  component.setEnabled(false);

  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 50, .localY = 50};

  bool handled = component.handleEvent(event);
  assert(handled == false);
  assert(component.receivedEvents.empty()); // Should not receive event

  std::cout << "✓ Disabled component test passed" << std::endl;
}

void test_handle_event_invisible_component() {
  std::cout << "Testing handleEvent() with invisible component..." << std::endl;

  EventTrackingComponent component;
  component.setBounds(0, 0, 100, 100);
  component.setVisible(false);

  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 50, .localY = 50};

  bool handled = component.handleEvent(event);
  assert(handled == false);
  assert(component.receivedEvents.empty()); // Should not receive event

  std::cout << "✓ Invisible component test passed" << std::endl;
}

void test_handle_event_outside_bounds() {
  std::cout << "Testing handleEvent() with event outside bounds..." << std::endl;

  EventTrackingComponent component;
  component.setBounds(0, 0, 100, 100);

  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 150, .localY = 50};

  bool handled = component.handleEvent(event);
  assert(handled == false);
  assert(component.receivedEvents.empty()); // Should not receive event

  std::cout << "✓ Outside bounds test passed" << std::endl;
}

void test_handle_event_self_handles() {
  std::cout << "Testing handleEvent() when self handles..." << std::endl;

  EventTrackingComponent parent;
  parent.setBounds(0, 0, 200, 200);
  parent.shouldHandleEvent = true;

  auto childPtr = std::make_unique<EventTrackingComponent>("child");
  childPtr->setBounds(50, 50, 100, 100);
  auto* child = childPtr.get();
  parent.addChild(std::move(childPtr));

  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};

  bool handled = parent.handleEvent(event);
  assert(handled == true);
  assert(parent.receivedEvents.size() == 1);
  // Child should not receive event because parent handled it
  assert(child->receivedEvents.empty());

  std::cout << "✓ Self handles test passed" << std::endl;
}

void test_handle_event_child_handles() {
  std::cout << "Testing handleEvent() when child handles..." << std::endl;

  EventTrackingComponent parent;
  parent.setBounds(0, 0, 200, 200);
  parent.shouldHandleEvent = false;

  auto childPtr = std::make_unique<EventTrackingComponent>("child");
  childPtr->setBounds(50, 50, 100, 100);
  childPtr->shouldHandleEvent = true;
  auto* child = childPtr.get();
  parent.addChild(std::move(childPtr));

  // Event at (100, 100) in parent space = (50, 50) in child space
  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};

  bool handled = parent.handleEvent(event);
  assert(handled == true);
  assert(parent.receivedEvents.size() == 1); // Parent saw it first
  assert(child->receivedEvents.size() == 1); // Child handled it
  assert(child->receivedEvents[0] == Event::Type::MOUSE_PRESS);

  std::cout << "✓ Child handles test passed" << std::endl;
}

void test_handle_event_coordinate_conversion() {
  std::cout << "Testing coordinate conversion during event propagation..." << std::endl;

  PositionalHandlerComponent parent("parent");
  parent.setBounds(0, 0, 200, 200);

  auto childPtr = std::make_unique<PositionalHandlerComponent>("child");
  childPtr->setBounds(50, 50, 100, 100); // Child at (50, 50) in parent space
  childPtr->handleX = 25;                // Handle at (25, 25) in child space
  childPtr->handleY = 25;
  childPtr->handleRadius = 10;
  auto* child = childPtr.get();
  parent.addChild(std::move(childPtr));

  // Event at (75, 75) in parent space = (25, 25) in child space
  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 75, .localY = 75};

  bool handled = parent.handleEvent(event);
  assert(handled == true);
  assert(child->wasHandled == true);

  // Event at (60, 60) in parent space = (10, 10) in child space (outside radius)
  child->reset();
  Event event2{.type = Event::Type::MOUSE_PRESS, .localX = 60, .localY = 60};
  handled = parent.handleEvent(event2);
  assert(handled == false);
  assert(child->wasHandled == false);

  std::cout << "✓ Coordinate conversion test passed" << std::endl;
}

void test_handle_event_z_order() {
  std::cout << "Testing z-order (reverse) event propagation..." << std::endl;

  EventTrackingComponent parent;
  parent.setBounds(0, 0, 200, 200);

  // Add two overlapping children
  auto child1Ptr = std::make_unique<EventTrackingComponent>("child1");
  child1Ptr->setBounds(50, 50, 100, 100);
  child1Ptr->shouldHandleEvent = false;
  auto* child1 = child1Ptr.get();
  parent.addChild(std::move(child1Ptr));

  auto child2Ptr = std::make_unique<EventTrackingComponent>("child2");
  child2Ptr->setBounds(50, 50, 100, 100); // Same position as child1
  child2Ptr->shouldHandleEvent = true;
  auto* child2 = child2Ptr.get();
  parent.addChild(std::move(child2Ptr));

  // Event in overlapping area
  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};

  bool handled = parent.handleEvent(event);
  assert(handled == true);
  // child2 was added last, so it should receive event first (z-order)
  assert(child2->receivedEvents.size() == 1);
  // child1 should not receive event because child2 handled it
  assert(child1->receivedEvents.empty());

  std::cout << "✓ Z-order test passed" << std::endl;
}

void test_handle_event_non_positional() {
  std::cout << "Testing non-positional event handling..." << std::endl;

  EventTrackingComponent component;
  component.setBounds(0, 0, 100, 100);

  // Key events should not require position checking
  Event keyEvent{.type = Event::Type::KEY_PRESS, .key = 65};

  bool handled = component.handleEvent(keyEvent);
  assert(handled == false); // Not handled by default
  assert(component.receivedEvents.size() == 1);
  assert(component.receivedEvents[0] == Event::Type::KEY_PRESS);

  std::cout << "✓ Non-positional event test passed" << std::endl;
}

void test_handle_event_nested_propagation() {
  std::cout << "Testing nested component event propagation..." << std::endl;

  EventTrackingComponent grandparent;
  grandparent.setBounds(0, 0, 300, 300);

  auto parentPtr = std::make_unique<EventTrackingComponent>("parent");
  parentPtr->setBounds(50, 50, 200, 200);
  auto* parent = parentPtr.get();
  grandparent.addChild(std::move(parentPtr));

  auto childPtr = std::make_unique<EventTrackingComponent>("child");
  childPtr->setBounds(25, 25, 100, 100); // At (75, 75) in grandparent space
  childPtr->shouldHandleEvent = true;
  auto* child = childPtr.get();
  parent->addChild(std::move(childPtr));

  // Event at (100, 100) in grandparent space
  // = (50, 50) in parent space
  // = (25, 25) in child space
  Event event{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};

  bool handled = grandparent.handleEvent(event);
  assert(handled == true);
  assert(grandparent.receivedEvents.size() == 1);
  assert(parent->receivedEvents.size() == 1);
  assert(child->receivedEvents.size() == 1);

  std::cout << "✓ Nested propagation test passed" << std::endl;
}

// ============================================================================
// Panel::containsEvent Tests
// ============================================================================

void test_panel_contains_event_no_border() {
  std::cout << "Testing Panel containsEvent() with no border..." << std::endl;

  Panel<> panel("testPanel");
  panel.setBounds(0, 0, 200, 200);
  panel.setBorderWidth(0);
  panel.setPadding(0);

  // Event inside panel should be contained
  Event inside{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};
  assert(panel.containsEvent(inside) == true);

  // Event at edge should be contained
  Event edge{.type = Event::Type::MOUSE_PRESS, .localX = 0, .localY = 0};
  assert(panel.containsEvent(edge) == true);

  std::cout << "✓ Panel containsEvent() no border test passed" << std::endl;
}

void test_panel_contains_event_with_border() {
  std::cout << "Testing Panel containsEvent() with border..." << std::endl;

  Panel<> panel("testPanel");
  panel.setBounds(0, 0, 200, 200);
  panel.setBorderWidth(5);
  panel.setPadding(0);

  // Event inside content area should be contained
  Event inside{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};
  assert(panel.containsEvent(inside) == true);

  // Event in border area should NOT be contained
  Event inBorder{.type = Event::Type::MOUSE_PRESS, .localX = 2, .localY = 2};
  assert(panel.containsEvent(inBorder) == false);

  // Event at content area edge should be contained
  Event contentEdge{.type = Event::Type::MOUSE_PRESS, .localX = 5, .localY = 5};
  assert(panel.containsEvent(contentEdge) == true);

  std::cout << "✓ Panel containsEvent() with border test passed" << std::endl;
}

void test_panel_contains_event_with_padding() {
  std::cout << "Testing Panel containsEvent() with padding..." << std::endl;

  Panel<> panel("testPanel");
  panel.setBounds(0, 0, 200, 200);
  panel.setBorderWidth(0);
  panel.setPadding(10);

  // Event inside content area should be contained
  Event inside{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};
  assert(panel.containsEvent(inside) == true);

  // Event in padding area should NOT be contained
  Event inPadding{.type = Event::Type::MOUSE_PRESS, .localX = 5, .localY = 5};
  assert(panel.containsEvent(inPadding) == false);

  // Event at content area edge should be contained
  Event contentEdge{.type = Event::Type::MOUSE_PRESS, .localX = 10, .localY = 10};
  assert(panel.containsEvent(contentEdge) == true);

  std::cout << "✓ Panel containsEvent() with padding test passed" << std::endl;
}

void test_panel_contains_event_with_titlebar() {
  std::cout << "Testing Panel containsEvent() with title bar..." << std::endl;

  Panel<> panel("testPanel");
  panel.setBounds(0, 0, 200, 200);
  panel.setBorderWidth(1);
  panel.setPadding(5);
  panel.setTitle("Test Panel");

  // Title bar height is 25 pixels
  // Content area starts at: border(1) + titlebar(25) + padding(5) = 31

  // Event inside content area should be contained
  Event inside{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};
  assert(panel.containsEvent(inside) == true);

  // Event in title bar should NOT be contained
  Event inTitleBar{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 15};
  assert(panel.containsEvent(inTitleBar) == false);

  // Event at content area top edge should be contained
  Event contentTop{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 31};
  assert(panel.containsEvent(contentTop) == true);

  std::cout << "✓ Panel containsEvent() with title bar test passed" << std::endl;
}

void test_panel_contains_event_complex() {
  std::cout << "Testing Panel containsEvent() with all features..." << std::endl;

  Panel<> panel("testPanel");
  panel.setBounds(0, 0, 200, 200);
  panel.setBorderWidth(5);
  panel.setPadding(10);
  panel.setTitle("Complex Panel");

  // Content area: starts at (15, 40), size (170, 145)
  // X: border(5) + padding(10) = 15
  // Y: border(5) + titlebar(25) + padding(10) = 40
  // Width: 200 - 2*(5+10) = 170
  // Height: 200 - 2*(5+10) - 25 = 145

  // Event in center of content area
  Event center{.type = Event::Type::MOUSE_PRESS, .localX = 100, .localY = 100};
  assert(panel.containsEvent(center) == true);

  // Event at content area origin
  Event origin{.type = Event::Type::MOUSE_PRESS, .localX = 15, .localY = 40};
  assert(panel.containsEvent(origin) == true);

  // Event just before content area (in padding)
  Event beforeContent{.type = Event::Type::MOUSE_PRESS, .localX = 14, .localY = 40};
  assert(panel.containsEvent(beforeContent) == false);

  // Event in border area
  Event inBorder{.type = Event::Type::MOUSE_PRESS, .localX = 2, .localY = 100};
  assert(panel.containsEvent(inBorder) == false);

  std::cout << "✓ Panel containsEvent() complex test passed" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
  std::cout << "Running Event API Tests\n" << std::endl;

  try {
    // Event struct tests
    test_event_struct_creation();
    test_event_type_enum();

    // Helper method tests
    test_is_positional_event();

    // containsEvent tests
    test_contains_event_default();

    // handleEventSelf tests
    test_handle_event_self_default();
    test_handle_event_self_custom();

    // handleEvent propagation tests
    test_handle_event_disabled_component();
    test_handle_event_invisible_component();
    test_handle_event_outside_bounds();
    test_handle_event_self_handles();
    test_handle_event_child_handles();
    test_handle_event_coordinate_conversion();
    test_handle_event_z_order();
    test_handle_event_non_positional();
    test_handle_event_nested_propagation();

    // Panel containsEvent tests
    test_panel_contains_event_no_border();
    test_panel_contains_event_with_border();
    test_panel_contains_event_with_padding();
    test_panel_contains_event_with_titlebar();
    test_panel_contains_event_complex();

    std::cout << "\n✓ All Event API tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception" << std::endl;
    return 1;
  }
}
