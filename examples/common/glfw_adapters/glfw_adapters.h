#pragma once

#include "glfw_clipboard.h"
#include "glfw_keyboard.h"
#include <GLFW/glfw3.h>

#include <memory>

namespace bombfork::prong::examples::glfw {

/**
 * @brief Factory struct for creating GLFW adapters
 *
 * This struct provides a convenient way to create all GLFW adapters
 * (clipboard and keyboard) in one call, ensuring they all reference
 * the same GLFW window.
 */
struct GLFWAdapters {
  /// Clipboard adapter instance
  std::unique_ptr<GLFWClipboard> clipboard;

  /// Keyboard adapter instance
  std::unique_ptr<GLFWKeyboard> keyboard;

  /**
   * @brief Create GLFW adapters for the given window
   * @param window Pointer to the GLFW window (must not be null)
   * @return GLFWAdapters struct with initialized adapters
   */
  static GLFWAdapters create(GLFWwindow* window) {
    GLFWAdapters adapters;
    adapters.clipboard = std::make_unique<GLFWClipboard>(window);
    adapters.keyboard = std::make_unique<GLFWKeyboard>();
    return adapters;
  }
};

} // namespace bombfork::prong::examples::glfw
