/**
 * @file main.cpp
 * @brief Demonstrates advanced event handling patterns
 *
 * This example shows:
 * - Custom event handling with handleEventSelf override
 * - Event propagation through the component tree
 * - Stopping event propagation when handled
 * - Focus management and keyboard shortcuts
 * - Mouse hover states and visual feedback
 * - Different event types (mouse, keyboard, char input)
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/event.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/layout/stack_layout.h>
#include <bombfork/prong/rendering/irenderer.h>
#include <bombfork/prong/theming/color.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

using namespace bombfork::prong;
using namespace bombfork::prong::core;
using namespace bombfork::prong::layout;
using namespace bombfork::prong::rendering;
using namespace bombfork::prong::examples;

// === Custom Interactive Panel ===
// This custom panel handles events directly and provides visual feedback
class InteractivePanel : public Component {
private:
  float r, g, b, a;
  float hoverR, hoverG, hoverB;
  bool isDragging = false;
  int dragStartX = 0, dragStartY = 0;
  int clickCount = 0;
  std::string label;
  std::string lastEvent;

public:
  InteractivePanel(IRenderer* renderer, const std::string& label, float r, float g, float b)
    : Component(renderer, "InteractivePanel"), r(r), g(g), b(b), a(1.0f), hoverR(r * 1.3f), hoverG(g * 1.3f),
      hoverB(b * 1.3f), label(label) {}

  void update(double /* deltaTime */) override {
    // Required override - nothing to update for this example
  }

  void render() override {
    if (!renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();

    // Use hover color when mouse is over this component
    if (isCurrentlyHovered) {
      renderer->drawRect(gx, gy, width, height, hoverR, hoverG, hoverB, a);
    } else {
      renderer->drawRect(gx, gy, width, height, r, g, b, a);
    }

    // Draw border to show focus
    if (focusState == FocusState::FOCUSED) {
      // Draw thick border
      renderer->drawRect(gx, gy, width, 4, 1.0f, 1.0f, 0.0f, 1.0f);              // Top
      renderer->drawRect(gx, gy + height - 4, width, 4, 1.0f, 1.0f, 0.0f, 1.0f); // Bottom
      renderer->drawRect(gx, gy, 4, height, 1.0f, 1.0f, 0.0f, 1.0f);             // Left
      renderer->drawRect(gx + width - 4, gy, 4, height, 1.0f, 1.0f, 0.0f, 1.0f); // Right
    }

    // Render label and state
    std::stringstream ss;
    ss << label << "\n\nClicks: " << clickCount << "\n\n" << lastEvent;

    if (isDragging) {
      ss << "\n\nDRAGGING";
    }

    if (focusState == FocusState::FOCUSED) {
      ss << "\n\n[FOCUSED]";
    }

    // Simple text rendering (button-style for visibility)
    renderer->drawText(ss.str(), gx + 10, gy + 10, 1.0f, 1.0f, 1.0f, 1.0f);
  }

protected:
  // Override handleEventSelf to handle events for this component
  bool handleEventSelf(const Event& event) override {
    switch (event.type) {
    case Event::Type::MOUSE_PRESS:
      if (event.button == 0) { // Left click
        clickCount++;
        lastEvent = "Mouse Press (Left)";
        isDragging = true;
        dragStartX = event.localX;
        dragStartY = event.localY;

        // Request focus when clicked
        requestFocus();

        return true; // Event handled, stop propagation
      }
      break;

    case Event::Type::MOUSE_RELEASE:
      if (event.button == 0) {
        lastEvent = "Mouse Release (Left)";
        isDragging = false;
        return true;
      }
      break;

    case Event::Type::MOUSE_MOVE:
      if (isDragging) {
        int dx = event.localX - dragStartX;
        int dy = event.localY - dragStartY;
        lastEvent = "Dragging dx:" + std::to_string(dx) + " dy:" + std::to_string(dy);
      } else {
        lastEvent = "Mouse Move (" + std::to_string(event.localX) + ", " + std::to_string(event.localY) + ")";
      }
      // Don't stop propagation for mouse move - let parent track too
      return false;

    case Event::Type::KEY_PRESS:
      // Handle keyboard shortcuts when focused
      if (focusState == FocusState::FOCUSED) {
        lastEvent = "Key Press: " + std::to_string(event.key);

        // Space bar to reset counter
        if (event.key == GLFW_KEY_SPACE) {
          clickCount = 0;
          lastEvent = "Counter Reset (Space)";
          return true;
        }

        // R to change red tint
        if (event.key == GLFW_KEY_R) {
          r = (r > 0.5f) ? 0.2f : 0.8f;
          lastEvent = "Red Toggle (R)";
          return true;
        }

        // G to change green tint
        if (event.key == GLFW_KEY_G) {
          g = (g > 0.5f) ? 0.2f : 0.8f;
          lastEvent = "Green Toggle (G)";
          return true;
        }

        // B to change blue tint
        if (event.key == GLFW_KEY_B) {
          b = (b > 0.5f) ? 0.2f : 0.8f;
          lastEvent = "Blue Toggle (B)";
          return true;
        }
      }
      break;

    case Event::Type::KEY_RELEASE:
      if (focusState == FocusState::FOCUSED) {
        lastEvent = "Key Release: " + std::to_string(event.key);
      }
      break;

    case Event::Type::CHAR_INPUT:
      if (focusState == FocusState::FOCUSED) {
        lastEvent = "Char Input: " + std::to_string(event.codepoint);
        return true;
      }
      break;

    case Event::Type::MOUSE_SCROLL:
      lastEvent = "Scroll: x=" + std::to_string(event.scrollX) + " y=" + std::to_string(event.scrollY);
      return true;

    default:
      break;
    }

    return false; // Event not handled
  }
};

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
  GLFWwindow* glfwWindow = glfwCreateWindow(1000, 600, "03 - Event Handling", nullptr, nullptr);
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

  if (!renderer->initialize(1000, 600)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  // Create scene
  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Root panel
  auto rootPanel = create<Panel<>>().withSize(1000, 600).withPosition(0, 0).build();
  rootPanel->setBackgroundColor(theming::Color(0.12f, 0.12f, 0.14f, 1.0f));

  auto rootLayout = std::make_shared<StackLayout>();
  rootLayout->configure(StackLayout::Configuration{.orientation = StackOrientation::HORIZONTAL, .spacing = 20.0f});
  rootPanel->setLayout(rootLayout);

  // Create three interactive panels
  auto panel1 = std::make_unique<InteractivePanel>(renderer.get(), "Panel 1\n(Red)", 0.6f, 0.2f, 0.2f);
  panel1->setBounds(0, 0, 300, 560);

  auto panel2 = std::make_unique<InteractivePanel>(renderer.get(), "Panel 2\n(Green)", 0.2f, 0.6f, 0.2f);
  panel2->setBounds(0, 0, 300, 560);

  auto panel3 = std::make_unique<InteractivePanel>(renderer.get(), "Panel 3\n(Blue)", 0.2f, 0.2f, 0.6f);
  panel3->setBounds(0, 0, 300, 560);

  rootPanel->addChild(std::move(panel1));
  rootPanel->addChild(std::move(panel2));
  rootPanel->addChild(std::move(panel3));

  scene->addChild(std::move(rootPanel));
  scene->attach();

  std::cout << "\n=== Event Handling Demo ===" << std::endl;
  std::cout << "\nInteractions:" << std::endl;
  std::cout << "- Click any panel to focus it (yellow border)" << std::endl;
  std::cout << "- Drag within a panel to see drag tracking" << std::endl;
  std::cout << "- Move mouse to see hover effects (lighter color)" << std::endl;
  std::cout << "- Mouse wheel to see scroll events" << std::endl;
  std::cout << "\nKeyboard shortcuts (when panel is focused):" << std::endl;
  std::cout << "- SPACE: Reset click counter" << std::endl;
  std::cout << "- R: Toggle red tint" << std::endl;
  std::cout << "- G: Toggle green tint" << std::endl;
  std::cout << "- B: Toggle blue tint" << std::endl;
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
  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();

  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}
