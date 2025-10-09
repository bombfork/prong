#pragma once

#include <functional>

namespace bombfork::prong::events {

// === Platform-agnostic Input Constants ===

// Mouse button codes
constexpr int MOUSE_BUTTON_LEFT = 0;
constexpr int MOUSE_BUTTON_RIGHT = 1;
constexpr int MOUSE_BUTTON_MIDDLE = 2;

// Input action codes
constexpr int INPUT_RELEASE = 0;
constexpr int INPUT_PRESS = 1;
constexpr int INPUT_REPEAT = 2;

// Modifier key bits
constexpr int MOD_SHIFT = 0x0001;
constexpr int MOD_CONTROL = 0x0002;
constexpr int MOD_ALT = 0x0004;
constexpr int MOD_SUPER = 0x0008;

/**
 * @brief Callback functions for window events
 *
 * These callbacks allow the window implementation to notify the event
 * system when input events occur. The implementation should register
 * these with the underlying windowing system (GLFW, SDL, etc.)
 */
struct WindowCallbacks {
  std::function<void(int button, int action, int mods)> mouseButton;
  std::function<void(double x, double y)> cursorPos;
  std::function<void(double xoffset, double yoffset)> scroll;
  std::function<void(int key, int scancode, int action, int mods)> key;
  std::function<void(unsigned int codepoint)> character;
  std::function<void(int width, int height)> framebufferSize;
};

/**
 * @brief Abstract window interface for platform/library independence
 *
 * Provides minimal window functionality needed by the event system.
 * This abstraction allows Prong to work with any windowing library
 * (GLFW, SDL, native OS APIs, etc.)
 *
 * Design principles:
 * - Minimal interface: only what event system needs
 * - Backend agnostic: no library-specific types
 * - Query-based: event system polls for state
 */
class IWindow {
public:
  virtual ~IWindow() = default;

  // === Window Properties ===

  /**
   * @brief Get window dimensions
   * @param width Output: window width in pixels
   * @param height Output: window height in pixels
   */
  virtual void getSize(int& width, int& height) const = 0;

  /**
   * @brief Get native window handle
   * @return Platform-specific window handle (e.g., GLFWwindow*, HWND, etc.)
   *
   * This allows implementations to access library-specific features
   * while keeping the main interface clean.
   */
  virtual void* getNativeHandle() = 0;

  // === Input State ===

  /**
   * @brief Get current cursor position
   * @param x Output: cursor X position in window coordinates
   * @param y Output: cursor Y position in window coordinates
   */
  virtual void getCursorPos(double& x, double& y) const = 0;

  /**
   * @brief Check if window should close
   * @return true if user requested window close
   */
  virtual bool shouldClose() const = 0;

  // === Input Queries (for event filtering) ===

  /**
   * @brief Get mouse button state
   * @param button Button code (0 = left, 1 = right, 2 = middle)
   * @return true if button is currently pressed
   */
  virtual bool isMouseButtonPressed(int button) const = 0;

  /**
   * @brief Get keyboard key state
   * @param key Key code (platform-specific, but typically ASCII or scan codes)
   * @return true if key is currently pressed
   */
  virtual bool isKeyPressed(int key) const = 0;

  /**
   * @brief Get keyboard modifier state
   * @return Bitmask of active modifiers (shift, ctrl, alt, super)
   *
   * Bit values (can be OR'd together):
   * - 0x0001: Shift
   * - 0x0002: Control
   * - 0x0004: Alt
   * - 0x0008: Super/Windows/Command
   */
  virtual int getModifiers() const = 0;

  // === Event Callbacks ===

  /**
   * @brief Register event callbacks with the window
   *
   * The implementation should register these callbacks with the underlying
   * windowing system (GLFW, SDL, etc.) so that they are called when
   * input events occur.
   *
   * @param callbacks Struct containing callback functions for various events
   */
  virtual void setCallbacks(const WindowCallbacks& callbacks) = 0;
};

} // namespace bombfork::prong::events
