#pragma once

#include <cstdint>

namespace bombfork::prong::events {

/**
 * @brief Platform-agnostic key codes
 */
enum class Key : int32_t {
  UNKNOWN = -1,

  // Arrow keys
  LEFT,
  RIGHT,
  UP,
  DOWN,

  // Navigation
  HOME,
  END,
  PAGE_UP,
  PAGE_DOWN,

  // Editing
  BACKSPACE,
  DELETE,
  INSERT,
  ENTER,
  TAB,
  ESCAPE,

  // Modifiers (for key events, not just modifier state)
  LEFT_SHIFT,
  RIGHT_SHIFT,
  LEFT_CONTROL,
  RIGHT_CONTROL,
  LEFT_ALT,
  RIGHT_ALT,
  LEFT_SUPER,
  RIGHT_SUPER,

  // Letters A-Z
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,

  // Numbers 0-9
  NUM_0,
  NUM_1,
  NUM_2,
  NUM_3,
  NUM_4,
  NUM_5,
  NUM_6,
  NUM_7,
  NUM_8,
  NUM_9,

  // Function keys
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  // Numpad
  KP_0,
  KP_1,
  KP_2,
  KP_3,
  KP_4,
  KP_5,
  KP_6,
  KP_7,
  KP_8,
  KP_9,
  KP_DECIMAL,
  KP_DIVIDE,
  KP_MULTIPLY,
  KP_SUBTRACT,
  KP_ADD,
  KP_ENTER,

  // Symbols (commonly used)
  SPACE,
  APOSTROPHE,
  COMMA,
  MINUS,
  PERIOD,
  SLASH,
  SEMICOLON,
  EQUAL,
  LEFT_BRACKET,
  BACKSLASH,
  RIGHT_BRACKET,
  GRAVE_ACCENT
};

/**
 * @brief Keyboard modifier flags
 */
enum class KeyModifier : uint8_t {
  NONE = 0,
  SHIFT = 1 << 0,
  CONTROL = 1 << 1,
  ALT = 1 << 2,
  SUPER = 1 << 3,
  CAPS_LOCK = 1 << 4,
  NUM_LOCK = 1 << 5
};

/**
 * @brief Key action types
 */
enum class KeyAction { RELEASE = 0, PRESS = 1, REPEAT = 2 };

/**
 * @brief Abstract keyboard interface for platform-agnostic keyboard input
 *
 * This interface provides utilities for converting between platform-specific
 * key codes (GLFW, SDL, etc.) and Prong's agnostic key codes.
 */
class IKeyboard {
public:
  virtual ~IKeyboard() = default;

  /**
   * @brief Convert platform-specific key code to Prong key code
   * @param platformKey Platform-specific key code
   * @return Prong Key enum value
   */
  virtual Key toProngKey(int platformKey) const = 0;

  /**
   * @brief Convert Prong key code to platform-specific key code
   * @param key Prong Key enum value
   * @return Platform-specific key code
   */
  virtual int fromProngKey(Key key) const = 0;

  /**
   * @brief Check if a modifier is active
   * @param mods Modifier bitfield
   * @param modifier Modifier to check
   * @return True if modifier is active
   */
  static bool hasModifier(uint8_t mods, KeyModifier modifier) { return (mods & static_cast<uint8_t>(modifier)) != 0; }
};

// Bitwise operators for KeyModifier
inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
  return static_cast<KeyModifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline KeyModifier operator&(KeyModifier a, KeyModifier b) {
  return static_cast<KeyModifier>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

} // namespace bombfork::prong::events
