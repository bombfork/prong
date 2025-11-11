/**
 * @file main.cpp
 * @brief FlexLayout example - Flexible box layout with grow factors
 *
 * Demonstrates:
 * - Using FlexLayout for responsive layouts
 * - Setting grow and shrink factors for flexible sizing
 * - Configuring justify and align properties
 * - How resize behavior works with FlexLayout
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/flex_layout.h>

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
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

  GLFWwindow* glfwWindow = glfwCreateWindow(900, 600, "03 - Flex Layout", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(900, 600)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Create a panel with FlexLayout (ROW direction)
  // FlexLayout is like CSS Flexbox - children can grow/shrink to fill space
  auto flexLayout = std::make_shared<FlexLayout>();
  flexLayout->configure(FlexLayout::Configuration{
    .direction = FlexDirection::ROW, .justify = FlexJustify::START, .align = FlexAlign::CENTER, .gap = 10.0f});

  auto flexPanel = create<Panel<FlexLayout>>().withSize(850, 120).withPosition(25, 50).withLayout(flexLayout).build();

  // Create buttons with different grow factors
  // Grow factor determines how much of the remaining space a component takes
  // grow=1 means "take 1 share", grow=2 means "take 2 shares", etc.

  // Set flex item properties: grow factors for flexible sizing
  flexLayout->setItemProperties({
    {.grow = 0.0f, .shrink = 1.0f, .basis = 0.0f}, // Fixed size button
    {.grow = 1.0f, .shrink = 1.0f, .basis = 0.0f}, // Grow 1x
    {.grow = 2.0f, .shrink = 1.0f, .basis = 0.0f}, // Grow 2x
    {.grow = 1.0f, .shrink = 1.0f, .basis = 0.0f}  // Grow 1x
  });

  // Small button: fixed size (grow=0, default)
  auto smallButton = create<Button>("Fixed")
                       .withSize(100, 60)
                       .withClickCallback([]() { std::cout << "Fixed button clicked" << std::endl; })
                       .build();
  flexPanel->addChild(std::move(smallButton));

  // Medium button: grows to take 1 share of remaining space
  auto mediumButton = create<Button>("Grow 1x")
                        .withSize(100, 60)
                        .withClickCallback([]() { std::cout << "Grow 1x button clicked" << std::endl; })
                        .build();
  flexPanel->addChild(std::move(mediumButton));

  // Large button: grows to take 2 shares of remaining space
  auto largeButton = create<Button>("Grow 2x")
                       .withSize(100, 60)
                       .withClickCallback([]() { std::cout << "Grow 2x button clicked" << std::endl; })
                       .build();
  flexPanel->addChild(std::move(largeButton));

  // Another grow 1x button
  auto anotherButton = create<Button>("Grow 1x")
                         .withSize(100, 60)
                         .withClickCallback([]() { std::cout << "Another button clicked" << std::endl; })
                         .build();
  flexPanel->addChild(std::move(anotherButton));

  scene->addChild(std::move(flexPanel));

  // Create a second panel demonstrating different justify/align options
  auto justifyLayout = std::make_shared<FlexLayout>();
  justifyLayout->configure(FlexLayout::Configuration{
    .direction = FlexDirection::ROW,
    .justify = FlexJustify::SPACE_BETWEEN,
    .align = FlexAlign::CENTER,
    .gap = 0.0f // No gap - SPACE_BETWEEN handles spacing
  });

  auto justifyPanel =
    create<Panel<FlexLayout>>().withSize(850, 120).withPosition(25, 200).withLayout(justifyLayout).build();

  // Add buttons - they'll be distributed with space between them
  for (int i = 1; i <= 4; ++i) {
    auto button = create<Button>("Button " + std::to_string(i))
                    .withSize(150, 60)
                    .withClickCallback([i]() { std::cout << "Button " << i << " clicked" << std::endl; })
                    .build();
    justifyPanel->addChild(std::move(button));
  }

  scene->addChild(std::move(justifyPanel));

  // Create a vertical FlexLayout panel
  auto verticalLayout = std::make_shared<FlexLayout>();
  verticalLayout->configure(FlexLayout::Configuration{
    .direction = FlexDirection::COLUMN, .justify = FlexJustify::START, .align = FlexAlign::STRETCH, .gap = 10.0f});

  auto verticalPanel =
    create<Panel<FlexLayout>>().withSize(200, 350).withPosition(25, 350).withLayout(verticalLayout).build();

  // Add buttons that stretch to fill width
  for (int i = 1; i <= 4; ++i) {
    auto button = create<Button>("Item " + std::to_string(i))
                    .withSize(180, 60)
                    .withClickCallback([i]() { std::cout << "Item " << i << " clicked" << std::endl; })
                    .build();
    verticalPanel->addChild(std::move(button));
  }

  scene->addChild(std::move(verticalPanel));

  scene->attach();

  std::cout << "Try resizing the window to see FlexLayout in action!" << std::endl;
  std::cout << "Notice how buttons with grow factors expand to fill space." << std::endl;

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
