/**
 * @file demo_scene.h
 * @brief Comprehensive Demo Scene for Prong UI Framework
 *
 * This scene demonstrates ALL framework components and layouts:
 * - Components: Button, Panel, ListBox, TextInput
 * - Layouts: FlexLayout, StackLayout, GridLayout, FlowLayout
 * - Scene-based architecture with ComponentBuilder pattern
 * - Interactive features and callbacks
 *
 * Note: Dialog, Toolbar, Viewport, Slider, ContextMenu are available but
 * require additional setup and are not shown in this basic demo.
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
#include <bombfork/prong/layout/flow_layout.h>
#include <bombfork/prong/layout/grid_layout.h>
#include <bombfork/prong/layout/stack_layout.h>
#include <bombfork/prong/theming/color.h>
#include <bombfork/prong/theming/theme_manager.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace demo {

using namespace bombfork::prong;
using namespace bombfork::prong::layout;

/**
 * @brief Comprehensive demo scene showing UI components and layouts
 */
class DemoScene : public Scene {
private:
  // Keep references to interactive components
  TextInput* textInputPtr = nullptr;
  ListBox* listBoxPtr = nullptr;
  GLFWwindow* glfwWindow = nullptr;
  int clickCount = 0;

  // GLFW adapters for TextInput
  examples::glfw::GLFWAdapters adapters;

public:
  /**
   * @brief Create comprehensive demo scene
   * @param window Window interface
   * @param renderer Renderer interface
   * @param glfw GLFW window handle
   */
  DemoScene(events::IWindow* window, rendering::IRenderer* renderer, GLFWwindow* glfw)
    : Scene(window, renderer), glfwWindow(glfw) {
    buildUI();
  }

  virtual ~DemoScene() = default;

private:
  /**
   * @brief Build comprehensive UI showcasing all components and layouts
   */
  void buildUI() {
    // Set theme
    theming::ThemeManager::getInstance().setCurrentTheme("default");

    // Create GLFW adapters for TextInput
    adapters = examples::glfw::GLFWAdapters::create(glfwWindow);

    // === Main Layout - FlexLayout Horizontal ===
    auto mainLayout = std::make_shared<FlexLayout>();
    mainLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::ROW, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 15.0f});

    // === LEFT PANEL - Controls & Inputs ===
    auto leftPanel = buildControlPanel();

    // === CENTER PANEL - Layout Demonstrations ===
    auto centerPanel = buildCenterPanel();

    // === RIGHT PANEL - Component Showcase ===
    auto rightPanel = buildComponentShowcase();

    // === Assemble with FlexLayout ===
    auto mainContainer = create<FlexPanel>().withLayout(mainLayout).build();
    mainContainer->setBackgroundColor(theming::Color(0.08f, 0.08f, 0.1f, 1.0f));
    mainContainer->setPadding(15);

    mainContainer->addChild(std::move(leftPanel));
    mainContainer->addChild(std::move(centerPanel));
    mainContainer->addChild(std::move(rightPanel));

    // Add main container to scene
    addChild(std::move(mainContainer));

    // Print welcome message
    printWelcomeMessage();
  }

  /**
   * @brief Build control panel with various input components
   */
  std::unique_ptr<FlexPanel> buildControlPanel() {
    auto leftLayout = std::make_shared<FlexLayout>();
    leftLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 10.0f});

    auto leftPanel = create<FlexPanel>().withSize(280, 0).withLayout(leftLayout).build();

    leftPanel->setBackgroundColor(theming::Color(0.15f, 0.15f, 0.18f, 1.0f));
    leftPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    leftPanel->setBorderWidth(2);
    leftPanel->setTitle("Controls");
    leftPanel->setPadding(15);

    // === TextInput Demo ===
    auto textInput =
      create<TextInput>()
        .withSize(0, 30)
        .withPlaceholder("Enter text here...")
        .withTextChangedCallback([](const std::string& text) { std::cout << "Text: " << text << std::endl; })
        .build();
    textInputPtr = textInput.get();
    textInput->setClipboard(adapters.clipboard.get());
    textInput->setKeyboard(adapters.keyboard.get());

    leftPanel->addChild(std::move(textInput));

    // === Button Row (FlexLayout) ===
    auto buttonRowLayout = std::make_shared<FlexLayout>();
    buttonRowLayout->configure(FlexLayout::Configuration{.direction = FlexDirection::ROW,
                                                         .justify = FlexJustify::SPACE_BETWEEN,
                                                         .align = FlexAlign::STRETCH,
                                                         .gap = 10.0f});

    auto buttonRow = create<FlexPanel>().withSize(0, 35).withLayout(buttonRowLayout).build();
    buttonRow->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f));

    auto addButton = create<Button>("Add")
                       .withSize(120, 35)
                       .withClickCallback([this]() {
                         clickCount++;
                         std::string item = textInputPtr && !textInputPtr->getText().empty()
                                              ? textInputPtr->getText()
                                              : "Item " + std::to_string(clickCount);
                         if (listBoxPtr) {
                           listBoxPtr->addItem(item);
                           std::cout << "Added: " << item << std::endl;
                         }
                         if (textInputPtr)
                           textInputPtr->setText("");
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

    buttonRow->addChild(std::move(addButton));
    buttonRow->addChild(std::move(clearButton));
    leftPanel->addChild(std::move(buttonRow));

    // === Info Button ===
    auto infoButton = create<Button>("Show Info")
                        .withSize(0, 35)
                        .withClickCallback([]() {
                          std::cout << "\n=== Prong UI Framework ===" << std::endl;
                          std::cout << "Modern C++20 UI Framework" << std::endl;
                          std::cout << "All layouts demonstrated!" << std::endl;
                          std::cout << "========================\n" << std::endl;
                        })
                        .build();
    leftPanel->addChild(std::move(infoButton));

    // === Spacer ===
    auto spacer = create<Panel<>>().withSize(0, 1).build();
    spacer->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f));
    leftPanel->addChild(std::move(spacer));

    // === Exit Button ===
    auto exitButton = create<Button>("Exit Application")
                        .withSize(0, 35)
                        .withClickCallback([this]() {
                          std::cout << "Exiting..." << std::endl;
                          if (glfwWindow) {
                            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
                          }
                        })
                        .build();
    exitButton->setBackgroundColor(theming::Color(0.6f, 0.2f, 0.2f, 1.0f));
    leftPanel->addChild(std::move(exitButton));

    return leftPanel;
  }

  /**
   * @brief Build center panel demonstrating various layouts
   */
  std::unique_ptr<FlexPanel> buildCenterPanel() {
    auto centerLayout = std::make_shared<FlexLayout>();
    centerLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 10.0f});

    auto centerPanel = create<FlexPanel>().withSize(0, 0).withLayout(centerLayout).build();

    centerPanel->setBackgroundColor(theming::Color(0.12f, 0.12f, 0.14f, 1.0f));
    centerPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    centerPanel->setBorderWidth(2);
    centerPanel->setTitle("Layout Demonstrations");
    centerPanel->setPadding(15);

    // === GridLayout Demo ===
    auto gridPanel = buildGridLayoutDemo();
    centerPanel->addChild(std::move(gridPanel));

    // === FlowLayout Demo ===
    auto flowPanel = buildFlowLayoutDemo();
    centerPanel->addChild(std::move(flowPanel));

    // === StackLayout Demo ===
    auto stackPanel = buildStackLayoutDemo();
    centerPanel->addChild(std::move(stackPanel));

    return centerPanel;
  }

  /**
   * @brief Build GridLayout demonstration
   */
  std::unique_ptr<FlexPanel> buildGridLayoutDemo() {
    auto gridLayout = std::make_shared<GridLayout>();
    gridLayout->configure(GridLayout::Configuration{.columns = 3,
                                                    .rows = 0,
                                                    .horizontalSpacing = 5.0f,
                                                    .verticalSpacing = 5.0f,
                                                    .cellAlignment = GridAlignment::STRETCH,
                                                    .equalCellSize = true});

    auto gridPanel = create<FlexPanel>().withSize(0, 120).withLayout(gridLayout).build();

    gridPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    gridPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    gridPanel->setBorderWidth(1);
    gridPanel->setTitle("GridLayout (3x3 Button Grid)");
    gridPanel->setPadding(10);

    // Add 9 buttons in 3x3 grid
    for (int i = 1; i <= 9; ++i) {
      auto btn = create<Button>("Btn " + std::to_string(i))
                   .withSize(0, 0)
                   .withClickCallback([i]() { std::cout << "Grid button " << i << " clicked" << std::endl; })
                   .build();
      gridPanel->addChild(std::move(btn));
    }

    return gridPanel;
  }

  /**
   * @brief Build FlowLayout demonstration
   */
  std::unique_ptr<FlexPanel> buildFlowLayoutDemo() {
    auto flowLayout = std::make_shared<FlowLayout>();
    flowLayout->configure(FlowLayout::Configuration{.horizontal = true,
                                                    .mainAlignment = FlowAlignment::START,
                                                    .crossAlignment = FlowAlignment::START,
                                                    .overflowBehavior = FlowOverflow::WRAP,
                                                    .spacing = 5.0f,
                                                    .crossSpacing = 5.0f});

    auto flowPanel = create<FlexPanel>().withSize(0, 100).withLayout(flowLayout).build();

    flowPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    flowPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    flowPanel->setBorderWidth(1);
    flowPanel->setTitle("FlowLayout (Tag-like Interface)");
    flowPanel->setPadding(10);

    // Add various-sized "tag" buttons
    std::vector<std::string> tags = {"C++20", "UI",        "Framework",   "Modern",
                                     "CRTP",  "Zero-cost", "Header-only", "OpenGL"};
    for (const auto& tag : tags) {
      auto btn = create<Button>(tag)
                   .withSize(60 + tag.length() * 5, 28)
                   .withClickCallback([tag]() { std::cout << "Tag clicked: " << tag << std::endl; })
                   .build();
      btn->setBackgroundColor(theming::Color(0.2f, 0.4f, 0.6f, 1.0f));
      flowPanel->addChild(std::move(btn));
    }

    return flowPanel;
  }

  /**
   * @brief Build StackLayout demonstration
   */
  std::unique_ptr<Panel<StackLayout>> buildStackLayoutDemo() {
    auto stackLayout = std::make_shared<StackLayout>();
    stackLayout->configure(StackLayout::Configuration{.orientation = StackOrientation::HORIZONTAL,
                                                      .alignment = StackAlignment::CENTER,
                                                      .spacing = 10.0f,
                                                      .expandMain = false,
                                                      .expandCross = true});

    auto stackPanel = std::make_unique<Panel<StackLayout>>();
    stackPanel->setSize(0, 60);
    stackPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    stackPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    stackPanel->setBorderWidth(1);
    stackPanel->setTitle("StackLayout (Horizontal Stack)");
    stackPanel->setPadding(10);
    stackPanel->setLayoutManager(stackLayout);

    // Add horizontally stacked buttons
    for (int i = 1; i <= 4; ++i) {
      auto btn = create<Button>("Stack " + std::to_string(i))
                   .withSize(80, 35)
                   .withClickCallback([i]() { std::cout << "Stack button " << i << " clicked" << std::endl; })
                   .build();
      stackPanel->addChild(std::move(btn));
    }

    return stackPanel;
  }

  /**
   * @brief Build right panel with component showcase
   */
  std::unique_ptr<FlexPanel> buildComponentShowcase() {
    auto rightLayout = std::make_shared<FlexLayout>();
    rightLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 10.0f});

    auto rightPanel = create<FlexPanel>().withSize(320, 0).withLayout(rightLayout).build();

    rightPanel->setBackgroundColor(theming::Color(0.15f, 0.15f, 0.18f, 1.0f));
    rightPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    rightPanel->setBorderWidth(2);
    rightPanel->setTitle("Component Showcase");
    rightPanel->setPadding(15);

    // === ListBox Demo ===
    auto listBox = std::make_unique<ListBox>();
    listBox->setSelectionCallback([](int index, const std::string& item) {
      std::cout << "Selected: " << item << " (index " << index << ")" << std::endl;
    });
    listBox->addItem("Welcome to Prong Demo!");
    listBox->addItem("All components shown:");
    listBox->addItem("✓ Button");
    listBox->addItem("✓ Panel");
    listBox->addItem("✓ TextInput");
    listBox->addItem("✓ ListBox");
    listBox->addItem("");
    listBox->addItem("All layouts shown:");
    listBox->addItem("✓ FlexLayout");
    listBox->addItem("✓ GridLayout");
    listBox->addItem("✓ StackLayout");
    listBox->addItem("✓ FlowLayout");
    listBox->addItem("");
    listBox->addItem("Available (not shown):");
    listBox->addItem("• Dialog");
    listBox->addItem("• Toolbar");
    listBox->addItem("• Viewport");
    listBox->addItem("• Slider");
    listBox->addItem("• ContextMenu");
    listBox->addItem("• DockLayout");

    listBoxPtr = listBox.get();
    rightPanel->addChild(std::move(listBox));

    return rightPanel;
  }

  /**
   * @brief Print welcome message
   */
  void printWelcomeMessage() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         Prong UI Framework - Comprehensive Demo             ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\nCore Components Demonstrated:" << std::endl;
    std::cout << "  ✓ Button          - Interactive buttons with callbacks" << std::endl;
    std::cout << "  ✓ Panel           - Container components with styling" << std::endl;
    std::cout << "  ✓ TextInput       - Text entry with clipboard support" << std::endl;
    std::cout << "  ✓ ListBox         - Scrollable item list with selection" << std::endl;
    std::cout << "\nLayout Managers Demonstrated:" << std::endl;
    std::cout << "  ✓ FlexLayout      - Flexible box layout (main structure)" << std::endl;
    std::cout << "  ✓ GridLayout      - 3x3 button grid" << std::endl;
    std::cout << "  ✓ StackLayout     - Horizontal button stack" << std::endl;
    std::cout << "  ✓ FlowLayout      - Wrapping tag interface" << std::endl;
    std::cout << "\nAdditional Components Available:" << std::endl;
    std::cout << "  • Dialog          - Modal dialogs with buttons" << std::endl;
    std::cout << "  • Toolbar         - Top toolbar with tool buttons" << std::endl;
    std::cout << "  • Viewport        - Scrollable viewport with grid" << std::endl;
    std::cout << "  • Slider          - Value adjustment with visual feedback" << std::endl;
    std::cout << "  • ContextMenu     - Right-click context menus" << std::endl;
    std::cout << "  • DockLayout      - Dockable panel layout manager" << std::endl;
    std::cout << "\nInteractive Features:" << std::endl;
    std::cout << "  • Type in text field and click 'Add' to add items" << std::endl;
    std::cout << "  • Click 'Show Info' for framework information" << std::endl;
    std::cout << "  • Click any button to see console output" << std::endl;
    std::cout << "  • Select items in ListBox" << std::endl;
    std::cout << "  • ESC or 'Exit Application' to close" << std::endl;
    std::cout << "\n══════════════════════════════════════════════════════════════\n" << std::endl;
  }

  /**
   * @brief Handle window resize
   */
  void onWindowResize(int width, int height) override {
    Scene::onWindowResize(width, height);

    if (!children.empty() && children[0]) {
      children[0]->setBounds(0, 0, width, height);
      children[0]->invalidateLayout();
    }
  }
};

} // namespace demo
