/**
 * @file demo_scene.h
 * @brief Demo Scene for Prong UI Framework
 *
 * This scene demonstrates:
 * - Scene-based architecture
 * - Automatic layout with FlexLayout
 * - Component hierarchy without manual positioning
 * - Event callbacks
 * - Theme integration
 */

#pragma once

#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/components/text_input.h>
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
   * @brief Build the UI hierarchy
   */
  void buildUI() {
    // Set theme
    theming::ThemeManager::getInstance().setCurrentTheme("default");

    // Create main horizontal layout (left panel and right panel side by side)
    auto mainPanel = std::make_unique<Panel<FlexLayoutManager<FlexLayout>>>();
    mainPanel->setBounds(0, 0, 1280, 720);

    // Configure FlexLayout for horizontal layout
    auto mainLayout = std::make_shared<FlexLayout>();
    mainLayout->configure(FlexLayoutManager<FlexLayout>::Configuration{
      .direction = FlexDirection::ROW, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 20.0f});
    mainPanel->setLayoutManager(mainLayout);
    mainPanel->setBackgroundColor(theming::Color(0.08f, 0.08f, 0.1f, 1.0f));
    mainPanel->setPadding(20);

    // === Left Panel - Control Panel ===
    auto leftPanel = std::make_unique<Panel<FlexLayoutManager<FlexLayout>>>();
    leftPanel->setBounds(0, 0, 300, 680);
    leftPanel->setBackgroundColor(theming::Color(0.15f, 0.15f, 0.18f, 1.0f));
    leftPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    leftPanel->setBorderWidth(2);
    leftPanel->setTitle("Control Panel");
    leftPanel->setPadding(20);

    // Configure vertical layout for left panel
    auto leftLayout = std::make_shared<FlexLayout>();
    leftLayout->configure(FlexLayoutManager<FlexLayout>::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 10.0f});
    leftPanel->setLayoutManager(leftLayout);

    // Text Input
    auto textInput = std::make_unique<TextInput>();
    textInput->setBounds(0, 0, 0, 30); // Width auto-filled, height fixed
    textInput->setPlaceholder("Enter text here...");
    textInput->setOnTextChanged([](const std::string& text) { std::cout << "Text changed: " << text << std::endl; });
    textInputPtr = textInput.get();

    // Button Row - Add and Clear
    auto buttonRow = std::make_unique<Panel<FlexLayoutManager<FlexLayout>>>();
    buttonRow->setBounds(0, 0, 0, 35);
    auto buttonRowLayout = std::make_shared<FlexLayout>();
    buttonRowLayout->configure(FlexLayoutManager<FlexLayout>::Configuration{.direction = FlexDirection::ROW,
                                                                            .justify = FlexJustify::SPACE_BETWEEN,
                                                                            .align = FlexAlign::STRETCH,
                                                                            .gap = 10.0f});
    buttonRow->setLayoutManager(buttonRowLayout);
    buttonRow->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent

    auto addButton = std::make_unique<Button>("Add Item");
    addButton->setBounds(0, 0, 120, 35);
    addButton->setClickCallback([this]() {
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
    });

    auto clearButton = std::make_unique<Button>("Clear");
    clearButton->setBounds(0, 0, 120, 35);
    clearButton->setClickCallback([this]() {
      if (listBoxPtr) {
        listBoxPtr->clearItems();
        std::cout << "List cleared" << std::endl;
      }
    });

    buttonRow->addChild(std::move(addButton));
    buttonRow->addChild(std::move(clearButton));

    // Info Button
    auto infoButton = std::make_unique<Button>("Show Info");
    infoButton->setBounds(0, 0, 0, 35);
    infoButton->setClickCallback([]() {
      std::cout << "\n=== Prong UI Framework ===" << std::endl;
      std::cout << "A modern C++20 UI framework" << std::endl;
      std::cout << "Features:" << std::endl;
      std::cout << "  - Header-mostly architecture" << std::endl;
      std::cout << "  - CRTP for zero-cost abstractions" << std::endl;
      std::cout << "  - Renderer and window agnostic" << std::endl;
      std::cout << "  - Scene-based architecture" << std::endl;
      std::cout << "  - Automatic layout managers" << std::endl;
      std::cout << "=========================\n" << std::endl;
    });

    // Spacer to push exit button to bottom
    auto spacer = std::make_unique<Panel<>>();
    spacer->setBounds(0, 0, 0, 400);                                    // Height will be filled by flex grow
    spacer->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent

    // Exit Button
    auto exitButton = std::make_unique<Button>("Exit Application");
    exitButton->setBounds(0, 0, 0, 35);
    exitButton->setBackgroundColor(theming::Color(0.6f, 0.2f, 0.2f, 1.0f));
    exitButton->setClickCallback([this]() {
      std::cout << "Exiting application..." << std::endl;
      if (glfwWindow) {
        glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
      }
    });
    exitButtonPtr = exitButton.get();

    // Add all components to left panel
    leftPanel->addChild(std::move(textInput));
    leftPanel->addChild(std::move(buttonRow));
    leftPanel->addChild(std::move(infoButton));
    leftPanel->addChild(std::move(spacer));
    leftPanel->addChild(std::move(exitButton));

    // === Right Panel - Display Area ===
    auto rightPanel = std::make_unique<Panel<FlexLayoutManager<FlexLayout>>>();
    rightPanel->setBounds(0, 0, 920, 680);
    rightPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    rightPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    rightPanel->setBorderWidth(2);
    rightPanel->setTitle("Items List");
    rightPanel->setPadding(20);

    // Configure vertical layout for right panel
    auto rightLayout = std::make_shared<FlexLayout>();
    rightLayout->configure(FlexLayoutManager<FlexLayout>::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 0.0f});
    rightPanel->setLayoutManager(rightLayout);

    // ListBox - fill remaining space
    auto listBox = std::make_unique<ListBox>();
    listBox->setBounds(0, 0, 0, 600); // Auto-fill width and height
    listBox->setSelectionCallback(
      [](int index, const std::string& item) { std::cout << "Selected item " << index << ": " << item << std::endl; });

    // Add default items
    listBox->addItem("Welcome to Prong UI Framework!");
    listBox->addItem("Click 'Add Item' to add more items");
    listBox->addItem("Click items to select them");
    listBox->addItem("Type in the text field to customize new items");
    listBox->addItem("This demo uses Scene-based architecture");
    listBox->addItem("Layouts are automatic with FlexLayout");
    listBox->addItem("No manual setBounds() positioning!");

    listBoxPtr = listBox.get();
    rightPanel->addChild(std::move(listBox));

    // Add panels to main panel
    mainPanel->addChild(std::move(leftPanel));
    mainPanel->addChild(std::move(rightPanel));

    // Add main panel to scene
    addChild(std::move(mainPanel));

    // Print welcome message
    std::cout << "\n=== Prong UI Framework Demo ===" << std::endl;
    std::cout << "Scene-based Architecture with Automatic Layouts" << std::endl;
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
