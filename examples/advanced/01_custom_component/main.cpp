/**
 * @file main.cpp
 * @brief Demonstrates creating a custom component from scratch - a color picker
 *
 * This example shows:
 * - Inheriting from Component using CRTP pattern
 * - Implementing update() and render()
 * - Custom event handling
 * - Custom properties and state management
 * - Builder pattern for custom components
 * - Callbacks for value changes
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/event.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/rendering/irenderer.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>

using namespace bombfork::prong;
using namespace bombfork::prong::core;
using namespace bombfork::prong::rendering;
using namespace bombfork::prong::examples;

// === Custom ColorPicker Component ===
class ColorPicker : public Component {
public:
  using ColorChangeCallback = std::function<void(float, float, float)>;

private:
  float selectedR = 1.0f;
  float selectedG = 0.5f;
  float selectedB = 0.0f;

  bool isDragging = false;
  ColorChangeCallback onColorChange;

public:
  ColorPicker(IRenderer* renderer) : Component(renderer, "ColorPicker") {
    width = 300;
    height = 250;
  }

  // Setters
  void setColor(float r, float g, float b) {
    selectedR = std::clamp(r, 0.0f, 1.0f);
    selectedG = std::clamp(g, 0.0f, 1.0f);
    selectedB = std::clamp(b, 0.0f, 1.0f);
  }

  void getColor(float& r, float& g, float& b) const {
    r = selectedR;
    g = selectedG;
    b = selectedB;
  }

  void setColorChangeCallback(ColorChangeCallback callback) { onColorChange = std::move(callback); }

  void update(double /* deltaTime */) override {
    // No animation in this simple example
  }

  void render() override {
    if (!renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();

    // Background
    renderer->drawRect(gx, gy, width, height, 0.2f, 0.2f, 0.22f, 1.0f);

    // Draw color gradient squares (simplified color picker)
    const int swatchSize = 40;
    const int padding = 10;
    const int cols = 6;

    float colors[18][3] = {{1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f},  {1.0f, 1.0f, 0.0f},
                           {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f},  {0.0f, 0.0f, 1.0f},

                           {0.8f, 0.0f, 0.0f}, {0.8f, 0.4f, 0.0f},  {0.8f, 0.8f, 0.0f},
                           {0.0f, 0.8f, 0.0f}, {0.0f, 0.8f, 0.8f},  {0.0f, 0.0f, 0.8f},

                           {0.5f, 0.0f, 0.0f}, {0.5f, 0.25f, 0.0f}, {0.5f, 0.5f, 0.0f},
                           {0.0f, 0.5f, 0.0f}, {0.0f, 0.5f, 0.5f},  {0.0f, 0.0f, 0.5f}};

    for (int i = 0; i < 18; ++i) {
      int col = i % cols;
      int row = i / cols;
      int x = gx + padding + col * (swatchSize + padding);
      int y = gy + padding + row * (swatchSize + padding);

      renderer->drawRect(x, y, swatchSize, swatchSize, colors[i][0], colors[i][1], colors[i][2], 1.0f);

      // Highlight selected color
      if (std::abs(selectedR - colors[i][0]) < 0.01f && std::abs(selectedG - colors[i][1]) < 0.01f &&
          std::abs(selectedB - colors[i][2]) < 0.01f) {
        // Draw border
        renderer->drawRect(x - 2, y - 2, swatchSize + 4, 2, 1.0f, 1.0f, 1.0f, 1.0f);
        renderer->drawRect(x - 2, y + swatchSize, swatchSize + 4, 2, 1.0f, 1.0f, 1.0f, 1.0f);
        renderer->drawRect(x - 2, y, 2, swatchSize, 1.0f, 1.0f, 1.0f, 1.0f);
        renderer->drawRect(x + swatchSize, y, 2, swatchSize, 1.0f, 1.0f, 1.0f, 1.0f);
      }
    }

    // Draw selected color preview
    renderer->drawRect(gx + padding, gy + height - 60, width - 2 * padding, 50, selectedR, selectedG, selectedB, 1.0f);
  }

protected:
  bool handleEventSelf(const Event& event) override {
    const int swatchSize = 40;
    const int padding = 10;
    const int cols = 6;

    float colors[18][3] = {{1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f},  {1.0f, 1.0f, 0.0f},
                           {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f},  {0.0f, 0.0f, 1.0f},

                           {0.8f, 0.0f, 0.0f}, {0.8f, 0.4f, 0.0f},  {0.8f, 0.8f, 0.0f},
                           {0.0f, 0.8f, 0.0f}, {0.0f, 0.8f, 0.8f},  {0.0f, 0.0f, 0.8f},

                           {0.5f, 0.0f, 0.0f}, {0.5f, 0.25f, 0.0f}, {0.5f, 0.5f, 0.0f},
                           {0.0f, 0.5f, 0.0f}, {0.0f, 0.5f, 0.5f},  {0.0f, 0.0f, 0.5f}};

    if (event.type == Event::Type::MOUSE_PRESS && event.button == 0) {
      // Check if click is on a color swatch
      for (int i = 0; i < 18; ++i) {
        int col = i % cols;
        int row = i / cols;
        int x = padding + col * (swatchSize + padding);
        int y = padding + row * (swatchSize + padding);

        if (event.localX >= x && event.localX < x + swatchSize && event.localY >= y && event.localY < y + swatchSize) {
          selectedR = colors[i][0];
          selectedG = colors[i][1];
          selectedB = colors[i][2];

          if (onColorChange) {
            onColorChange(selectedR, selectedG, selectedB);
          }

          return true;
        }
      }
    }

    return false;
  }
};

// === Builder for ColorPicker ===
template <typename Derived = ColorPicker>
class ColorPickerBuilder {
private:
  IRenderer* renderer = nullptr;
  float r = 1.0f, g = 0.5f, b = 0.0f;
  int x = 0, y = 0;
  ColorPicker::ColorChangeCallback callback;

public:
  ColorPickerBuilder(IRenderer* renderer) : renderer(renderer) {}

  ColorPickerBuilder& withPosition(int newX, int newY) {
    x = newX;
    y = newY;
    return *this;
  }

  ColorPickerBuilder& withColor(float newR, float newG, float newB) {
    r = newR;
    g = newG;
    b = newB;
    return *this;
  }

  ColorPickerBuilder& withCallback(ColorPicker::ColorChangeCallback cb) {
    callback = std::move(cb);
    return *this;
  }

  std::unique_ptr<ColorPicker> build() {
    auto picker = std::make_unique<ColorPicker>(renderer);
    picker->setPosition(x, y);
    picker->setColor(r, g, b);
    if (callback) {
      picker->setColorChangeCallback(std::move(callback));
    }
    return picker;
  }
};

// === Main Application ===
int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "01 - Custom Component (ColorPicker)", nullptr, nullptr);
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

  // Create panel to show selected color
  auto displayPanel = create<Panel<>>().withSize(300, 200).withPosition(50, 320).build();
  displayPanel->setBackgroundColor(theming::Color(1.0f, 0.5f, 0.0f, 1.0f));

  auto* displayPtr = displayPanel.get();

  // Create color picker with callback
  auto colorPicker = ColorPickerBuilder(renderer.get())
                       .withPosition(50, 50)
                       .withColor(1.0f, 0.5f, 0.0f)
                       .withCallback([=](float r, float g, float b) {
                         displayPtr->setBackgroundColor(theming::Color(r, g, b, 1.0f));
                         std::cout << "Color changed: RGB(" << r << ", " << g << ", " << b << ")" << std::endl;
                       })
                       .build();

  scene->addChild(std::move(colorPicker));
  scene->addChild(std::move(displayPanel));
  scene->attach();

  std::cout << "\n=== Custom Component Demo ===" << std::endl;
  std::cout << "Click on color swatches to change the color" << std::endl;
  std::cout << "The bottom panel shows the selected color" << std::endl;
  std::cout << "Press ESC to exit\n" << std::endl;

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

  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();
  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
