#pragma once

#include <GLFW/glfw3.h>
#include <bombfork/prong/events/iclipboard.h>

#include <string>

namespace bombfork::prong::examples::glfw {

/**
 * @brief GLFW implementation of the IClipboard interface
 *
 * This adapter provides clipboard operations using GLFW's clipboard API.
 * It requires a valid GLFWwindow pointer to access the system clipboard.
 */
class GLFWClipboard : public events::IClipboard {
public:
  /**
   * @brief Construct a new GLFWClipboard adapter
   * @param window Pointer to the GLFW window (must not be null)
   */
  explicit GLFWClipboard(GLFWwindow* window) : window_(window) {}

  /**
   * @brief Retrieve text from the system clipboard
   * @return The clipboard text content, or empty string if unavailable
   */
  std::string getString() const override {
    if (!window_) {
      return "";
    }

    const char* text = glfwGetClipboardString(window_);
    return text ? std::string(text) : "";
  }

  /**
   * @brief Set text to the system clipboard
   * @param text The text to copy to the clipboard
   */
  void setString(const std::string& text) override {
    if (window_) {
      glfwSetClipboardString(window_, text.c_str());
    }
  }

  /**
   * @brief Check if clipboard contains text
   * @return true if clipboard has text content, false otherwise
   */
  bool hasText() const override {
    if (!window_) {
      return false;
    }

    const char* text = glfwGetClipboardString(window_);
    return text != nullptr && text[0] != '\0';
  }

private:
  GLFWwindow* window_; ///< GLFW window handle for clipboard operations
};

} // namespace bombfork::prong::examples::glfw
