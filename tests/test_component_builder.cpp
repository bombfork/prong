#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
// NOTE: TextInput not included due to GLFW dependencies
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/event.h>
#include <bombfork/prong/layout/stack_layout.h>

#include <cassert>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bombfork::prong;

// Helper function to check if callback was invoked
bool callbackInvoked = false;
void resetCallback() {
  callbackInvoked = false;
}

// === Basic Builder Tests ===

void test_create_button_with_factory() {
  auto button = create<Button>("Test Button").build();

  assert(button != nullptr);
  assert(button->getText() == "Test Button");

  std::cout << "✓ Create button with factory test passed\n";
}

void test_create_button_with_default_constructor() {
  auto button = create<Button>().build();

  assert(button != nullptr);

  std::cout << "✓ Create button with default constructor test passed\n";
}

// NOTE: TextInput tests disabled due to GLFW dependencies
// void test_create_text_input() {
//   auto textInput = create<TextInput>().build();
//   assert(textInput != nullptr);
//   std::cout << "✓ Create text input test passed\n";
// }

void test_create_list_box() {
  auto listBox = create<ListBox>().build();

  assert(listBox != nullptr);

  std::cout << "✓ Create list box test passed\n";
}

void test_create_panel() {
  auto panel = create<Panel<>>().build();

  assert(panel != nullptr);

  std::cout << "✓ Create panel test passed\n";
}

// === Property Setting Tests ===

void test_with_position() {
  auto button = create<Button>("Test").withPosition(10, 20).build();

  int x, y;
  button->getPosition(x, y);
  assert(x == 10);
  assert(y == 20);

  std::cout << "✓ With position test passed\n";
}

void test_with_size() {
  auto button = create<Button>("Test").withSize(100, 50).build();

  int w, h;
  button->getSize(w, h);
  assert(w == 100);
  assert(h == 50);

  std::cout << "✓ With size test passed\n";
}

void test_with_bounds() {
  auto button = create<Button>("Test").withBounds(5, 10, 150, 75).build();

  int x, y, w, h;
  button->getBounds(x, y, w, h);
  assert(x == 5);
  assert(y == 10);
  assert(w == 150);
  assert(h == 75);

  std::cout << "✓ With bounds test passed\n";
}

void test_with_visible() {
  auto button = create<Button>("Test").withVisible(false).build();

  assert(!button->isVisible());

  std::cout << "✓ With visible test passed\n";
}

void test_with_enabled() {
  auto button = create<Button>("Test").withEnabled(false).build();

  assert(!button->isEnabled());

  std::cout << "✓ With enabled test passed\n";
}

void test_with_debug_name() {
  auto button = create<Button>("Test").withDebugName("MyButton").build();

  assert(button->getDebugName() == "MyButton");

  std::cout << "✓ With debug name test passed\n";
}

// === Callback Tests ===

void test_button_with_click_callback() {
  resetCallback();

  auto button = create<Button>("Test")
                  .withSize(100, 40) // Need size for isPointInside to work
                  .withClickCallback([]() { callbackInvoked = true; })
                  .build();

  // Simulate click using the event API (press + release)
  core::Event pressEvent{.type = core::Event::Type::MOUSE_PRESS, .localX = 10, .localY = 10, .button = 0};
  button->handleEvent(pressEvent);

  core::Event releaseEvent{.type = core::Event::Type::MOUSE_RELEASE, .localX = 10, .localY = 10, .button = 0};
  button->handleEvent(releaseEvent);

  assert(callbackInvoked);

  std::cout << "✓ Button with click callback test passed\n";
}

// NOTE: Disabled due to GLFW dependency in TextInput
/*
void test_text_input_with_text_changed_callback() {
  resetCallback();
  auto textInput = create<TextInput>()
                     .withTextChangedCallback([](const std::string& text) { callbackInvoked = true; })
                     .build();
  assert(textInput != nullptr);
  std::cout << "✓ Text input with text changed callback test passed\n";
}
*/

void test_list_box_with_selection_callback() {
  resetCallback();
  int capturedIndex = -1;
  std::string capturedItem;

  auto listBox = create<ListBox>()
                   .withItems({"Item 1", "Item 2", "Item 3"})
                   .withSelectionCallback([&capturedIndex, &capturedItem](int index, const std::string& item) {
                     callbackInvoked = true;
                     capturedIndex = index;
                     capturedItem = item;
                   })
                   .build();

  // Simulate selection using setSelectedIndex
  listBox->setSelectedIndex(1);

  assert(callbackInvoked);
  assert(capturedIndex == 1);
  assert(capturedItem == "Item 2");

  std::cout << "✓ List box with selection callback test passed\n";
}

void test_with_focus_callback() {
  resetCallback();

  auto button = create<Button>("Test").withFocusCallback([](bool focused) { callbackInvoked = focused; }).build();

  // Simulate focus gain using setFocusState
  button->setFocusState(Component::FocusState::FOCUSED);

  assert(callbackInvoked);

  std::cout << "✓ With focus callback test passed\n";
}

// === Component-Specific Property Tests ===

void test_button_with_text() {
  auto button = create<Button>().withText("New Text").build();

  assert(button->getText() == "New Text");

  std::cout << "✓ Button with text test passed\n";
}

// NOTE: Disabled due to GLFW dependency in TextInput
/*
void test_text_input_with_placeholder() {
  auto textInput = create<TextInput>().withPlaceholder("Enter text...").build();
  assert(textInput->getPlaceholder() == "Enter text...");
  std::cout << "✓ Text input with placeholder test passed\n";
}
*/

void test_list_box_with_items() {
  std::vector<std::string> items = {"A", "B", "C"};
  auto listBox = create<ListBox>().withItems(items).build();

  assert(listBox->getItems().size() == 3);
  assert(listBox->getItems()[0] == "A");
  assert(listBox->getItems()[1] == "B");
  assert(listBox->getItems()[2] == "C");

  std::cout << "✓ List box with items test passed\n";
}

// === Child Management Tests ===

void test_with_single_child() {
  auto panel = create<Panel<>>().withChild(create<Button>("Child Button").build()).build();

  assert(panel->getChildren().size() == 1);

  std::cout << "✓ With single child test passed\n";
}

void test_with_multiple_children() {
  auto panel = create<Panel<>>()
                 .withChildren(create<Button>("Button 1").build(), create<Button>("Button 2").build(),
                               create<Button>("Button 3").build())
                 .build();

  assert(panel->getChildren().size() == 3);

  std::cout << "✓ With multiple children test passed\n";
}

void test_nested_panels() {
  auto innerPanel =
    create<Panel<>>().withChildren(create<Button>("Inner 1").build(), create<Button>("Inner 2").build()).build();

  auto outerPanel = create<Panel<>>().withChild(std::move(innerPanel)).build();

  assert(outerPanel->getChildren().size() == 1);
  assert(outerPanel->getChildren()[0]->getChildren().size() == 2);

  std::cout << "✓ Nested panels test passed\n";
}

// === Layout Manager Tests ===

void test_with_layout() {
  auto stackLayout = std::make_shared<layout::StackLayout>();
  auto panel = create<Panel<>>().withLayout(stackLayout).build();

  assert(panel->hasLayout());

  std::cout << "✓ With layout test passed\n";
}

// === Chaining Tests ===

void test_method_chaining() {
  resetCallback();

  auto button = create<Button>("Chain Test")
                  .withPosition(10, 20)
                  .withSize(100, 50)
                  .withVisible(true)
                  .withEnabled(true)
                  .withDebugName("ChainedButton")
                  .withClickCallback([]() { callbackInvoked = true; })
                  .build();

  int x, y, w, h;
  button->getBounds(x, y, w, h);
  assert(x == 10);
  assert(y == 20);
  assert(w == 100);
  assert(h == 50);
  assert(button->isVisible());
  assert(button->isEnabled());
  assert(button->getDebugName() == "ChainedButton");

  // Simulate click using the event API (press + release)
  core::Event pressEvent{.type = core::Event::Type::MOUSE_PRESS, .localX = 10, .localY = 10, .button = 0};
  button->handleEvent(pressEvent);

  core::Event releaseEvent{.type = core::Event::Type::MOUSE_RELEASE, .localX = 10, .localY = 10, .button = 0};
  button->handleEvent(releaseEvent);
  assert(callbackInvoked);

  std::cout << "✓ Method chaining test passed\n";
}

// NOTE: Disabled due to GLFW dependency in TextInput
/*
void test_complex_hierarchy_with_chaining() {
  auto form = create<Panel<>>()
                .withSize(400, 300)
                .withDebugName("Form")
                .withChildren(create<TextInput>().withPlaceholder("Name").withSize(200, 30).withPosition(10,
10).build(), create<TextInput>().withPlaceholder("Email").withSize(200, 30).withPosition(10, 50).build(),
                              create<Panel<>>()
                                .withPosition(10, 100)
                                .withSize(200, 40)
                                .withChildren(create<Button>("Submit").withSize(95, 30).build(),
                                              create<Button>("Cancel").withSize(95, 30).build())
                                .build())
                .build();
  assert(form->getChildren().size() == 3);
  assert(form->width == 400);
  assert(form->height == 300);
  auto& buttonPanel = form->getChildren()[2];
  assert(buttonPanel->getChildren().size() == 2);
  std::cout << "✓ Complex hierarchy with chaining test passed\n";
}
*/

// === Direct Access Tests ===

void test_arrow_operator() {
  auto builder = create<Button>("Test");

  // Direct access via arrow operator
  builder->setText("Modified");
  builder->setSize(120, 40);

  auto button = builder.build();

  int w, h;
  button->getSize(w, h);
  assert(button->getText() == "Modified");
  assert(w == 120);
  assert(h == 40);

  std::cout << "✓ Arrow operator test passed\n";
}

void test_dereference_operator() {
  auto builder = create<Button>("Test");

  // Direct access via dereference operator
  (*builder).setText("Modified");
  (*builder).setSize(150, 50);

  auto button = builder.build();

  int w, h;
  button->getSize(w, h);
  assert(button->getText() == "Modified");
  assert(w == 150);
  assert(h == 50);

  std::cout << "✓ Dereference operator test passed\n";
}

void test_conditional_configuration() {
  bool useConfirmation = true;
  resetCallback();
  bool confirmationUsed = false;

  auto buttonBuilder = create<Button>("Action");

  if (useConfirmation) {
    buttonBuilder.withClickCallback([&confirmationUsed]() { confirmationUsed = true; });
  } else {
    buttonBuilder.withClickCallback([&confirmationUsed]() { confirmationUsed = false; });
  }

  buttonBuilder.withSize(100, 40); // Set size for event handling
  auto button = buttonBuilder.build();

  // Simulate click using the event API (press + release)
  core::Event pressEvent{.type = core::Event::Type::MOUSE_PRESS, .localX = 10, .localY = 10, .button = 0};
  button->handleEvent(pressEvent);

  core::Event releaseEvent{.type = core::Event::Type::MOUSE_RELEASE, .localX = 10, .localY = 10, .button = 0};
  button->handleEvent(releaseEvent);

  assert(confirmationUsed);

  std::cout << "✓ Conditional configuration test passed\n";
}

// === Edge Cases and Error Handling ===

void test_build_moves_ownership() {
  auto builder = create<Button>("Test");
  auto button1 = builder.build();

  // After build(), the builder should not have a component
  // (we can't easily test this without accessing private members,
  // but we can verify the returned pointer is valid)
  assert(button1 != nullptr);
  assert(button1->getText() == "Test");

  std::cout << "✓ Build moves ownership test passed\n";
}

void test_empty_children_list() {
  auto panel = create<Panel<>>().build();

  assert(panel->getChildren().empty());

  std::cout << "✓ Empty children list test passed\n";
}

void test_multiple_property_updates() {
  auto button = create<Button>("Test")
                  .withSize(100, 50)
                  .withSize(200, 100) // Override previous size
                  .build();

  int w, h;
  button->getSize(w, h);
  assert(w == 200);
  assert(h == 100);

  std::cout << "✓ Multiple property updates test passed\n";
}

// === Integration Tests ===

// NOTE: Disabled due to GLFW dependency in TextInput
/*
void test_builder_with_all_features() {
  // Large integration test with TextInput disabled
}
*/

// === Main Test Runner ===

int main() {
  std::cout << "Running ComponentBuilder Tests\n";
  std::cout << "==============================\n\n";

  try {
    // Basic builder tests
    test_create_button_with_factory();
    test_create_button_with_default_constructor();
    // test_create_text_input(); // Disabled: GLFW dependency
    test_create_list_box();
    test_create_panel();

    // Property setting tests
    test_with_position();
    test_with_size();
    test_with_bounds();
    test_with_visible();
    test_with_enabled();
    test_with_debug_name();

    // Callback tests
    test_button_with_click_callback();
    // test_text_input_with_text_changed_callback(); // Disabled: GLFW dependency
    test_list_box_with_selection_callback();
    test_with_focus_callback();

    // Component-specific property tests
    test_button_with_text();
    // test_text_input_with_placeholder(); // Disabled: GLFW dependency
    test_list_box_with_items();

    // Child management tests
    test_with_single_child();
    test_with_multiple_children();
    test_nested_panels();

    // Layout manager tests
    test_with_layout();

    // Chaining tests
    test_method_chaining();
    // test_complex_hierarchy_with_chaining(); // Disabled: GLFW dependency

    // Direct access tests
    test_arrow_operator();
    test_dereference_operator();
    test_conditional_configuration();

    // Edge cases
    test_build_moves_ownership();
    test_empty_children_list();
    test_multiple_property_updates();

    // Integration tests
    // test_builder_with_all_features(); // Disabled: GLFW dependency

    std::cout << "\n==============================\n";
    std::cout << "All ComponentBuilder tests passed! ✓\n";
    return 0;

  } catch (const std::exception& e) {
    std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\n✗ Test failed with unknown exception\n";
    return 1;
  }
}
