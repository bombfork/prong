/**
 * @file component_builder_example.cpp
 * @brief Example demonstrating the ComponentBuilder pattern
 *
 * This example shows how to use the ComponentBuilder to create UI components
 * with a fluent interface, without needing to pass renderers or manually set properties.
 *
 * Compile and link against prong library and its dependencies.
 */

#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/layout/flex_layout.h>

#include <iostream>
#include <memory>

using namespace bombfork::prong;

/**
 * Example 1: Creating a simple button with callback
 *
 * Note: withPosition() sets the position relative to the parent component.
 * If this button is added as a child to another component, (10, 10) means
 * 10 pixels from the parent's origin, not the screen origin.
 */
void example1_simple_button() {
  std::cout << "\n=== Example 1: Simple Button ===" << std::endl;

  auto button = create<Button>("Click Me")
                  .withSize(120, 40)
                  .withPosition(10, 10) // Position relative to parent
                  .withClickCallback([]() { std::cout << "Button clicked!" << std::endl; })
                  .build();

  std::cout << "Created button: " << button->getText() << std::endl;
}

/**
 * Example 2: Creating a text input with validation callback
 */
void example2_text_input() {
  std::cout << "\n=== Example 2: Text Input ===" << std::endl;

  auto textInput =
    create<TextInput>()
      .withPlaceholder("Enter your name...")
      .withSize(200, 30)
      .withPosition(10, 60)
      .withTextChangedCallback([](const std::string& text) { std::cout << "Text changed to: " << text << std::endl; })
      .build();

  // NOTE: In a real application, you would inject GLFW adapters like this:
  //   auto adapters = examples::glfw::GLFWAdapters::create(window);
  //   textInput->setClipboard(adapters.clipboard.get());
  //   textInput->setKeyboard(adapters.keyboard.get());
  // This enables copy/paste and proper keyboard handling.
  // See examples/demo_app/scenes/demo_scene.h for a complete example.

  std::cout << "Created text input with placeholder: " << textInput->getPlaceholder() << std::endl;
}

/**
 * Example 3: Creating a list box with selection callback
 */
void example3_list_box() {
  std::cout << "\n=== Example 3: List Box ===" << std::endl;

  std::vector<std::string> items = {"Option 1", "Option 2", "Option 3", "Option 4"};

  auto listBox = create<ListBox>()
                   .withItems(items)
                   .withSize(200, 150)
                   .withPosition(10, 100)
                   .withSelectionCallback([](int index, const std::string& item) {
                     std::cout << "Selected: " << item << " at index " << index << std::endl;
                   })
                   .build();

  std::cout << "Created list box with " << listBox->getItems().size() << " items" << std::endl;
}

/**
 * Example 4: Creating a panel with nested children
 *
 * Important: Child components use coordinates relative to their parent.
 * In this example, the buttons at positions (10, 250) and (120, 250) are
 * relative to the panel's origin, NOT the screen. So if the panel is at
 * screen position (10, 10), the OK button will be at screen position (20, 260).
 */
void example4_nested_panel() {
  std::cout << "\n=== Example 4: Nested Panel ===" << std::endl;

  // Create child buttons with positions relative to the panel
  auto okButton = create<Button>("OK")
                    .withSize(100, 30)
                    .withPosition(10, 250) // Relative to panel origin
                    .withClickCallback([]() { std::cout << "OK clicked!" << std::endl; })
                    .build();

  auto cancelButton = create<Button>("Cancel")
                        .withSize(100, 30)
                        .withPosition(120, 250) // Relative to panel origin
                        .withClickCallback([]() { std::cout << "Cancel clicked!" << std::endl; })
                        .build();

  // Create panel with children
  auto panel = create<Panel<>>()
                 .withSize(400, 300)
                 .withPosition(10, 10) // Position relative to screen (if no parent)
                 .withChildren(std::move(okButton), std::move(cancelButton))
                 .build();

  std::cout << "Created panel with " << panel->getChildren().size() << " child components" << std::endl;
}

/**
 * Example 5: Complex form with multiple components and callbacks
 */
void example5_complex_form() {
  std::cout << "\n=== Example 5: Complex Form ===" << std::endl;

  // Store references to components we need to access
  std::shared_ptr<TextInput> nameInput;
  std::shared_ptr<TextInput> emailInput;
  std::shared_ptr<ListBox> resultList;

  // Create form components
  auto nameInputPtr =
    create<TextInput>().withPlaceholder("Enter name...").withSize(200, 30).withDebugName("NameInput").build();

  auto emailInputPtr =
    create<TextInput>().withPlaceholder("Enter email...").withSize(200, 30).withDebugName("EmailInput").build();

  auto resultListPtr = create<ListBox>().withSize(200, 100).withDebugName("ResultList").build();

  // Keep shared pointers for callback access
  nameInput = std::shared_ptr<TextInput>(std::move(nameInputPtr));
  emailInput = std::shared_ptr<TextInput>(std::move(emailInputPtr));
  resultList = std::shared_ptr<ListBox>(std::move(resultListPtr));

  // NOTE: In a real application with GLFW, inject adapters for both TextInputs:
  //   auto adapters = examples::glfw::GLFWAdapters::create(window);
  //   nameInput->setClipboard(adapters.clipboard.get());
  //   nameInput->setKeyboard(adapters.keyboard.get());
  //   emailInput->setClipboard(adapters.clipboard.get());
  //   emailInput->setKeyboard(adapters.keyboard.get());

  // Create submit button that accesses other components
  auto submitButton = create<Button>("Submit")
                        .withSize(100, 30)
                        .withClickCallback([nameInput, emailInput, resultList]() {
                          std::string name = nameInput->getText();
                          std::string email = emailInput->getText();
                          std::string entry = name + " <" + email + ">";

                          // In a real app, this would add to the list
                          std::cout << "Would add entry: " << entry << std::endl;
                        })
                        .build();

  std::cout << "Created complex form with interconnected components" << std::endl;
}

/**
 * Example 6: Using builder with direct access for advanced setup
 */
void example6_advanced_setup() {
  std::cout << "\n=== Example 6: Advanced Setup ===" << std::endl;

  // Create builder and configure step by step
  auto buttonBuilder = create<Button>("Process");

  // Use direct access for complex setup
  buttonBuilder->setSize(150, 40);
  buttonBuilder->setPosition(10, 10);

  // Conditionally configure based on some logic
  bool needsConfirmation = true;

  if (needsConfirmation) {
    buttonBuilder.withClickCallback([]() { std::cout << "Showing confirmation dialog..." << std::endl; });
  } else {
    buttonBuilder.withClickCallback([]() { std::cout << "Processing directly..." << std::endl; });
  }

  // Build the final component
  auto button = buttonBuilder.build();

  std::cout << "Created button with conditional configuration" << std::endl;
}

int main() {
  std::cout << "ComponentBuilder Examples" << std::endl;
  std::cout << "=========================" << std::endl;

  // Run all examples
  example1_simple_button();
  example2_text_input();
  example3_list_box();
  example4_nested_panel();
  example5_complex_form();
  example6_advanced_setup();

  std::cout << "\n=== All examples completed ===" << std::endl;

  return 0;
}
