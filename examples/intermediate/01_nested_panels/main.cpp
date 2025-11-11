/**
 * @file main.cpp
 * @brief Demonstrates complex nested layout composition
 *
 * This example shows how to combine multiple layout managers in a nested
 * hierarchy:
 * - Root: FlexLayout (ROW) - Main horizontal split
 * - Left: StackLayout (VERTICAL) - Vertical stack of buttons
 * - Center: GridLayout (2x2) - Grid of colored panels
 * - Right: FlexLayout (COLUMN) - Vertical flex container
 *   - Top section: StackLayout (HORIZONTAL) - Horizontal button row
 *   - Bottom section: Panel with centered button
 *
 * This demonstrates the power of layout composition in Prong.
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

#include <iostream>

using namespace bombfork::prong;
using namespace bombfork::prong::layout;
using namespace bombfork::prong::theming;
using namespace bombfork::prong::examples;

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
  GLFWwindow* glfwWindow = glfwCreateWindow(1200, 700, "01 - Nested Panels", nullptr, nullptr);
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

  // Create scene
  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Root container: FlexLayout ROW (horizontal split)
  auto rootPanel = create<Panel<>>().withSize(1200, 700).withPosition(0, 0).build();
  rootPanel->setBackgroundColor(Color(0.15f, 0.15f, 0.18f, 1.0f));

  auto rootLayout = std::make_shared<FlexLayout>();
  rootLayout->configure(
    FlexLayout::Configuration{.direction = FlexDirection::ROW, .justify = FlexJustify::SPACE_BETWEEN, .gap = 10});
  rootPanel->setLayout(rootLayout);

  // LEFT SECTION: StackLayout with vertical buttons
  auto leftPanel = create<Panel<>>().withSize(250, 680).build();
  leftPanel->setBackgroundColor(Color(0.2f, 0.25f, 0.3f, 1.0f));

  auto leftLayout = std::make_shared<StackLayout>();
  leftLayout->configure(StackLayout::Configuration{.orientation = StackOrientation::VERTICAL, .spacing = 10});
  leftPanel->setLayout(leftLayout);

  // Add buttons to left panel
  for (int i = 1; i <= 5; ++i) {
    auto button = create<Button>("Action " + std::to_string(i))
                    .withSize(230, 50)
                    .withClickCallback([i]() { std::cout << "Left Action " << i << " clicked!" << std::endl; })
                    .build();
    leftPanel->addChild(std::move(button));
  }

  rootPanel->addChild(std::move(leftPanel));

  // CENTER SECTION: GridLayout (2x2 grid of colored panels)
  auto centerPanel = create<Panel<>>().withSize(600, 680).build();
  centerPanel->setBackgroundColor(Color(0.18f, 0.18f, 0.2f, 1.0f));

  auto gridLayout = std::make_shared<GridLayout>();
  gridLayout->configure(
    GridLayout::Configuration{.columns = 2, .rows = 2, .horizontalSpacing = 15, .verticalSpacing = 15});
  centerPanel->setLayout(gridLayout);

  // Grid cell colors
  const float colors[4][4] = {
    {0.8f, 0.3f, 0.3f, 1.0f}, // Red
    {0.3f, 0.8f, 0.3f, 1.0f}, // Green
    {0.3f, 0.3f, 0.8f, 1.0f}, // Blue
    {0.8f, 0.8f, 0.3f, 1.0f}  // Yellow
  };

  const char* labels[4] = {"Red\nPanel", "Green\nPanel", "Blue\nPanel", "Yellow\nPanel"};

  for (int i = 0; i < 4; ++i) {
    auto gridCell = create<Panel<>>().withSize(285, 325).build();
    gridCell->setBackgroundColor(Color(colors[i][0], colors[i][1], colors[i][2], colors[i][3]));

    // Add a button to each grid cell
    auto cellButton =
      create<Button>(labels[i])
        .withSize(200, 80)
        .withPosition(42, 122)
        .withClickCallback([i]() { std::cout << "Grid cell " << i + 1 << " button clicked!" << std::endl; })
        .build();

    gridCell->addChild(std::move(cellButton));
    centerPanel->addChild(std::move(gridCell));
  }

  rootPanel->addChild(std::move(centerPanel));

  // RIGHT SECTION: FlexLayout COLUMN
  auto rightPanel = create<Panel<>>().withSize(320, 680).build();
  rightPanel->setBackgroundColor(Color(0.2f, 0.25f, 0.3f, 1.0f));

  auto rightLayout = std::make_shared<FlexLayout>();
  rightLayout->configure(
    FlexLayout::Configuration{.direction = FlexDirection::COLUMN, .justify = FlexJustify::SPACE_BETWEEN, .gap = 10});
  rightPanel->setLayout(rightLayout);

  // Top section of right: StackLayout HORIZONTAL
  auto rightTopPanel = create<Panel<>>().withSize(310, 200).build();
  rightTopPanel->setBackgroundColor(Color(0.25f, 0.3f, 0.35f, 1.0f));

  auto rightTopLayout = std::make_shared<StackLayout>();
  rightTopLayout->configure(StackLayout::Configuration{.orientation = StackOrientation::HORIZONTAL, .spacing = 10});
  rightTopPanel->setLayout(rightTopLayout);

  // Add horizontal buttons
  for (int i = 1; i <= 2; ++i) {
    auto button = create<Button>("H" + std::to_string(i))
                    .withSize(145, 60)
                    .withClickCallback([i]() { std::cout << "Right Top H" << i << " clicked!" << std::endl; })
                    .build();
    rightTopPanel->addChild(std::move(button));
  }

  rightPanel->addChild(std::move(rightTopPanel));

  // Bottom section of right: Panel with centered button
  auto rightBottomPanel = create<Panel<>>().withSize(310, 460).build();
  rightBottomPanel->setBackgroundColor(Color(0.3f, 0.25f, 0.35f, 1.0f));

  auto centerButton = create<Button>("Centered\nAction")
                        .withSize(200, 100)
                        .withPosition(55, 180)
                        .withClickCallback([]() { std::cout << "Centered action clicked!" << std::endl; })
                        .build();

  rightBottomPanel->addChild(std::move(centerButton));
  rightPanel->addChild(std::move(rightBottomPanel));

  rootPanel->addChild(std::move(rightPanel));

  // Add root panel to scene
  scene->addChild(std::move(rootPanel));
  scene->attach();

  // Main loop
  while (!windowAdapter->shouldClose()) {
    glfwPollEvents();

    if (windowAdapter->isKeyPressed(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    if (renderer->beginFrame()) {
      renderer->clear(0.1f, 0.1f, 0.12f, 1.0f);

      scene->updateAll(0.016);
      scene->renderAll();

      renderer->endFrame();
    }

    glfwSwapBuffers(glfwWindow);
  }

  // Cleanup
  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();

  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
