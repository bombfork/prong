/**
 * @file main.cpp
 * @brief FlowLayout example - Automatic wrapping layout
 *
 * Demonstrates:
 * - Using FlowLayout for tag-like interfaces
 * - Automatic wrapping to next line when space runs out
 * - Configuring horizontal and vertical gaps
 * - Responsive behavior when resizing window
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/flow_layout.h>

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

  GLFWwindow* glfwWindow = glfwCreateWindow(900, 700, "06 - Flow Layout", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(900, 700)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Create a panel with FlowLayout
  // FlowLayout arranges children left-to-right, wrapping to next line when needed
  auto flowLayout1 = std::make_shared<FlowLayout>();
  flowLayout1->configure(FlowLayout::Configuration{.horizontal = true,
                                                   .mainAlignment = FlowAlignment::START,
                                                   .crossAlignment = FlowAlignment::START,
                                                   .overflowBehavior = FlowOverflow::WRAP,
                                                   .spacing = 10.0f,
                                                   .crossSpacing = 10.0f});

  auto flowPanel1 = create<Panel<FlowLayout>>().withSize(850, 200).withPosition(25, 25).withLayout(flowLayout1).build();

  // Add buttons of varying widths to demonstrate wrapping
  // When the next button doesn't fit on the current line, it wraps to the next line
  std::vector<std::string> tags = {"C++",        "Python", "JavaScript", "Rust",  "Go",  "Java",
                                   "TypeScript", "Swift",  "Kotlin",     "Ruby",  "PHP", "C#",
                                   "Haskell",    "Scala",  "Clojure",    "Elixir"};

  for (const auto& tag : tags) {
    // Variable button widths based on text length (simulating tag widths)
    int width = 60 + static_cast<int>(tag.length()) * 8;
    auto button = create<Button>(tag)
                    .withSize(width, 40)
                    .withClickCallback([tag]() { std::cout << tag << " clicked" << std::endl; })
                    .build();
    flowPanel1->addChild(std::move(button));
  }

  scene->addChild(std::move(flowPanel1));

  // Create a second panel with different gap sizes
  auto flowLayout2 = std::make_shared<FlowLayout>();
  flowLayout2->configure(FlowLayout::Configuration{
    .horizontal = true,
    .mainAlignment = FlowAlignment::START,
    .crossAlignment = FlowAlignment::START,
    .overflowBehavior = FlowOverflow::WRAP,
    .spacing = 20.0f,     // Larger horizontal gap
    .crossSpacing = 15.0f // Larger vertical gap
  });

  auto flowPanel2 =
    create<Panel<FlowLayout>>().withSize(850, 200).withPosition(25, 250).withLayout(flowLayout2).build();

  // Add more buttons with uniform size
  for (int i = 1; i <= 20; ++i) {
    auto button = create<Button>("Tag " + std::to_string(i))
                    .withSize(100, 45)
                    .withClickCallback([i]() { std::cout << "Tag " << i << " clicked" << std::endl; })
                    .build();
    flowPanel2->addChild(std::move(button));
  }

  scene->addChild(std::move(flowPanel2));

  // Create a third panel with very small gaps (compact layout)
  auto flowLayout3 = std::make_shared<FlowLayout>();
  flowLayout3->configure(FlowLayout::Configuration{.horizontal = true,
                                                   .mainAlignment = FlowAlignment::START,
                                                   .crossAlignment = FlowAlignment::START,
                                                   .overflowBehavior = FlowOverflow::WRAP,
                                                   .spacing = 5.0f,
                                                   .crossSpacing = 5.0f});

  auto flowPanel3 =
    create<Panel<FlowLayout>>().withSize(850, 180).withPosition(25, 480).withLayout(flowLayout3).build();

  // Add many small buttons for a compact tag cloud
  std::vector<std::string> skills = {"UI",     "UX",    "Design", "Layout", "Grid",   "Flex",   "Flow",  "Stack",
                                     "Dock",   "Theme", "Color",  "Font",   "Render", "Event",  "Click", "Resize",
                                     "Update", "Build", "Test",   "Deploy", "Debug",  "Profile"};

  for (const auto& skill : skills) {
    int width = 45 + static_cast<int>(skill.length()) * 7;
    auto button = create<Button>(skill)
                    .withSize(width, 35)
                    .withClickCallback([skill]() { std::cout << skill << " clicked" << std::endl; })
                    .build();
    flowPanel3->addChild(std::move(button));
  }

  scene->addChild(std::move(flowPanel3));

  scene->attach();

  std::cout << "FlowLayout Examples:" << std::endl;
  std::cout << "- Top: Programming languages with 10px gaps" << std::endl;
  std::cout << "- Middle: Numbered tags with 20px/15px gaps" << std::endl;
  std::cout << "- Bottom: Skills cloud with compact 5px gaps" << std::endl;
  std::cout << "\nTry resizing the window to see automatic wrapping!" << std::endl;

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
