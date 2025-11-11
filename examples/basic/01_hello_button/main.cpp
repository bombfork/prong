/**
 * @file main.cpp
 * @brief Absolute minimal Prong UI example - A single button
 *
 * This is the simplest possible Prong application. It demonstrates:
 * - Basic window and renderer setup
 * - Creating a scene
 * - Adding a single button with a click callback
 * - Main event/render loop
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>

#include <functional>
#include <iostream>
#include <memory>
#include <utility>

using namespace bombfork::prong;
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

  // Create window (640x480 is plenty for a single button)
  GLFWwindow* glfwWindow = glfwCreateWindow(640, 480, "01 - Hello Button", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1); // Enable vsync

  // Create adapters
  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(640, 480)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  // Create scene - the root container for all UI components
  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Create a button centered on screen
  // Position: center of 640x480 window with 200x60 button = (220, 210)
  auto button = create<Button>("Click Me!")
                  .withSize(200, 60)
                  .withPosition(220, 210)
                  .withClickCallback([]() { std::cout << "Button clicked!" << std::endl; })
                  .build();

  // Add button to scene
  scene->addChild(std::move(button));

  // Attach scene to window (enables event handling)
  scene->attach();

  // Main loop
  while (!windowAdapter->shouldClose()) {
    glfwPollEvents();

    // ESC to exit
    if (windowAdapter->isKeyPressed(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    if (renderer->beginFrame()) {
      renderer->clear(0.1f, 0.1f, 0.12f, 1.0f); // Dark background

      scene->updateAll(0.016); // ~60 FPS
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
