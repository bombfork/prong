#pragma once

namespace bombfork::prong::core {

/**
 * @brief Unified event structure for hierarchical event handling
 *
 * This structure represents all types of input events in the Prong UI framework.
 * It uses a type-discriminated union pattern where the Type enum determines
 * which fields are relevant for each event.
 *
 * Events contain coordinates in LOCAL space relative to the component receiving
 * the event. Component::handleEvent() automatically converts global screen
 * coordinates to local coordinates during propagation.
 *
 * Usage with C++20 designated initializers:
 * @code
 *   Event mousePress {
 *     .type = Event::Type::MOUSE_PRESS,
 *     .localX = 10,
 *     .localY = 20,
 *     .button = 0
 *   };
 *
 *   Event keyPress {
 *     .type = Event::Type::KEY_PRESS,
 *     .key = static_cast<int>(Key::A),
 *     .mods = CTRL
 *   };
 * @endcode
 */
struct Event {
  /**
   * @brief Event type discriminator
   */
  enum class Type {
    MOUSE_PRESS,   ///< Mouse button pressed
    MOUSE_RELEASE, ///< Mouse button released
    MOUSE_MOVE,    ///< Mouse cursor moved
    MOUSE_SCROLL,  ///< Mouse wheel scrolled
    KEY_PRESS,     ///< Keyboard key pressed
    KEY_RELEASE,   ///< Keyboard key released
    CHAR_INPUT     ///< Character input (for text entry)
  };

  // Event type
  Type type;

  // Positional data (for MOUSE_* events)
  // These are in LOCAL space relative to the component receiving the event
  int localX = 0;
  int localY = 0;

  // Mouse-specific data (for MOUSE_PRESS, MOUSE_RELEASE)
  int button = 0; ///< Mouse button index (0=left, 1=right, 2=middle)

  // Keyboard-specific data (for KEY_PRESS, KEY_RELEASE)
  int key = 0;  ///< Platform-agnostic key code
  int mods = 0; ///< Modifier key flags (CTRL, SHIFT, ALT, etc.)

  // Character input data (for CHAR_INPUT)
  unsigned int codepoint = 0; ///< Unicode codepoint for text input

  // Scroll-specific data (for MOUSE_SCROLL)
  double scrollX = 0.0; ///< Horizontal scroll delta (positive = right)
  double scrollY = 0.0; ///< Vertical scroll delta (positive = down)
};

} // namespace bombfork::prong::core
