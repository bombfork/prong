/**
 * @file main.cpp
 * @brief GridLayout example - 2D grid arrangement like a calculator
 *
 * Demonstrates:
 * - Using GridLayout for 2D grid arrangements
 * - Configuring rows, columns, and gaps
 * - Creating a calculator-style button grid
 * - Equal distribution of space across cells
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/grid_layout.h>

#include <iostream>
#include <vector>

using namespace bombfork::prong;
using namespace bombfork::prong::examples;
using namespace bombfork::prong::layout;

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(700, 700, "04 - Grid Layout", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(700, 700)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Create a panel with GridLayout (3x3 grid)
  // GridLayout divides available space into equal-sized cells
  auto gridLayout = std::make_shared<GridLayout>();
  gridLayout->configure(GridLayout::Configuration{.columns = 3,
                                                  .rows = 3,
                                                  .horizontalSpacing = 10.0f,
                                                  .verticalSpacing = 10.0f,
                                                  .cellAlignment = GridAlignment::STRETCH});

  auto gridPanel = create<Panel<GridLayout>>().withSize(400, 400).withPosition(50, 50).withLayout(gridLayout).build();

  // Add 9 buttons (3x3 grid)
  // GridLayout automatically places them left-to-right, top-to-bottom
  std::vector<std::string> labels = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

  for (const auto& label : labels) {
    auto button = create<Button>(label)
                    .withSize(100, 100) // Size will be overridden by layout
                    .withClickCallback([label]() { std::cout << "Button " << label << " clicked" << std::endl; })
                    .build();
    gridPanel->addChild(std::move(button));
  }

  scene->addChild(std::move(gridPanel));

  // Create a calculator-style grid (4x4)
  auto calcLayout = std::make_shared<GridLayout>();
  calcLayout->configure(GridLayout::Configuration{.columns = 4,
                                                  .rows = 5,
                                                  .horizontalSpacing = 8.0f,
                                                  .verticalSpacing = 8.0f,
                                                  .cellAlignment = GridAlignment::STRETCH});

  auto calcPanel = create<Panel<GridLayout>>().withSize(400, 500).withPosition(250, 50).withLayout(calcLayout).build();

  // Calculator button labels
  std::vector<std::string> calcLabels = {"7", "8", "9", "/", "4", "5", "6", "*",
                                         "1", "2", "3", "-", "0", ".", "=", "+"};

  for (const auto& label : calcLabels) {
    auto button = create<Button>(label)
                    .withSize(80, 80)
                    .withClickCallback([label]() { std::cout << label << " pressed" << std::endl; })
                    .build();
    calcPanel->addChild(std::move(button));
  }

  scene->addChild(std::move(calcPanel));

  // Create a 2x5 grid for a different aspect ratio
  auto rectLayout = std::make_shared<GridLayout>();
  rectLayout->configure(GridLayout::Configuration{.columns = 5,
                                                  .rows = 2,
                                                  .horizontalSpacing = 12.0f,
                                                  .verticalSpacing = 12.0f,
                                                  .cellAlignment = GridAlignment::STRETCH});

  auto rectangularPanel =
    create<Panel<GridLayout>>().withSize(600, 240).withPosition(50, 480).withLayout(rectLayout).build();

  // Add 10 buttons
  for (int i = 1; i <= 10; ++i) {
    auto button = create<Button>("Item " + std::to_string(i))
                    .withSize(100, 100)
                    .withClickCallback([i]() { std::cout << "Item " << i << " clicked" << std::endl; })
                    .build();
    rectangularPanel->addChild(std::move(button));
  }

  scene->addChild(std::move(rectangularPanel));

  scene->attach();

  std::cout << "GridLayout Examples:" << std::endl;
  std::cout << "- Top left: 3x3 grid with 10px gaps" << std::endl;
  std::cout << "- Top right: 4x4 calculator-style grid with 8px gaps" << std::endl;
  std::cout << "- Bottom: 2x5 rectangular grid with 12px gaps" << std::endl;

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
