/**
 * @file main.cpp
 * @brief StackLayout example - Simple vertical and horizontal stacking
 *
 * Demonstrates:
 * - Using StackLayout for vertical button arrangement
 * - Configuring spacing between elements
 * - Switching between vertical and horizontal directions
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/stack_layout.h>

#include <iostream>

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

  GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "02 - Stack Layout", nullptr, nullptr);
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

  // Create a panel with StackLayout (vertical)
  // StackLayout arranges children in a line with configurable spacing
  auto verticalLayout = std::make_shared<StackLayout>();
  verticalLayout->configure(StackLayout::Configuration{
    .orientation = StackOrientation::VERTICAL, .alignment = StackAlignment::START, .spacing = 15.0f});

  auto verticalPanel =
    create<Panel<StackLayout>>().withSize(250, 400).withPosition(50, 100).withLayout(verticalLayout).build();

  // Add buttons to the vertical panel
  // They will be automatically arranged top-to-bottom with 15px gaps
  auto button1 = create<Button>("Button 1")
                   .withSize(200, 50)
                   .withClickCallback([]() { std::cout << "Button 1 clicked" << std::endl; })
                   .build();

  auto button2 = create<Button>("Button 2")
                   .withSize(200, 50)
                   .withClickCallback([]() { std::cout << "Button 2 clicked" << std::endl; })
                   .build();

  auto button3 = create<Button>("Button 3")
                   .withSize(200, 50)
                   .withClickCallback([]() { std::cout << "Button 3 clicked" << std::endl; })
                   .build();

  auto button4 = create<Button>("Button 4")
                   .withSize(200, 50)
                   .withClickCallback([]() { std::cout << "Button 4 clicked" << std::endl; })
                   .build();

  verticalPanel->addChild(std::move(button1));
  verticalPanel->addChild(std::move(button2));
  verticalPanel->addChild(std::move(button3));
  verticalPanel->addChild(std::move(button4));

  // Create a panel with StackLayout (horizontal)
  auto horizontalLayout = std::make_shared<StackLayout>();
  horizontalLayout->configure(StackLayout::Configuration{
    .orientation = StackOrientation::HORIZONTAL, .alignment = StackAlignment::START, .spacing = 20.0f});

  auto horizontalPanel =
    create<Panel<StackLayout>>().withSize(600, 100).withPosition(350, 100).withLayout(horizontalLayout).build();

  // Add buttons to the horizontal panel
  // They will be automatically arranged left-to-right with 20px gaps
  auto hButton1 = create<Button>("Left")
                    .withSize(120, 60)
                    .withClickCallback([]() { std::cout << "Left clicked" << std::endl; })
                    .build();

  auto hButton2 = create<Button>("Center")
                    .withSize(120, 60)
                    .withClickCallback([]() { std::cout << "Center clicked" << std::endl; })
                    .build();

  auto hButton3 = create<Button>("Right")
                    .withSize(120, 60)
                    .withClickCallback([]() { std::cout << "Right clicked" << std::endl; })
                    .build();

  horizontalPanel->addChild(std::move(hButton1));
  horizontalPanel->addChild(std::move(hButton2));
  horizontalPanel->addChild(std::move(hButton3));

  // Add panels to scene
  scene->addChild(std::move(verticalPanel));
  scene->addChild(std::move(horizontalPanel));

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
