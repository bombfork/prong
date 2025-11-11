/**
 * @file main.cpp
 * @brief Custom CircularLayout - positions children in a circle
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/layout_manager.h>
#include <bombfork/prong/theming/color.h>

#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bombfork::prong;
using namespace bombfork::prong::layout;
using namespace bombfork::prong::examples;

// Custom CircularLayout using CRTP
template <typename ParentT>
class CircularLayout : public LayoutManager<CircularLayout<ParentT>> {
private:
  float radius = 150.0f;

public:
  void setRadius(float r) { radius = r; }

  Dimensions measureLayout(const std::vector<Component*>& /* components */) override {
    int size = static_cast<int>(radius * 2 + 100);
    return {size, size};
  }

  Dimensions measureLayoutConstrained(const std::vector<Component*>& components,
                                      const Dimensions& /* constraints */) override {
    return measureLayout(components);
  }

  void layout(std::vector<Component*>& components, const Dimensions& available) override {
    if (components.empty())
      return;

    int centerX = available.width / 2;
    int centerY = available.height / 2;
    int count = static_cast<int>(components.size());
    float angleStep = 2.0f * 3.14159f / count;

    for (int i = 0; i < count; ++i) {
      float angle = i * angleStep;
      int x = centerX + static_cast<int>(radius * std::cos(angle));
      int y = centerY + static_cast<int>(radius * std::sin(angle));

      int w, h;
      components[i]->getSize(w, h);
      components[i]->setPosition(x - w / 2, y - h / 2);
    }
  }
};

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "02 - Custom CircularLayout", nullptr, nullptr);
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

  using PanelType = Panel<>;

  auto panel = create<Panel<>>().withSize(800, 600).withPosition(0, 0).build();
  panel->setBackgroundColor(theming::Color(0.12f, 0.12f, 0.14f, 1.0f));

  auto circularLayout = std::make_shared<CircularLayout<PanelType>>();
  circularLayout->setRadius(200.0f);
  panel->setLayout(circularLayout);

  for (int i = 1; i <= 8; ++i) {
    auto btn = create<Button>(std::to_string(i))
                 .withSize(60, 60)
                 .withClickCallback([i]() { std::cout << "Button " << i << " clicked!" << std::endl; })
                 .build();
    panel->addChild(std::move(btn));
  }

  scene->addChild(std::move(panel));
  scene->attach();

  std::cout << "Custom CircularLayout Demo - 8 buttons in a circle" << std::endl;

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

  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();
  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
