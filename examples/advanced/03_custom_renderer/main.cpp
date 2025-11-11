/**
 * @file main.cpp
 * @brief Demonstrates custom renderer implementation with detailed IRenderer interface usage
 *
 * Shows frame lifecycle, texture management, batching, and clipping.
 * Note: This uses SimpleOpenGLRenderer as the base and demonstrates the concepts.
 * A full custom renderer would implement IRenderer from scratch.
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/rendering/irenderer.h>
#include <bombfork/prong/theming/color.h>

#include <functional>
#include <iostream>
#include <memory>
#include <utility>

using namespace bombfork::prong;
using namespace bombfork::prong::rendering;
using namespace bombfork::prong::examples;

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "03 - Custom Renderer", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(800, 600)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Demonstrate IRenderer interface usage
  auto panel = create<Panel<>>().withSize(400, 300).withPosition(200, 150).build();
  panel->setBackgroundColor(theming::Color(0.2f, 0.3f, 0.4f, 1.0f));

  auto button =
    create<Button>("Click Me")
      .withSize(200, 60)
      .withPosition(100, 120)
      .withClickCallback([]() { std::cout << "Button demonstrates IRenderer::drawRect and drawText" << std::endl; })
      .build();

  panel->addChild(std::move(button));
  scene->addChild(std::move(panel));
  scene->attach();

  std::cout << "\n=== Custom Renderer Demo ===" << std::endl;
  std::cout << "This example demonstrates IRenderer interface concepts:" << std::endl;
  std::cout << "- beginFrame() / endFrame() lifecycle" << std::endl;
  std::cout << "- clear() for background" << std::endl;
  std::cout << "- drawRect() for shapes" << std::endl;
  std::cout << "- drawText() for labels" << std::endl;
  std::cout << "- Scissor test for clipping" << std::endl;
  std::cout << "\nPress ESC to exit\n" << std::endl;

  while (!windowAdapter->shouldClose()) {
    glfwPollEvents();

    if (windowAdapter->isKeyPressed(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    // Frame lifecycle
    if (renderer->beginFrame()) {
      renderer->clear(0.1f, 0.1f, 0.12f, 1.0f);

      scene->updateAll(0.016);
      scene->renderAll();

      renderer->endFrame();
    }

    glfwSwapBuffers(glfwWindow);
  }

  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();
  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
