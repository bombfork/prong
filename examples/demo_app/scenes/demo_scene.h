/**
 * @file demo_scene.h
 * @brief Comprehensive Demo Scene for Prong UI Framework
 *
 * This scene demonstrates ALL framework components and layouts:
 * - Components: Button, Panel, ListBox, TextInput, Dialog, ToolBar, Viewport
 * - Layouts: FlexLayout, StackLayout, GridLayout, FlowLayout
 * - Scene-based architecture with ComponentBuilder pattern
 * - Hierarchical event handling (Scene::handleEvent propagates to children)
 * - Interactive features and callbacks
 *
 * The demo uses the hierarchical event model where:
 * - Window callbacks convert events to Event structs
 * - Scene::handleEvent() automatically propagates events through the component tree
 * - Children handle events first (topmost rendered components get priority)
 * - Components override handleEventSelf() for custom event handling
 *
 * Note: Slider, ContextMenu are available but
 * require additional setup and are not shown in this basic demo.
 */

#pragma once

#include "../../common/glfw_adapters/glfw_adapters.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/dialog.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/components/toolbar.h>
#include <bombfork/prong/components/viewport.h>
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
 * @brief Simple label component for rendering text
 */
class StatusLabel : public Component {
private:
  std::string text_;
  theming::Color textColor_;

public:
  StatusLabel(const std::string& text, theming::Color color = theming::Color(1.0f, 1.0f, 1.0f, 1.0f))
    : Component(nullptr), text_(text), textColor_(color) {}

  void setText(const std::string& text) { text_ = text; }

  void update(double deltaTime) override { (void)deltaTime; }

  void render() override {
    if (!renderer)
      return;

    // Render text at the component's global position
    int gx = getGlobalX();
    int gy = getGlobalY();

    // Calculate baseline Y position
    // Text baseline needs to be positioned so the text appears centered in the component
    // For a 24px font in a 30px component, baseline at +18 from top works well
    int baselineY = gy;

    renderer->drawText(text_.c_str(), gx + 10, baselineY, textColor_.r, textColor_.g, textColor_.b, textColor_.a);
  }
};

/**
 * @brief Comprehensive demo scene showing UI components and layouts
 */
class DemoScene : public Scene {
private:
  // Keep references to interactive components
  TextInput* textInputPtr = nullptr;
  ListBox* listBoxPtr = nullptr;
  Dialog* dialogPtr = nullptr;
  ToolBar* toolBarPtr = nullptr;
  StatusLabel* fpsLabelPtr = nullptr;
  Viewport* viewportPtr = nullptr;
  GLFWwindow* glfwWindow = nullptr;
  int clickCount = 0;
  double lastFpsUpdateTime = 0.0;
  double lastDeltaTime = 0.016;

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
    setupEventCallbacks();
  }

  virtual ~DemoScene() = default;

  /**
   * @brief Update scene and FPS counter
   */
  void update(double deltaTime) override {
    lastDeltaTime = deltaTime;
    lastFpsUpdateTime += deltaTime;

    // Update FPS counter every 0.1 seconds
    if (lastFpsUpdateTime >= 0.1 && fpsLabelPtr) {
      int fps = static_cast<int>(std::round(1.0 / deltaTime));
      fpsLabelPtr->setText("FPS: " + std::to_string(fps));
      lastFpsUpdateTime = 0.0;
    }
  }

  /**
   * @brief Setup window callbacks to route events to the scene
   */
  void setupEventCallbacks() {
    auto* window = getWindow();
    if (!window) {
      return;
    }

    events::WindowCallbacks callbacks;

    // Mouse button callback
    callbacks.mouseButton = [this](int button, int action, int mods) {
      (void)mods; // Unused for now

      // Convert window-space coordinates to scene-local coordinates
      double xpos, ypos;
      getWindow()->getCursorPos(xpos, ypos);

      core::Event event{.type = (action == events::INPUT_PRESS) ? core::Event::Type::MOUSE_PRESS
                                                                : core::Event::Type::MOUSE_RELEASE,
                        .localX = static_cast<int>(xpos),
                        .localY = static_cast<int>(ypos),
                        .button = button};

      handleEvent(event);
    };

    // Cursor position callback
    callbacks.cursorPos = [this](double xpos, double ypos) {
      core::Event event{
        .type = core::Event::Type::MOUSE_MOVE, .localX = static_cast<int>(xpos), .localY = static_cast<int>(ypos)};

      handleEvent(event);
    };

    // Scroll callback
    callbacks.scroll = [this](double xoffset, double yoffset) {
      double xpos, ypos;
      getWindow()->getCursorPos(xpos, ypos);

      core::Event event{.type = core::Event::Type::MOUSE_SCROLL,
                        .localX = static_cast<int>(xpos),
                        .localY = static_cast<int>(ypos),
                        .scrollX = xoffset,
                        .scrollY = yoffset};

      handleEvent(event);
    };

    // Key callback
    callbacks.key = [this](int key, int scancode, int action, int mods) {
      (void)scancode; // Unused

      if (action == events::INPUT_REPEAT) {
        return; // Ignore repeat for now
      }

      core::Event event{.type = (action == events::INPUT_PRESS) ? core::Event::Type::KEY_PRESS
                                                                : core::Event::Type::KEY_RELEASE,
                        .key = key,
                        .mods = mods};

      handleEvent(event);
    };

    // Character callback
    callbacks.character = [this](unsigned int codepoint) {
      core::Event event{.type = core::Event::Type::CHAR_INPUT, .codepoint = codepoint};

      handleEvent(event);
    };

    // Framebuffer size callback
    callbacks.framebufferSize = [this](int width, int height) { onWindowResize(width, height); };

    window->setCallbacks(callbacks);
  }

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

    // Set flex item properties: left and right panels fixed width, center panel grows to fill
    mainLayout->setItemProperties({
      {.grow = 0.0f, .shrink = 0.0f, .basis = 0.0f}, // Left panel: fixed width (280px)
      {.grow = 1.0f, .shrink = 1.0f, .basis = 0.0f}, // Center panel: grow to fill remaining space
      {.grow = 0.0f, .shrink = 0.0f, .basis = 0.0f}  // Right panel: fixed width (320px)
    });

    // === TOOLBAR PANEL - Top Application Toolbar ===
    auto toolbar = buildToolbar();
    toolBarPtr = toolbar.get();

    // Create toolbar panel with FlexLayout to properly size the toolbar
    auto toolbarPanelLayout = std::make_shared<FlexLayout>();
    toolbarPanelLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::ROW, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 0.0f});

    auto toolbarPanel = create<FlexPanel>().withSize(0, 40).withLayout(toolbarPanelLayout).build();
    toolbarPanel->setBackgroundColor(theming::Color(0.12f, 0.12f, 0.14f, 1.0f));
    toolbarPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    toolbarPanel->setBorderWidth(1);
    toolbarPanel->setPadding(5);
    toolbarPanel->addChild(std::move(toolbar));

    // === LEFT PANEL - Controls & Inputs ===
    auto leftPanel = buildControlPanel();

    // === CENTER PANEL - Layout Demonstrations ===
    auto centerPanel = buildCenterPanel();

    // === RIGHT PANEL - Component Showcase ===
    auto rightPanel = buildComponentShowcase();

    // === Assemble 3-panel layout with FlexLayout ===
    auto threePanelContainer = create<FlexPanel>().withLayout(mainLayout).build();
    threePanelContainer->setBackgroundColor(theming::Color(0.08f, 0.08f, 0.1f, 1.0f));
    threePanelContainer->setPadding(15);

    threePanelContainer->addChild(std::move(leftPanel));
    threePanelContainer->addChild(std::move(centerPanel));
    threePanelContainer->addChild(std::move(rightPanel));

    // === STATUS BAR PANEL - Bottom Status Information ===
    auto statusBarLayout = std::make_shared<FlexLayout>();
    statusBarLayout->configure(FlexLayout::Configuration{.direction = FlexDirection::ROW,
                                                         .justify = FlexJustify::SPACE_BETWEEN,
                                                         .align = FlexAlign::CENTER,
                                                         .gap = 10.0f});

    auto statusBarPanel = create<FlexPanel>().withSize(0, 30).withLayout(statusBarLayout).build();
    statusBarPanel->setBackgroundColor(theming::Color(0.1f, 0.1f, 0.12f, 1.0f));
    statusBarPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    statusBarPanel->setBorderWidth(1);
    statusBarPanel->setPadding(5);

    // Left status label
    auto appNameLabel = std::make_unique<StatusLabel>("Prong UI Framework - Scene Demo");
    appNameLabel->setRenderer(renderer);
    appNameLabel->setBounds(0, 0, 300, 20);
    statusBarPanel->addChild(std::move(appNameLabel));

    // Right FPS label
    auto fpsLabel = std::make_unique<StatusLabel>("FPS: 60", theming::Color(0.5f, 1.0f, 0.5f, 1.0f));
    fpsLabel->setRenderer(renderer);
    fpsLabel->setBounds(0, 0, 100, 20);
    fpsLabelPtr = fpsLabel.get();
    statusBarPanel->addChild(std::move(fpsLabel));

    // === Create outer vertical layout: Toolbar on top, 3-panel in middle, status bar at bottom ===
    auto outerLayout = std::make_shared<FlexLayout>();
    outerLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 0.0f});
    outerLayout->setItemProperties({
      {.grow = 0.0f, .shrink = 0.0f, .basis = 0.0f}, // Toolbar panel: fixed height
      {.grow = 1.0f, .shrink = 1.0f, .basis = 0.0f}, // 3-panel container: fill remaining space
      {.grow = 0.0f, .shrink = 0.0f, .basis = 0.0f}  // Status bar: fixed height
    });

    auto rootContainer = create<FlexPanel>().withLayout(outerLayout).build();
    rootContainer->setBackgroundColor(theming::Color(0.08f, 0.08f, 0.1f, 1.0f));

    rootContainer->addChild(std::move(toolbarPanel));
    rootContainer->addChild(std::move(threePanelContainer));
    rootContainer->addChild(std::move(statusBarPanel));

    // Add root container to scene
    addChild(std::move(rootContainer));

    // Initialize main container size to match scene/window
    if (!children.empty() && children[0]) {
      children[0]->setBounds(0, 0, width, height);
      children[0]->invalidateLayout();
    }

    // === Create About Dialog ===
    auto dialog = buildAboutDialog();
    dialogPtr = dialog.get();
    addChild(std::move(dialog));

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

    auto buttonRow = create<FlexPanel>().withLayout(buttonRowLayout).build();
    buttonRow->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f));

    auto addButton = create<Button>("Add")
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

    // === About Dialog Button ===
    auto aboutButton = create<Button>("About")
                         .withClickCallback([this]() {
                           if (dialogPtr) {
                             std::cout << "Showing About dialog..." << std::endl;
                             dialogPtr->show();
                           }
                         })
                         .build();
    aboutButton->setBackgroundColor(theming::Color(0.2f, 0.5f, 0.7f, 1.0f));
    leftPanel->addChild(std::move(aboutButton));

    // === Spacer ===
    auto spacer = create<Panel<>>().build();
    spacer->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f));
    leftPanel->addChild(std::move(spacer));

    // === Exit Button ===
    auto exitButton = create<Button>("Exit Application")
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

    // Configure flex properties: all panels fixed at their natural sizes
    // This allows each layout to size itself based on its content
    centerLayout->setItemProperties({
      {.grow = 0.0f, .shrink = 0.0f, .basis = 0.0f}, // GridLayout: fixed at natural size (120px)
      {.grow = 0.0f, .shrink = 1.0f, .basis = 0.0f}, // FlowLayout: fixed at natural size, can shrink if needed
      {.grow = 0.0f, .shrink = 0.0f, .basis = 0.0f}, // StackLayout: fixed at natural size (60px)
      {.grow = 1.0f, .shrink = 1.0f, .basis = 0.0f}  // Viewport: grow to fill remaining space
    });

    auto centerPanel = create<FlexPanel>().withLayout(centerLayout).build();

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

    // === Viewport Demo ===
    auto viewportPanel = buildViewportDemo();
    centerPanel->addChild(std::move(viewportPanel));

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

    auto gridPanel = create<FlexPanel>().withSize(0, 0).withLayout(gridLayout).build();

    gridPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    gridPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    gridPanel->setBorderWidth(1);
    gridPanel->setTitle("GridLayout (3x3 Button Grid)");
    gridPanel->setPadding(10);

    // Add 9 buttons in 3x3 grid
    for (int i = 1; i <= 9; ++i) {
      auto btn = create<Button>("Btn " + std::to_string(i))
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

    auto flowPanel = create<FlexPanel>().withSize(0, 0).withLayout(flowLayout).build();

    flowPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    flowPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    flowPanel->setBorderWidth(1);
    flowPanel->setTitle("FlowLayout (Tag-like Interface)");
    flowPanel->setPadding(10);

    // Add various-sized "tag" buttons
    std::vector<std::string> tags = {"C++20", "UI",        "Framework",   "Modern",
                                     "CRTP",  "Zero-cost", "Header-only", "OpenGL"};
    for (const auto& tag : tags) {
      auto btn =
        create<Button>(tag).withClickCallback([tag]() { std::cout << "Tag clicked: " << tag << std::endl; }).build();
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
    stackPanel->setSize(0, 0);
    stackPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    stackPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    stackPanel->setBorderWidth(1);
    stackPanel->setTitle("StackLayout (Horizontal Stack)");
    stackPanel->setPadding(10);
    stackPanel->setLayoutManager(stackLayout);

    // Add horizontally stacked buttons
    for (int i = 1; i <= 4; ++i) {
      auto btn = create<Button>("Stack " + std::to_string(i))
                   .withClickCallback([i]() { std::cout << "Stack button " << i << " clicked" << std::endl; })
                   .build();
      stackPanel->addChild(std::move(btn));
    }

    return stackPanel;
  }

  /**
   * @brief Build Viewport demonstration with scrollable content
   */
  std::unique_ptr<FlexPanel> buildViewportDemo() {
    // Create wrapper panel with title
    auto wrapperLayout = std::make_shared<FlexLayout>();
    wrapperLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 0.0f});

    auto wrapperPanel = create<FlexPanel>().withSize(0, 0).withLayout(wrapperLayout).build();
    wrapperPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
    wrapperPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
    wrapperPanel->setBorderWidth(1);
    wrapperPanel->setTitle("Viewport (Scrollable Content with Pan & Zoom)");
    wrapperPanel->setPadding(10);

    // Create viewport with fixed size to demonstrate scrolling
    auto viewport = std::make_unique<Viewport>();
    viewport->setRenderer(renderer);
    viewport->setBounds(0, 0, 0, 250); // Fixed height to ensure scrolling is needed

    // Set large content size to enable scrolling
    viewport->setContentSize(800, 600);

    // Enable visual features
    viewport->setShowGrid(true);
    viewport->setShowScrollbars(true);

    // Create render callback with visible content
    viewport->setRenderCallback(
      [this](rendering::IRenderer* rend, const Viewport::ViewportTransform& transform, int, int) {
        // Draw a checkerboard pattern to show viewport boundaries
        const int tileSize = 40;
        for (int y = 0; y < 15; ++y) {
          for (int x = 0; x < 20; ++x) {
            bool isDark = (x + y) % 2 == 0;
            float brightness = isDark ? 0.25f : 0.35f;

            int tileX = static_cast<int>(x * tileSize * transform.zoomLevel + transform.panX);
            int tileY = static_cast<int>(y * tileSize * transform.zoomLevel + transform.panY);
            int tileW = static_cast<int>(tileSize * transform.zoomLevel);
            int tileH = static_cast<int>(tileSize * transform.zoomLevel);

            rend->drawRect(tileX, tileY, tileW, tileH, brightness, brightness, brightness, 1.0f);
          }
        }

        // Draw border rectangles to show content boundaries
        int contentW = static_cast<int>(800 * transform.zoomLevel);
        int contentH = static_cast<int>(600 * transform.zoomLevel);
        int borderX = static_cast<int>(transform.panX);
        int borderY = static_cast<int>(transform.panY);

        // Outer content border (cyan)
        rend->drawRect(borderX, borderY, contentW, contentH, 0.0f, 0.8f, 0.8f, 1.0f);

        // Inner border showing safe area (yellow)
        rend->drawRect(borderX + 20, borderY + 20, contentW - 40, contentH - 40, 0.8f, 0.8f, 0.0f, 1.0f);

        // Draw some text labels to demonstrate content
        if (rend) {
          int labelX = static_cast<int>(50 * transform.zoomLevel + transform.panX);
          int labelY = static_cast<int>(50 * transform.zoomLevel + transform.panY);
          rend->drawText("Viewport Demo: Drag to pan, scroll to zoom", labelX, labelY, 1.0f, 1.0f, 1.0f, 1.0f);

          labelY += static_cast<int>(30 * transform.zoomLevel);
          rend->drawText("Content Size: 800x600", labelX, labelY, 0.8f, 0.8f, 0.8f, 1.0f);

          labelY += static_cast<int>(30 * transform.zoomLevel);
          rend->drawText("Cyan border = content boundary", labelX, labelY, 0.0f, 0.8f, 0.8f, 1.0f);

          labelY += static_cast<int>(30 * transform.zoomLevel);
          rend->drawText("Yellow border = safe area", labelX, labelY, 0.8f, 0.8f, 0.0f, 1.0f);
        }
      });

    // Add zoom/pan change callbacks for debugging
    viewport->setZoomCallback([](float zoom) { std::cout << "Viewport zoom: " << zoom << std::endl; });

    viewport->setPanCallback(
      [](float panX, float panY) { std::cout << "Viewport pan: (" << panX << ", " << panY << ")" << std::endl; });

    // Center content initially
    viewport->centerContent();

    viewportPtr = viewport.get();
    wrapperPanel->addChild(std::move(viewport));

    return wrapperPanel;
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
    listBox->addItem("* Button");
    listBox->addItem("* Panel");
    listBox->addItem("* TextInput");
    listBox->addItem("* ListBox");
    listBox->addItem("* Dialog");
    listBox->addItem("* ToolBar");
    listBox->addItem("* Viewport");
    listBox->addItem("");
    listBox->addItem("All layouts shown:");
    listBox->addItem("* FlexLayout");
    listBox->addItem("* GridLayout");
    listBox->addItem("* StackLayout");
    listBox->addItem("* FlowLayout");
    listBox->addItem("");
    listBox->addItem("Available (not shown):");
    listBox->addItem("• Slider");
    listBox->addItem("• ContextMenu");
    listBox->addItem("• DockLayout");

    listBoxPtr = listBox.get();
    rightPanel->addChild(std::move(listBox));

    return rightPanel;
  }

  /**
   * @brief Build Toolbar with multiple actions and separator
   */
  std::unique_ptr<ToolBar> buildToolbar() {
    auto toolbar = std::make_unique<ToolBar>();
    toolbar->setRenderer(renderer);

    // Set toolbar size and orientation
    toolbar->setOrientation(ToolBar::Orientation::HORIZONTAL);
    toolbar->setToolSize(ToolBar::ToolSize::MEDIUM);
    toolbar->setShowText(true);

    // Add File action
    int fileId = toolbar->addTool("File", "", "Open or save files", "Ctrl+F");

    // Add Edit action
    int editId = toolbar->addTool("Edit", "", "Edit operations", "Ctrl+E");

    // Add a separator
    toolbar->addSeparator();

    // Add View action (toggle button)
    int viewId = toolbar->addToggleTool("View", "", "Toggle view options", false, "Ctrl+V");

    // Add another separator
    toolbar->addSeparator();

    // Add Help action
    int helpId = toolbar->addTool("Help", "", "Show help documentation", "F1");

    // Set toolbar callback for actions
    toolbar->setToolCallback([this, fileId, editId, viewId, helpId](int toolId) {
      if (toolId == fileId) {
        std::cout << "[Toolbar] File action triggered" << std::endl;
      } else if (toolId == editId) {
        std::cout << "[Toolbar] Edit action triggered" << std::endl;
      } else if (toolId == viewId) {
        std::cout << "[Toolbar] View action triggered (toggle state changed)" << std::endl;
      } else if (toolId == helpId) {
        std::cout << "[Toolbar] Help action triggered" << std::endl;
      }
    });

    // Set toolbar state callback for toggle buttons
    toolbar->setToolStateCallback([](int toolId, bool checked) {
      std::cout << "[Toolbar] Tool " << toolId << " checked state: " << (checked ? "ON" : "OFF") << std::endl;
    });

    return toolbar;
  }

  /**
   * @brief Build About Dialog with content and close button
   */
  std::unique_ptr<Dialog> buildAboutDialog() {
    auto dialog = std::make_unique<Dialog>();

    // Configure dialog properties
    dialog->setTitle("About Prong UI Framework");
    dialog->setDialogType(Dialog::DialogType::MODAL);
    dialog->setMinimumSize(500, 400);

    // Center dialog in window (will be positioned when shown)
    int dialogWidth = 500;
    int dialogHeight = 400;
    int dialogX = (width - dialogWidth) / 2;
    int dialogY = (height - dialogHeight) / 2;
    dialog->setBounds(dialogX, dialogY, dialogWidth, dialogHeight);

    // Initially hidden
    dialog->hide();

    // Create content panel with information
    auto contentLayout = std::make_shared<FlexLayout>();
    contentLayout->configure(FlexLayout::Configuration{
      .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 15.0f});

    auto contentPanel = create<FlexPanel>().withLayout(contentLayout).build();
    contentPanel->setBackgroundColor(theming::Color(0.0f, 0.0f, 0.0f, 0.0f));
    contentPanel->setPadding(20);
    contentPanel->setBounds(0, 40, 500, 310); // Below title bar, above button area

    // Add content text using buttons as labels (since we don't have a Label component)
    auto titleLabel = create<Button>("Prong UI Framework").build();
    titleLabel->setBackgroundColor(theming::Color(0.2f, 0.2f, 0.25f, 1.0f));
    titleLabel->setEnabled(false); // Make it non-interactive
    contentPanel->addChild(std::move(titleLabel));

    auto versionLabel = create<Button>("Version 1.0.0").build();
    versionLabel->setBackgroundColor(theming::Color(0.2f, 0.2f, 0.25f, 1.0f));
    versionLabel->setEnabled(false);
    contentPanel->addChild(std::move(versionLabel));

    auto descLabel = create<Button>("A modern C++20 UI framework").build();
    descLabel->setBackgroundColor(theming::Color(0.2f, 0.2f, 0.25f, 1.0f));
    descLabel->setEnabled(false);
    contentPanel->addChild(std::move(descLabel));

    auto featuresLabel = create<Button>("Features: CRTP, Zero-cost abstractions").build();
    featuresLabel->setBackgroundColor(theming::Color(0.2f, 0.2f, 0.25f, 1.0f));
    featuresLabel->setEnabled(false);
    contentPanel->addChild(std::move(featuresLabel));

    auto rendererLabel = create<Button>("Renderer-agnostic and Window-agnostic").build();
    rendererLabel->setBackgroundColor(theming::Color(0.2f, 0.2f, 0.25f, 1.0f));
    rendererLabel->setEnabled(false);
    contentPanel->addChild(std::move(rendererLabel));

    // Add close button at the bottom
    auto closeButton = create<Button>("Close")
                         .withClickCallback([this]() {
                           std::cout << "Closing About dialog..." << std::endl;
                           if (dialogPtr) {
                             dialogPtr->hide();
                           }
                         })
                         .build();
    closeButton->setBackgroundColor(theming::Color(0.3f, 0.5f, 0.3f, 1.0f));
    closeButton->setBounds(175, 360, 150, 30); // Centered at bottom

    // Add content and button to dialog
    dialog->addChild(std::move(contentPanel));
    dialog->addChild(std::move(closeButton));

    // Set dialog callback
    dialog->setDialogCallback([](Dialog::DialogResult result) {
      std::cout << "Dialog closed with result: " << static_cast<int>(result) << std::endl;
    });

    return dialog;
  }

  /**
   * @brief Print welcome message
   */
  void printWelcomeMessage() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         Prong UI Framework - Comprehensive Demo             ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\nCore Components Demonstrated:" << std::endl;
    std::cout << "  * Button          - Interactive buttons with callbacks" << std::endl;
    std::cout << "  * Panel           - Container components with styling" << std::endl;
    std::cout << "  * TextInput       - Text entry with clipboard support" << std::endl;
    std::cout << "  * ListBox         - Scrollable item list with selection" << std::endl;
    std::cout << "  * Dialog          - Modal dialogs with buttons and content" << std::endl;
    std::cout << "  * ToolBar         - Toolbar with actions, toggles, and separators" << std::endl;
    std::cout << "  * Viewport        - Pan & zoom viewport with scrollable content" << std::endl;
    std::cout << "\nLayout Managers Demonstrated:" << std::endl;
    std::cout << "  * FlexLayout      - Flexible box layout (main structure)" << std::endl;
    std::cout << "  * GridLayout      - 3x3 button grid" << std::endl;
    std::cout << "  * StackLayout     - Horizontal button stack" << std::endl;
    std::cout << "  * FlowLayout      - Wrapping tag interface" << std::endl;
    std::cout << "\nAdditional Components Available:" << std::endl;
    std::cout << "  • Slider          - Value adjustment with visual feedback" << std::endl;
    std::cout << "  • ContextMenu     - Right-click context menus" << std::endl;
    std::cout << "  • DockLayout      - Dockable panel layout manager" << std::endl;
    std::cout << "\nInteractive Features:" << std::endl;
    std::cout << "  • Click Toolbar actions (File, Edit, View, Help) to see console output" << std::endl;
    std::cout << "  • Type in text field and click 'Add' to add items" << std::endl;
    std::cout << "  • Click 'About' button to see modal dialog with framework info" << std::endl;
    std::cout << "  • Drag in Viewport to pan, scroll wheel to zoom" << std::endl;
    std::cout << "  • Click any button to see console output" << std::endl;
    std::cout << "  • Select items in ListBox" << std::endl;
    std::cout << "  • ESC key or 'Exit Application' to close" << std::endl;
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
