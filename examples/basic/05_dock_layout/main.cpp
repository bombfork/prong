/**
 * @file main.cpp
 * @brief DockLayout example - Application-style layout with docked panels
 *
 * Demonstrates:
 * - Using DockLayout for application-style interfaces
 * - Docking panels to top, bottom, left, right
 * - Center panel that fills remaining space
 * - Typical IDE/application layout structure
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/dock_layout.h>

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

  GLFWwindow* glfwWindow = glfwCreateWindow(1000, 700, "05 - Dock Layout", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(1000, 700)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Create a panel with DockLayout
  // DockLayout divides space into 5 regions: TOP, BOTTOM, LEFT, RIGHT, CENTER
  // Each docked panel takes a fixed size, and CENTER fills remaining space
  auto dockLayout = std::make_shared<DockLayout>();
  dockLayout->configure(
    DockLayout::DockConfiguration{.allowFloating = false, .showTabs = false, .splitterThickness = 3.0f});

  auto mainPanel = create<Panel<DockLayout>>().withSize(950, 650).withPosition(25, 25).withLayout(dockLayout).build();

  // Create TOP toolbar panel (docked to top)
  auto toolbarPanel = create<Panel<>>()
                        .withSize(0, 60) // Width is ignored for TOP dock
                        .build();

  // Add some toolbar buttons
  auto newButton = create<Button>("New")
                     .withSize(80, 40)
                     .withPosition(10, 10)
                     .withClickCallback([]() { std::cout << "New clicked" << std::endl; })
                     .build();

  auto openButton = create<Button>("Open")
                      .withSize(80, 40)
                      .withPosition(100, 10)
                      .withClickCallback([]() { std::cout << "Open clicked" << std::endl; })
                      .build();

  auto saveButton = create<Button>("Save")
                      .withSize(80, 40)
                      .withPosition(190, 10)
                      .withClickCallback([]() { std::cout << "Save clicked" << std::endl; })
                      .build();

  toolbarPanel->addChild(std::move(newButton));
  toolbarPanel->addChild(std::move(openButton));
  toolbarPanel->addChild(std::move(saveButton));

  // Create LEFT sidebar panel (docked to left)
  auto leftSidebar = create<Panel<>>()
                       .withSize(200, 0) // Height is ignored for LEFT dock
                       .build();

  // Add sidebar buttons
  for (int i = 1; i <= 5; ++i) {
    auto button = create<Button>("Tool " + std::to_string(i))
                    .withSize(180, 50)
                    .withPosition(10, 10 + (i - 1) * 60)
                    .withClickCallback([i]() { std::cout << "Tool " << i << " clicked" << std::endl; })
                    .build();
    leftSidebar->addChild(std::move(button));
  }

  // Create RIGHT sidebar panel (docked to right)
  auto rightSidebar = create<Panel<>>()
                        .withSize(200, 0) // Height is ignored for RIGHT dock
                        .build();

  // Add properties panel buttons
  auto prop1 = create<Button>("Property 1")
                 .withSize(180, 40)
                 .withPosition(10, 10)
                 .withClickCallback([]() { std::cout << "Property 1" << std::endl; })
                 .build();

  auto prop2 = create<Button>("Property 2")
                 .withSize(180, 40)
                 .withPosition(10, 60)
                 .withClickCallback([]() { std::cout << "Property 2" << std::endl; })
                 .build();

  rightSidebar->addChild(std::move(prop1));
  rightSidebar->addChild(std::move(prop2));

  // Create BOTTOM status bar panel (docked to bottom)
  auto statusBar = create<Panel<>>()
                     .withSize(0, 40) // Width is ignored for BOTTOM dock
                     .build();

  auto statusButton = create<Button>("Status: Ready")
                        .withSize(200, 30)
                        .withPosition(10, 5)
                        .withClickCallback([]() { std::cout << "Status bar clicked" << std::endl; })
                        .build();

  statusBar->addChild(std::move(statusButton));

  // Create CENTER panel (fills remaining space)
  auto centerPanel = create<Panel<>>()
                       .withSize(0, 0) // Size is ignored for CENTER
                       .build();

  // Add a button in the center area
  auto centerButton = create<Button>("Main Content Area")
                        .withSize(300, 100)
                        .withPosition(100, 100)
                        .withClickCallback([]() { std::cout << "Center area clicked" << std::endl; })
                        .build();

  centerPanel->addChild(std::move(centerButton));

  // Configure dock regions
  DockLayout::DockRegion topRegion;
  topRegion.area = DockArea::TOP;
  topRegion.components.push_back(toolbarPanel.get());
  topRegion.activeComponentIndex = 0;
  topRegion.splitterConfig.initialRatio = 0.09f;
  topRegion.splitterConfig.minSize = 40.0f;
  topRegion.splitterConfig.maxSize = 80.0f;

  DockLayout::DockRegion leftRegion;
  leftRegion.area = DockArea::LEFT;
  leftRegion.components.push_back(leftSidebar.get());
  leftRegion.activeComponentIndex = 0;
  leftRegion.splitterConfig.initialRatio = 0.21f;
  leftRegion.splitterConfig.minSize = 150.0f;
  leftRegion.splitterConfig.maxSize = 300.0f;

  DockLayout::DockRegion rightRegion;
  rightRegion.area = DockArea::RIGHT;
  rightRegion.components.push_back(rightSidebar.get());
  rightRegion.activeComponentIndex = 0;
  rightRegion.splitterConfig.initialRatio = 0.21f;
  rightRegion.splitterConfig.minSize = 150.0f;
  rightRegion.splitterConfig.maxSize = 300.0f;

  DockLayout::DockRegion bottomRegion;
  bottomRegion.area = DockArea::BOTTOM;
  bottomRegion.components.push_back(statusBar.get());
  bottomRegion.activeComponentIndex = 0;
  bottomRegion.splitterConfig.initialRatio = 0.06f;
  bottomRegion.splitterConfig.minSize = 30.0f;
  bottomRegion.splitterConfig.maxSize = 60.0f;

  DockLayout::DockRegion centerRegion;
  centerRegion.area = DockArea::CENTER;
  centerRegion.components.push_back(centerPanel.get());
  centerRegion.activeComponentIndex = 0;

  // Add regions to layout
  dockLayout->addRegion(topRegion);
  dockLayout->addRegion(leftRegion);
  dockLayout->addRegion(rightRegion);
  dockLayout->addRegion(bottomRegion);
  dockLayout->addRegion(centerRegion);

  // Add panels to main panel
  mainPanel->addChild(std::move(toolbarPanel));
  mainPanel->addChild(std::move(leftSidebar));
  mainPanel->addChild(std::move(rightSidebar));
  mainPanel->addChild(std::move(statusBar));
  mainPanel->addChild(std::move(centerPanel));

  scene->addChild(std::move(mainPanel));

  scene->attach();

  std::cout << "DockLayout Example - Application-style interface" << std::endl;
  std::cout << "- Top: Toolbar with New/Open/Save buttons" << std::endl;
  std::cout << "- Left: Tool sidebar" << std::endl;
  std::cout << "- Right: Properties panel" << std::endl;
  std::cout << "- Bottom: Status bar" << std::endl;
  std::cout << "- Center: Main content area (fills remaining space)" << std::endl;

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
