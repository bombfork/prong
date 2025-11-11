/**
 * @file main.cpp
 * @brief Demonstrates runtime UI changes and dynamic layout management
 *
 * This example shows how to:
 * - Add and remove components at runtime
 * - Switch layout managers dynamically
 * - Update component properties programmatically
 * - Show and hide components
 * - Rebuild layouts on demand
 *
 * A control panel lets you manipulate a dynamic content area in real-time.
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/flex_layout.h>
#include <bombfork/prong/layout/grid_layout.h>
#include <bombfork/prong/layout/stack_layout.h>
#include <bombfork/prong/theming/color.h>

#include <iostream>
#include <random>

using namespace bombfork::prong;
using namespace bombfork::prong::layout;
using namespace bombfork::prong::theming;
using namespace bombfork::prong::examples;

// Global state for dynamic manipulation
Panel<>* g_contentPanel = nullptr;
rendering::IRenderer* g_renderer = nullptr;
int g_panelCounter = 0;

// Random number generator for colors
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> colorDist(0.2f, 0.8f);

// Helper: Create a random colored panel
std::unique_ptr<Panel<>> createRandomPanel() {
  g_panelCounter++;

  float r = colorDist(gen);
  float g = colorDist(gen);
  float b = colorDist(gen);

  auto panel = create<Panel<>>().withSize(120, 100).build();
  panel->setBackgroundColor(Color(r, g, b, 1.0f));

  // Add a label button to show the panel number
  auto label =
    create<Button>("Panel\n" + std::to_string(g_panelCounter)).withSize(100, 60).withPosition(10, 20).build();

  panel->addChild(std::move(label));
  return panel;
}

// Action: Add a panel
void addPanel() {
  if (!g_contentPanel)
    return;

  std::cout << "Adding panel #" << g_panelCounter + 1 << std::endl;
  g_contentPanel->addChild(createRandomPanel());
  g_contentPanel->invalidateLayout();
}

// Action: Remove last panel
void removePanel() {
  if (!g_contentPanel)
    return;

  auto& children = g_contentPanel->getChildren();
  if (!children.empty()) {
    std::cout << "Removing last panel" << std::endl;
    g_contentPanel->removeChild(children.back().get());
    g_contentPanel->invalidateLayout();
  } else {
    std::cout << "No panels to remove" << std::endl;
  }
}

// Action: Clear all panels
void clearPanels() {
  if (!g_contentPanel)
    return;

  std::cout << "Clearing all panels" << std::endl;
  auto& children = g_contentPanel->getChildren();
  while (!children.empty()) {
    g_contentPanel->removeChild(children.back().get());
  }
  g_contentPanel->invalidateLayout();
}

// Action: Switch to FlexLayout ROW
void useFlexLayoutRow() {
  if (!g_contentPanel)
    return;

  std::cout << "Switching to FlexLayout (ROW)" << std::endl;
  auto layout = std::make_shared<FlexLayout>();
  layout->configure(
    FlexLayout::Configuration{.direction = FlexDirection::ROW, .justify = FlexJustify::START, .gap = 10.0f});
  g_contentPanel->setLayout(layout);
  g_contentPanel->invalidateLayout();
}

// Action: Switch to FlexLayout COLUMN
void useFlexLayoutColumn() {
  if (!g_contentPanel)
    return;

  std::cout << "Switching to FlexLayout (COLUMN)" << std::endl;
  auto layout = std::make_shared<FlexLayout>();
  layout->configure(
    FlexLayout::Configuration{.direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .gap = 10.0f});
  g_contentPanel->setLayout(layout);
  g_contentPanel->invalidateLayout();
}

// Action: Switch to GridLayout
void useGridLayout() {
  if (!g_contentPanel)
    return;

  std::cout << "Switching to GridLayout (3 columns)" << std::endl;
  auto layout = std::make_shared<GridLayout>();
  layout->configure(GridLayout::Configuration{.columns = 3, .horizontalSpacing = 10.0f, .verticalSpacing = 10.0f});
  g_contentPanel->setLayout(layout);
  g_contentPanel->invalidateLayout();
}

// Action: Switch to StackLayout HORIZONTAL
void useStackLayoutHorizontal() {
  if (!g_contentPanel)
    return;

  std::cout << "Switching to StackLayout (HORIZONTAL)" << std::endl;
  auto layout = std::make_shared<StackLayout>();
  layout->configure(StackLayout::Configuration{.orientation = StackOrientation::HORIZONTAL, .spacing = 10.0f});
  g_contentPanel->setLayout(layout);
  g_contentPanel->invalidateLayout();
}

// Action: Switch to StackLayout VERTICAL
void useStackLayoutVertical() {
  if (!g_contentPanel)
    return;

  std::cout << "Switching to StackLayout (VERTICAL)" << std::endl;
  auto layout = std::make_shared<StackLayout>();
  layout->configure(StackLayout::Configuration{.orientation = StackOrientation::VERTICAL, .spacing = 10.0f});
  g_contentPanel->setLayout(layout);
  g_contentPanel->invalidateLayout();
}

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  // Create window
  GLFWwindow* glfwWindow = glfwCreateWindow(1200, 700, "04 - Dynamic Layout", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  // Create adapters
  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(1200, 700)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  g_renderer = renderer.get();

  // Create scene
  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Root panel with horizontal layout
  auto rootPanel = create<Panel<>>().withSize(1200, 700).withPosition(0, 0).build();
  rootPanel->setBackgroundColor(Color(0.12f, 0.12f, 0.14f, 1.0f));

  auto rootLayout = std::make_shared<FlexLayout>();
  rootLayout->configure(FlexLayout::Configuration{.direction = FlexDirection::ROW, .gap = 15.0f});
  rootPanel->setLayout(rootLayout);

  // LEFT: Control panel with buttons
  auto controlPanel = create<Panel<>>().withSize(280, 680).build();
  controlPanel->setBackgroundColor(Color(0.18f, 0.2f, 0.25f, 1.0f));

  auto controlLayout = std::make_shared<StackLayout>();
  controlLayout->configure(StackLayout::Configuration{.orientation = StackOrientation::VERTICAL, .spacing = 8.0f});
  controlPanel->setLayout(controlLayout);

  // Content manipulation buttons
  auto addBtn = create<Button>("Add Panel").withSize(260, 40).withClickCallback(addPanel).build();
  controlPanel->addChild(std::move(addBtn));

  auto removeBtn = create<Button>("Remove Panel").withSize(260, 40).withClickCallback(removePanel).build();
  controlPanel->addChild(std::move(removeBtn));

  auto clearBtn = create<Button>("Clear All").withSize(260, 40).withClickCallback(clearPanels).build();
  controlPanel->addChild(std::move(clearBtn));

  // Spacer
  auto spacer1 = create<Panel<>>().withSize(260, 20).build();
  spacer1->setBackgroundColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
  controlPanel->addChild(std::move(spacer1));

  // Layout switching buttons
  auto flexRowBtn = create<Button>("FlexLayout ROW").withSize(260, 40).withClickCallback(useFlexLayoutRow).build();
  controlPanel->addChild(std::move(flexRowBtn));

  auto flexColBtn =
    create<Button>("FlexLayout COLUMN").withSize(260, 40).withClickCallback(useFlexLayoutColumn).build();
  controlPanel->addChild(std::move(flexColBtn));

  auto gridBtn = create<Button>("GridLayout (3 cols)").withSize(260, 40).withClickCallback(useGridLayout).build();
  controlPanel->addChild(std::move(gridBtn));

  auto stackHBtn =
    create<Button>("StackLayout HORIZ").withSize(260, 40).withClickCallback(useStackLayoutHorizontal).build();
  controlPanel->addChild(std::move(stackHBtn));

  auto stackVBtn =
    create<Button>("StackLayout VERT").withSize(260, 40).withClickCallback(useStackLayoutVertical).build();
  controlPanel->addChild(std::move(stackVBtn));

  rootPanel->addChild(std::move(controlPanel));

  // RIGHT: Dynamic content panel
  auto contentPanel = create<Panel<>>().withSize(890, 680).build();
  contentPanel->setBackgroundColor(Color(0.15f, 0.15f, 0.18f, 1.0f));

  // Start with FlexLayout ROW
  auto initialLayout = std::make_shared<FlexLayout>();
  initialLayout->configure(FlexLayout::Configuration{.direction = FlexDirection::ROW, .gap = 10.0f});
  contentPanel->setLayout(initialLayout);

  // Store pointer for manipulation
  g_contentPanel = contentPanel.get();

  // Add a few initial panels
  for (int i = 0; i < 3; ++i) {
    contentPanel->addChild(createRandomPanel());
  }

  rootPanel->addChild(std::move(contentPanel));

  // Add root to scene
  scene->addChild(std::move(rootPanel));
  scene->attach();

  std::cout << "\n=== Dynamic Layout Demo ===" << std::endl;
  std::cout << "\nContent Manipulation:" << std::endl;
  std::cout << "- Add Panel: Add a new random colored panel" << std::endl;
  std::cout << "- Remove Panel: Remove the last panel" << std::endl;
  std::cout << "- Clear All: Remove all panels" << std::endl;
  std::cout << "\nLayout Switching:" << std::endl;
  std::cout << "- FlexLayout ROW: Horizontal flexible layout" << std::endl;
  std::cout << "- FlexLayout COLUMN: Vertical flexible layout" << std::endl;
  std::cout << "- GridLayout: 3-column grid" << std::endl;
  std::cout << "- StackLayout HORIZ: Horizontal stack" << std::endl;
  std::cout << "- StackLayout VERT: Vertical stack" << std::endl;
  std::cout << "\nPress ESC to exit\n" << std::endl;

  // Main loop
  while (!windowAdapter->shouldClose()) {
    glfwPollEvents();

    if (windowAdapter->isKeyPressed(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    if (renderer->beginFrame()) {
      renderer->clear(0.08f, 0.08f, 0.1f, 1.0f);

      scene->updateAll(0.016);
      scene->renderAll();

      renderer->endFrame();
    }

    glfwSwapBuffers(glfwWindow);
  }

  // Cleanup
  g_contentPanel = nullptr;
  g_renderer = nullptr;
  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();

  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
