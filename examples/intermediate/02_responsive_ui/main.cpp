/**
 * @file main.cpp
 * @brief Demonstrates window resize behavior and responsive constraints
 *
 * This example shows how components can adapt to window resizing using:
 * - ResizeBehavior (unified behavior for both axes)
 * - AxisResizeBehavior (independent horizontal/vertical behavior)
 * - ResponsiveConstraints (min/max bounds)
 *
 * Try resizing the window to see how different panels respond!
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/flex_layout.h>
#include <bombfork/prong/theming/color.h>

#include <iostream>

using namespace bombfork::prong;
using namespace bombfork::prong::layout;
using namespace bombfork::prong::theming;
using namespace bombfork::prong::examples;

// Global scene pointer for resize callback
Scene* g_scene = nullptr;

void framebufferSizeCallback(GLFWwindow* /* window */, int width, int height) {
  if (g_scene) {
    g_scene->onWindowResize(width, height);
  }
  glViewport(0, 0, width, height);
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
  const int initialWidth = 1000;
  const int initialHeight = 600;
  GLFWwindow* glfwWindow =
    glfwCreateWindow(initialWidth, initialHeight, "02 - Responsive UI (Resize Me!)", nullptr, nullptr);
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

  if (!renderer->initialize(initialWidth, initialHeight)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  // Create scene
  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());
  g_scene = scene.get();

  // Set resize callback
  glfwSetFramebufferSizeCallback(glfwWindow, framebufferSizeCallback);

  // Root panel with FlexLayout (horizontal)
  auto rootPanel = create<Panel<>>().withSize(initialWidth, initialHeight).withPosition(0, 0).build();

  rootPanel->setBackgroundColor(theming::Color(0.12f, 0.12f, 0.14f, 1.0f));
  rootPanel->setResizeBehavior(Component::ResizeBehavior::FILL);

  auto rootLayout = std::make_shared<FlexLayout>();
  rootLayout->configure(FlexLayout::Configuration{.direction = FlexDirection::ROW, .gap = 15.0f});
  rootPanel->setLayout(rootLayout);

  // === PANEL 1: FIXED BEHAVIOR ===
  // This panel maintains its original size regardless of window resize
  auto fixedPanel = create<Panel<>>().withSize(200, 550).build();
  fixedPanel->setBackgroundColor(theming::Color(0.3f, 0.2f, 0.25f, 1.0f));

  fixedPanel->setResizeBehavior(Component::ResizeBehavior::FIXED);

  auto fixedLabel = create<Button>("FIXED\n\nStays 200x550\nregardless of\nwindow size")
                      .withSize(180, 120)
                      .withPosition(10, 10)
                      .build();
  fixedPanel->addChild(std::move(fixedLabel));

  auto fixedButton = create<Button>("I don't resize!")
                       .withSize(180, 50)
                       .withPosition(10, 150)
                       .withClickCallback([]() { std::cout << "Fixed button clicked!" << std::endl; })
                       .build();
  fixedPanel->addChild(std::move(fixedButton));

  rootPanel->addChild(std::move(fixedPanel));

  // === PANEL 2: SCALE BEHAVIOR ===
  // This panel scales proportionally with the window
  auto scalePanel = create<Panel<>>().withSize(200, 550).build();
  scalePanel->setBackgroundColor(theming::Color(0.2f, 0.3f, 0.25f, 1.0f));

  scalePanel->setResizeBehavior(Component::ResizeBehavior::SCALE);

  auto scaleLabel =
    create<Button>("SCALE\n\nScales\nproportionally\nwith window").withSize(180, 120).withPosition(10, 10).build();
  scalePanel->addChild(std::move(scaleLabel));

  auto scaleButton = create<Button>("I scale!")
                       .withSize(180, 50)
                       .withPosition(10, 150)
                       .withClickCallback([]() { std::cout << "Scale button clicked!" << std::endl; })
                       .build();
  scalePanel->addChild(std::move(scaleButton));

  rootPanel->addChild(std::move(scalePanel));

  // === PANEL 3: FILL BEHAVIOR ===
  // This panel fills all available space
  auto fillPanel = create<Panel<>>().withSize(200, 550).build();
  fillPanel->setBackgroundColor(theming::Color(0.2f, 0.25f, 0.35f, 1.0f));

  fillPanel->setResizeBehavior(Component::ResizeBehavior::FILL);

  auto fillLabel =
    create<Button>("FILL\n\nFills all\navailable space\nin parent").withSize(180, 120).withPosition(10, 10).build();
  fillPanel->addChild(std::move(fillLabel));

  auto fillButton = create<Button>("I fill space!")
                      .withSize(180, 50)
                      .withPosition(10, 150)
                      .withClickCallback([]() { std::cout << "Fill button clicked!" << std::endl; })
                      .build();
  fillPanel->addChild(std::move(fillButton));

  rootPanel->addChild(std::move(fillPanel));

  // === PANEL 4: PER-AXIS BEHAVIOR WITH CONSTRAINTS ===
  // Fixed width, fill height, with min/max constraints
  auto constrainedPanel = create<Panel<>>().withSize(250, 550).build();
  constrainedPanel->setBackgroundColor(theming::Color(0.3f, 0.25f, 0.2f, 1.0f));

  // Fixed horizontal (keeps 250px width), fill vertical (stretches height)
  constrainedPanel->setAxisResizeBehavior(Component::AxisResizeBehavior::FIXED, Component::AxisResizeBehavior::FILL);

  // Set constraints to prevent too small or too large sizes
  Component::ResponsiveConstraints constraints;
  constraints.minWidth = 200;
  constraints.maxWidth = 300;
  constraints.minHeight = 300;
  constraints.maxHeight = 800;
  constrainedPanel->setConstraints(constraints);

  auto constrainedLabel = create<Button>("PER-AXIS\n+ CONSTRAINTS\n\nFixed width\nFill height\n\nMin: 200x300\nMax: "
                                         "300x800")
                            .withSize(230, 180)
                            .withPosition(10, 10)
                            .build();
  constrainedPanel->addChild(std::move(constrainedLabel));

  auto constrainedButton = create<Button>("Constrained!")
                             .withSize(230, 50)
                             .withPosition(10, 210)
                             .withClickCallback([]() { std::cout << "Constrained button clicked!" << std::endl; })
                             .build();
  constrainedPanel->addChild(std::move(constrainedButton));

  rootPanel->addChild(std::move(constrainedPanel));

  // Add root to scene
  scene->addChild(std::move(rootPanel));
  scene->attach();

  std::cout << "\n=== Responsive UI Demo ===" << std::endl;
  std::cout << "Resize the window to see different resize behaviors!" << std::endl;
  std::cout << "\nPanel behaviors from left to right:" << std::endl;
  std::cout << "1. FIXED - Stays 200x550 regardless of window size" << std::endl;
  std::cout << "2. SCALE - Scales proportionally with window" << std::endl;
  std::cout << "3. FILL - Fills all available space" << std::endl;
  std::cout << "4. PER-AXIS + CONSTRAINTS - Fixed width, fill height, bounded by min/max" << std::endl;
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
  g_scene = nullptr;
  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();

  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
