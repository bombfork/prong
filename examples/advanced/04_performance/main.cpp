/**
 * @file main.cpp
 * @brief Performance optimization demonstration - many components with efficient updates
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/grid_layout.h>

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using namespace bombfork::prong;
using namespace bombfork::prong::layout;
using namespace bombfork::prong::examples;

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(1200, 800, "04 - Performance (100 Components)", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(1200, 800)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  auto gridLayout = std::make_shared<GridLayout>();
  gridLayout->configure(GridLayout::Configuration{.columns = 10, .horizontalSpacing = 5.0f, .verticalSpacing = 5.0f});

  auto panel = create<Panel<GridLayout>>().withSize(1200, 800).withPosition(0, 0).withLayout(gridLayout).build();

  // Create 100 buttons
  std::cout << "Creating 100 components..." << std::endl;
  for (int i = 1; i <= 100; ++i) {
    auto btn = create<Button>(std::to_string(i))
                 .withSize(110, 70)
                 .withClickCallback([i]() { std::cout << "Button " << i << std::endl; })
                 .build();
    panel->addChild(std::move(btn));
  }

  scene->addChild(std::move(panel));
  scene->attach();

  std::cout << "\n=== Performance Demo ===" << std::endl;
  std::cout << "100 components in GridLayout" << std::endl;
  std::cout << "Watch FPS to see performance characteristics" << std::endl;
  std::cout << "\nOptimization techniques demonstrated:" << std::endl;
  std::cout << "- Layout caching (layout only on invalidation)" << std::endl;
  std::cout << "- Coordinate caching (global coordinates cached)" << std::endl;
  std::cout << "- Efficient event propagation" << std::endl;
  std::cout << "- CRTP for zero-overhead polymorphism" << std::endl;
  std::cout << "\nPress ESC to exit\n" << std::endl;

  auto lastTime = std::chrono::high_resolution_clock::now();
  int frameCount = 0;

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

    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

    if (duration >= 1000) {
      std::cout << "FPS: " << frameCount << " (" << (1000.0 / frameCount) << "ms per frame)" << std::endl;
      frameCount = 0;
      lastTime = currentTime;
    }
  }

  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();
  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
