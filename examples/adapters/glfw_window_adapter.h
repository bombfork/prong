#pragma once

#include <GLFW/glfw3.h>
#include <bombfork/prong/events/iwindow.h>

namespace bombfork::prong::examples {

/**
 * @brief GLFW window adapter for Prong
 *
 * This adapter makes a GLFWwindow compatible with Prong's IWindow interface.
 * It demonstrates how to bridge GLFW to the Prong event system.
 *
 * Usage:
 * @code
 * GLFWwindow* glfwWindow = glfwCreateWindow(1280, 720, "App", nullptr, nullptr);
 * auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
 * EventDispatcher dispatcher(windowAdapter.get());
 * @endcode
 */
class GLFWWindowAdapter : public bombfork::prong::events::IWindow {
private:
  GLFWwindow* glfwWindow;
  bombfork::prong::events::WindowCallbacks callbacks;

public:
  explicit GLFWWindowAdapter(GLFWwindow* window) : glfwWindow(window) {
    if (glfwWindow) {
      // Store 'this' pointer so GLFW callbacks can access the adapter
      glfwSetWindowUserPointer(glfwWindow, this);
    }
  }

  ~GLFWWindowAdapter() {
    if (glfwWindow) {
      unregisterGLFWCallbacks();
      glfwSetWindowUserPointer(glfwWindow, nullptr);
    }
  }

  // Prevent copying (GLFW window is not copyable)
  GLFWWindowAdapter(const GLFWWindowAdapter&) = delete;
  GLFWWindowAdapter& operator=(const GLFWWindowAdapter&) = delete;

  // === IWindow Implementation ===

  void getSize(int& width, int& height) const override { glfwGetWindowSize(glfwWindow, &width, &height); }

  void* getNativeHandle() override { return glfwWindow; }

  void getCursorPos(double& x, double& y) const override { glfwGetCursorPos(glfwWindow, &x, &y); }

  bool shouldClose() const override { return glfwWindowShouldClose(glfwWindow); }

  bool isMouseButtonPressed(int button) const override { return glfwGetMouseButton(glfwWindow, button) == GLFW_PRESS; }

  bool isKeyPressed(int key) const override { return glfwGetKey(glfwWindow, key) == GLFW_PRESS; }

  int getModifiers() const override {
    int mods = 0;
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
      mods |= bombfork::prong::events::MOD_SHIFT;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
      mods |= bombfork::prong::events::MOD_CONTROL;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
      mods |= bombfork::prong::events::MOD_ALT;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS) {
      mods |= bombfork::prong::events::MOD_SUPER;
    }
    return mods;
  }

  void setCallbacks(const bombfork::prong::events::WindowCallbacks& cbs) override {
    callbacks = cbs;

    // If callbacks are empty, unregister GLFW callbacks
    if (!callbacks.mouseButton && !callbacks.cursorPos && !callbacks.scroll && !callbacks.key && !callbacks.character &&
        !callbacks.framebufferSize) {
      unregisterGLFWCallbacks();
    } else {
      registerGLFWCallbacks();
    }
  }

private:
  void registerGLFWCallbacks() {
    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow* window, int button, int action, int mods) {
      auto* adapter = static_cast<GLFWWindowAdapter*>(glfwGetWindowUserPointer(window));
      if (adapter && adapter->callbacks.mouseButton) {
        // Convert GLFW constants to Prong constants
        int prongAction = (action == GLFW_PRESS)     ? bombfork::prong::events::INPUT_PRESS
                          : (action == GLFW_RELEASE) ? bombfork::prong::events::INPUT_RELEASE
                                                     : bombfork::prong::events::INPUT_REPEAT;
        adapter->callbacks.mouseButton(button, prongAction, mods);
      }
    });

    glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow* window, double x, double y) {
      auto* adapter = static_cast<GLFWWindowAdapter*>(glfwGetWindowUserPointer(window));
      if (adapter && adapter->callbacks.cursorPos) {
        adapter->callbacks.cursorPos(x, y);
      }
    });

    glfwSetScrollCallback(glfwWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
      auto* adapter = static_cast<GLFWWindowAdapter*>(glfwGetWindowUserPointer(window));
      if (adapter && adapter->callbacks.scroll) {
        adapter->callbacks.scroll(xoffset, yoffset);
      }
    });

    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
      auto* adapter = static_cast<GLFWWindowAdapter*>(glfwGetWindowUserPointer(window));
      if (adapter && adapter->callbacks.key) {
        // Convert GLFW constants to Prong constants
        int prongAction = (action == GLFW_PRESS)     ? bombfork::prong::events::INPUT_PRESS
                          : (action == GLFW_RELEASE) ? bombfork::prong::events::INPUT_RELEASE
                                                     : bombfork::prong::events::INPUT_REPEAT;
        adapter->callbacks.key(key, scancode, prongAction, mods);
      }
    });

    glfwSetCharCallback(glfwWindow, [](GLFWwindow* window, unsigned int codepoint) {
      auto* adapter = static_cast<GLFWWindowAdapter*>(glfwGetWindowUserPointer(window));
      if (adapter && adapter->callbacks.character) {
        adapter->callbacks.character(codepoint);
      }
    });

    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height) {
      auto* adapter = static_cast<GLFWWindowAdapter*>(glfwGetWindowUserPointer(window));
      if (adapter && adapter->callbacks.framebufferSize) {
        adapter->callbacks.framebufferSize(width, height);
      }
    });
  }

  void unregisterGLFWCallbacks() {
    glfwSetMouseButtonCallback(glfwWindow, nullptr);
    glfwSetCursorPosCallback(glfwWindow, nullptr);
    glfwSetScrollCallback(glfwWindow, nullptr);
    glfwSetKeyCallback(glfwWindow, nullptr);
    glfwSetCharCallback(glfwWindow, nullptr);
    glfwSetFramebufferSizeCallback(glfwWindow, nullptr);
  }
};

} // namespace bombfork::prong::examples
