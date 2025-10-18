/**
 * @file demo_scene.h
 * @brief Demo Scene for Prong UI Framework
 *
 * This scene demonstrates:
 * - Scene-based architecture
 * - ComponentBuilder pattern (recommended approach)
 * - Automatic layout with FlexLayout
 * - Component hierarchy without manual positioning
 * - Event callbacks
 * - Theme integration
 */

#pragma once

#include "../../common/glfw_adapters/glfw_adapters.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/flex_layout.h>
#include <bombfork/prong/theming/color.h>
#include <bombfork/prong/theming/theme_manager.h>

#include <iostream>
#include <memory>
#include <string>

namespace demo {

using namespace bombfork::prong;
using namespace bombfork::prong::layout;

/**
 * @brief Demo scene showing basic UI components with automatic layouts
 */
class DemoScene : public Scene {
private:
  // Keep references to components we need to interact with
  TextInput* textInputPtr = nullptr;
  ListBox* listBoxPtr = nullptr;
  Button* exitButtonPtr = nullptr;
  GLFWwindow* glfwWindow = nullptr;
  int clickCount = 0;

  // GLFW adapters for TextInput (must be kept alive)
  examples::glfw::GLFWAdapters adapters;

public:
  /**
   * @brief Create demo scene
   * @param window Window interface
   * @param renderer Renderer interface
   * @param glfw GLFW window handle for exit button
   */
  DemoScene(events::IWindow* window, rendering::IRenderer* renderer, GLFWwindow* glfw)
    : Scene(window, renderer), glfwWindow(glfw) {
    buildUI();
  }

  virtual ~DemoScene() = default;

private:
  /**
   * @brief Build the UI hierarchy using ComponentBuilder (recommended pattern)
   */
  void buildUI() {
    // Set theme
    theming::ThemeManager::getInstance().setCurrentTheme("default");

    // Configure horizontal flex layout for main panel
    auto mainLayout = std::make_shared<FlexLayout>();
    mainLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::ROW, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 20.0f});

    // Configure vertical flex layout for left panel
    auto leftLayout = std::make_shared<FlexLayout>();
    leftLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 10.0f});

    // Configure horizontal flex layout for button row
    auto buttonRowLayout = std::make_shared<FlexLayout>();
    buttonRowLayout->configure(FlexLayout::Configuration{.direction = FlexDirection::ROW,
                                                         .justify = FlexJustify::SPACE_BETWEEN,
                                                         .align = FlexAlign::STRETCH,
                                                         .gap = 10.0f});

    // === ComponentBuilder Pattern (Recommended) ===
    // Build UI using fluent ComponentBuilder API
    //
    // CURRENT LIMITATION: Components don't yet calculate intrinsic sizes
    // (e.g., Button doesn't auto-size from text, TextInput doesn't have default height).
    // For now, we provide minimal size hints where needed. Future improvement will add
    // intrinsic sizing so components can be created without any size specification.
    //
    // SIZE SPECIFICATIONS EXPLAINED:
    // - withSize(0, height): Height needed for vertical layout, width stretched by FlexAlign::STRETCH
    // - withSize(width, 0): Width needed for horizontal layout, height stretched by FlexAlign::STRETCH
    // - No withSize(): Component needs intrinsic size calculation (TODO: implement)
    //
    // The layout handles:
    // - Cross-axis sizing (perpendicular to flex direction) via FlexAlign::STRETCH
    // - Main-axis positioning and spacing via FlexJustify
    // - Position calculation (no manual x,y coordinates needed)

    // Create GLFW adapters for TextInput dependencies
    adapters = examples::glfw::GLFWAdapters::create(glfwWindow);

    // Text Input - height for line height, width stretched by layout
    auto textInput =
      create<TextInput>()
        .withSize(0, 30) // Height: 30px line + padding; Width: filled by FlexAlign::STRETCH
        .withPlaceholder("Enter text here...")
        .withTextChangedCallback([](const std::string& text) { std::cout << "Text changed: " << text << std::endl; })
        .build();
    textInputPtr = textInput.get();

    // Inject GLFW adapters into TextInput for clipboard and keyboard support
    textInput->setClipboard(adapters.clipboard.get());
    textInput->setKeyboard(adapters.keyboard.get());

    // Buttons - TODO: should calculate size from text + padding automatically
    auto addButton = create<Button>("Add Item")
                       .withSize(120, 35)
                       .withClickCallback([this]() {
                         clickCount++;
                         std::string newItem = "Item " + std::to_string(clickCount);
                         if (textInputPtr && !textInputPtr->getText().empty()) {
                           newItem = textInputPtr->getText();
                           textInputPtr->setText("");
                         }
                         if (listBoxPtr) {
                           listBoxPtr->addItem(newItem);
                           std::cout << "Added: " << newItem << std::endl;
                         }
                       })
                       .build();

    auto clearButton = create<Button>("Clear")
                         .withSize(120, 35)
                         .withClickCallback([this]() {
                           if (listBoxPtr) {
                             listBoxPtr->clearItems();
                             std::cout << "List cleared" << std::endl;
                           }
                         })
                         .build();

    // Button Row container - height for buttons, width stretched by parent
    auto buttonRow = create<FlexPanel>()
                       .withSize(0, 35) // Height: match button height; Width: filled by parent
                       .withLayout(buttonRowLayout)
                       .withChildren(std::move(addButton), std::move(clearButton))
                       .build();
    buttonRow->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent

    // Info Button - width stretched by FlexAlign::STRETCH, height from base size
    auto infoButton = create<Button>("Show Info")
                        .withSize(0, 35) // Height: 35px; Width: filled by FlexAlign::STRETCH
                        .withClickCallback([]() {
                          std::cout << "\n=== Prong UI Framework ===" << std::endl;
                          std::cout << "A modern C++20 UI framework" << std::endl;
                          std::cout << "Features:" << std::endl;
                          std::cout << "  - ComponentBuilder pattern for clean API" << std::endl;
                          std::cout << "  - Header-mostly architecture" << std::endl;
                          std::cout << "  - CRTP for zero-cost abstractions" << std::endl;
                          std::cout << "  - Renderer and window agnostic" << std::endl;
                          std::cout << "  - Scene-based architecture" << std::endl;
                          std::cout << "  - Automatic layout managers" << std::endl;
                          std::cout << "=========================\n" << std::endl;
                        })
                        .build();

    // Spacer - no size, grows to fill all remaining vertical space
    auto spacer = create<Panel<>>().withSize(0, 1).build();             // Minimal height, grows with flex
    spacer->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent

    // Exit Button - width stretched, fixed height
    auto exitButton = create<Button>("Exit Application")
                        .withSize(0, 35) // Height: 35px; Width: filled by FlexAlign::STRETCH
                        .withClickCallback([this]() {
                          std::cout << "Exiting application..." << std::endl;
                          if (glfwWindow) {
                            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
                          }
                        })
                        .build();
    exitButton->setBackgroundColor(theming::Color(0.6f, 0.2f, 0.2f, 1.0f));
    exitButtonPtr = exitButton.get();

    // Left Panel - Fixed width, height stretched by parent layout
    auto leftPanel = create<FlexPanel>()
                       .withSize(300, 0)
                       .withLayout(leftLayout)
                       .withChildren(std::move(textInput), std::move(buttonRow), std::move(infoButton),
                                     std::move(spacer), std::move(exitButton))
                       .build();

    leftPanel->setBackgroundColor(theming::Color(0.15f, 0.15f, 0.18f, 1.0f));
    leftPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    leftPanel->setBorderWidth(2);
    leftPanel->setTitle("Control Panel");
    leftPanel->setPadding(20);

    // === Traditional Pattern (also supported, shown for comparison) ===
    // Right Panel - Display Area using traditional approach

    auto rightPanel = std::make_unique<FlexPanel>();
    rightPanel->setSize(0, 0); // Both dimensions filled by parent layout
    rightPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    rightPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    rightPanel->setBorderWidth(2);
    rightPanel->setTitle("Items List");
    rightPanel->setPadding(20);

    auto rightLayout = std::make_shared<FlexLayout>();
    rightLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 0.0f});
    rightPanel->setLayoutManager(rightLayout);

    // ListBox - no size needed, fills parent
    auto listBox = std::make_unique<ListBox>();
    listBox->setSelectionCallback(
      [](int index, const std::string& item) { std::cout << "Selected item " << index << ": " << item << std::endl; });
    listBox->addItem("Welcome to Prong UI Framework!");
    listBox->addItem("Click 'Add Item' to add more items");
    listBox->addItem("Click items to select them");
    listBox->addItem("Type in the text field to customize new items");
    listBox->addItem("This demo uses ComponentBuilder pattern");
    listBox->addItem("Layouts are automatic with FlexLayout");
    listBox->addItem("Clean, fluent API for building UIs!");
    listBox->addItem("No manual positioning needed!");

    listBoxPtr = listBox.get();
    rightPanel->addChild(std::move(listBox));

    // === Main Panel - Inherits size from Scene (which gets it from window) ===
    auto mainPanel =
      create<FlexPanel>().withLayout(mainLayout).withChildren(std::move(leftPanel), std::move(rightPanel)).build();

    mainPanel->setBackgroundColor(theming::Color(0.08f, 0.08f, 0.1f, 1.0f));
    mainPanel->setPadding(20);

    // Add main panel to scene (will be sized by onWindowResize)
    addChild(std::move(mainPanel));

    // Print welcome message
    std::cout << "\n=== Prong UI Framework Demo ===" << std::endl;
    std::cout << "ComponentBuilder Pattern with Scene-Based Architecture" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - Type in the text field and click 'Add Item'" << std::endl;
    std::cout << "  - Click items in the list to select them" << std::endl;
    std::cout << "  - Click 'Show Info' to see framework information" << std::endl;
    std::cout << "  - ESC or 'Exit Application' to close" << std::endl;
    std::cout << "===============================\n" << std::endl;
  }

  /**
   * @brief Handle window resize
   */
  void onWindowResize(int width, int height) override {
    // Call parent implementation
    Scene::onWindowResize(width, height);

    // Update main panel size if needed
    if (!children.empty() && children[0]) {
      children[0]->setBounds(0, 0, width, height);
    }
  }
};

} // namespace demo
