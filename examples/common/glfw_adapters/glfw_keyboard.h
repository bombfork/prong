#pragma once

#include <bombfork/prong/events/ikeyboard.h>

#include <cstdint>
#include <unordered_map>

namespace bombfork::prong::examples::glfw {

/**
 * @brief GLFW implementation of the IKeyboard interface
 *
 * This adapter provides key code conversion between GLFW and Prong's
 * platform-agnostic key code system. It maintains bidirectional mappings
 * for efficient conversion in both directions.
 */
class GLFWKeyboard : public events::IKeyboard {
public:
  /**
   * @brief Default constructor
   */
  GLFWKeyboard() = default;

  /**
   * @brief Convert GLFW key code to Prong key code
   * @param platformKey GLFW key code (GLFW_KEY_*)
   * @return Prong Key enum value
   */
  events::Key toProngKey(int platformKey) const override;

  /**
   * @brief Convert Prong key code to GLFW key code
   * @param key Prong Key enum value
   * @return GLFW key code (GLFW_KEY_*)
   */
  int fromProngKey(events::Key key) const override;

  /**
   * @brief Convert GLFW modifier flags to Prong modifier flags
   * @param glfwMods GLFW modifier bitfield (GLFW_MOD_*)
   * @return Prong KeyModifier bitfield
   */
  static uint8_t convertModifiers(int glfwMods);

  /**
   * @brief Convert GLFW action to Prong action
   * @param glfwAction GLFW action (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT)
   * @return Prong KeyAction enum value
   */
  static events::KeyAction convertAction(int glfwAction);

private:
  /// Map from GLFW key codes to Prong key codes
  static const std::unordered_map<int, events::Key> glfwToProngMap_;

  /// Map from Prong key codes to GLFW key codes
  static const std::unordered_map<events::Key, int> prongToGlfwMap_;
};

} // namespace bombfork::prong::examples::glfw
