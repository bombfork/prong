#include "glfw_keyboard.h"

#include <GLFW/glfw3.h>
#include <bombfork/prong/events/ikeyboard.h>

#include <cstdint>
#include <unordered_map>
#include <utility>

namespace bombfork::prong::examples::glfw {

// Initialize GLFW to Prong key mapping
const std::unordered_map<int, events::Key> GLFWKeyboard::glfwToProngMap_ = {
  // Arrow keys
  {GLFW_KEY_LEFT, events::Key::LEFT},
  {GLFW_KEY_RIGHT, events::Key::RIGHT},
  {GLFW_KEY_UP, events::Key::UP},
  {GLFW_KEY_DOWN, events::Key::DOWN},

  // Navigation
  {GLFW_KEY_HOME, events::Key::HOME},
  {GLFW_KEY_END, events::Key::END},
  {GLFW_KEY_PAGE_UP, events::Key::PAGE_UP},
  {GLFW_KEY_PAGE_DOWN, events::Key::PAGE_DOWN},

  // Editing
  {GLFW_KEY_BACKSPACE, events::Key::BACKSPACE},
  {GLFW_KEY_DELETE, events::Key::DELETE},
  {GLFW_KEY_INSERT, events::Key::INSERT},
  {GLFW_KEY_ENTER, events::Key::ENTER},
  {GLFW_KEY_TAB, events::Key::TAB},
  {GLFW_KEY_ESCAPE, events::Key::ESCAPE},

  // Modifiers
  {GLFW_KEY_LEFT_SHIFT, events::Key::LEFT_SHIFT},
  {GLFW_KEY_RIGHT_SHIFT, events::Key::RIGHT_SHIFT},
  {GLFW_KEY_LEFT_CONTROL, events::Key::LEFT_CONTROL},
  {GLFW_KEY_RIGHT_CONTROL, events::Key::RIGHT_CONTROL},
  {GLFW_KEY_LEFT_ALT, events::Key::LEFT_ALT},
  {GLFW_KEY_RIGHT_ALT, events::Key::RIGHT_ALT},
  {GLFW_KEY_LEFT_SUPER, events::Key::LEFT_SUPER},
  {GLFW_KEY_RIGHT_SUPER, events::Key::RIGHT_SUPER},

  // Letters A-Z
  {GLFW_KEY_A, events::Key::A},
  {GLFW_KEY_B, events::Key::B},
  {GLFW_KEY_C, events::Key::C},
  {GLFW_KEY_D, events::Key::D},
  {GLFW_KEY_E, events::Key::E},
  {GLFW_KEY_F, events::Key::F},
  {GLFW_KEY_G, events::Key::G},
  {GLFW_KEY_H, events::Key::H},
  {GLFW_KEY_I, events::Key::I},
  {GLFW_KEY_J, events::Key::J},
  {GLFW_KEY_K, events::Key::K},
  {GLFW_KEY_L, events::Key::L},
  {GLFW_KEY_M, events::Key::M},
  {GLFW_KEY_N, events::Key::N},
  {GLFW_KEY_O, events::Key::O},
  {GLFW_KEY_P, events::Key::P},
  {GLFW_KEY_Q, events::Key::Q},
  {GLFW_KEY_R, events::Key::R},
  {GLFW_KEY_S, events::Key::S},
  {GLFW_KEY_T, events::Key::T},
  {GLFW_KEY_U, events::Key::U},
  {GLFW_KEY_V, events::Key::V},
  {GLFW_KEY_W, events::Key::W},
  {GLFW_KEY_X, events::Key::X},
  {GLFW_KEY_Y, events::Key::Y},
  {GLFW_KEY_Z, events::Key::Z},

  // Numbers 0-9
  {GLFW_KEY_0, events::Key::NUM_0},
  {GLFW_KEY_1, events::Key::NUM_1},
  {GLFW_KEY_2, events::Key::NUM_2},
  {GLFW_KEY_3, events::Key::NUM_3},
  {GLFW_KEY_4, events::Key::NUM_4},
  {GLFW_KEY_5, events::Key::NUM_5},
  {GLFW_KEY_6, events::Key::NUM_6},
  {GLFW_KEY_7, events::Key::NUM_7},
  {GLFW_KEY_8, events::Key::NUM_8},
  {GLFW_KEY_9, events::Key::NUM_9},

  // Function keys
  {GLFW_KEY_F1, events::Key::F1},
  {GLFW_KEY_F2, events::Key::F2},
  {GLFW_KEY_F3, events::Key::F3},
  {GLFW_KEY_F4, events::Key::F4},
  {GLFW_KEY_F5, events::Key::F5},
  {GLFW_KEY_F6, events::Key::F6},
  {GLFW_KEY_F7, events::Key::F7},
  {GLFW_KEY_F8, events::Key::F8},
  {GLFW_KEY_F9, events::Key::F9},
  {GLFW_KEY_F10, events::Key::F10},
  {GLFW_KEY_F11, events::Key::F11},
  {GLFW_KEY_F12, events::Key::F12},

  // Numpad
  {GLFW_KEY_KP_0, events::Key::KP_0},
  {GLFW_KEY_KP_1, events::Key::KP_1},
  {GLFW_KEY_KP_2, events::Key::KP_2},
  {GLFW_KEY_KP_3, events::Key::KP_3},
  {GLFW_KEY_KP_4, events::Key::KP_4},
  {GLFW_KEY_KP_5, events::Key::KP_5},
  {GLFW_KEY_KP_6, events::Key::KP_6},
  {GLFW_KEY_KP_7, events::Key::KP_7},
  {GLFW_KEY_KP_8, events::Key::KP_8},
  {GLFW_KEY_KP_9, events::Key::KP_9},
  {GLFW_KEY_KP_DECIMAL, events::Key::KP_DECIMAL},
  {GLFW_KEY_KP_DIVIDE, events::Key::KP_DIVIDE},
  {GLFW_KEY_KP_MULTIPLY, events::Key::KP_MULTIPLY},
  {GLFW_KEY_KP_SUBTRACT, events::Key::KP_SUBTRACT},
  {GLFW_KEY_KP_ADD, events::Key::KP_ADD},
  {GLFW_KEY_KP_ENTER, events::Key::KP_ENTER},

  // Symbols
  {GLFW_KEY_SPACE, events::Key::SPACE},
  {GLFW_KEY_APOSTROPHE, events::Key::APOSTROPHE},
  {GLFW_KEY_COMMA, events::Key::COMMA},
  {GLFW_KEY_MINUS, events::Key::MINUS},
  {GLFW_KEY_PERIOD, events::Key::PERIOD},
  {GLFW_KEY_SLASH, events::Key::SLASH},
  {GLFW_KEY_SEMICOLON, events::Key::SEMICOLON},
  {GLFW_KEY_EQUAL, events::Key::EQUAL},
  {GLFW_KEY_LEFT_BRACKET, events::Key::LEFT_BRACKET},
  {GLFW_KEY_BACKSLASH, events::Key::BACKSLASH},
  {GLFW_KEY_RIGHT_BRACKET, events::Key::RIGHT_BRACKET},
  {GLFW_KEY_GRAVE_ACCENT, events::Key::GRAVE_ACCENT},

  // Unknown key
  {GLFW_KEY_UNKNOWN, events::Key::UNKNOWN},
};

// Initialize Prong to GLFW key mapping (reverse map)
const std::unordered_map<events::Key, int> GLFWKeyboard::prongToGlfwMap_ = {
  // Arrow keys
  {events::Key::LEFT, GLFW_KEY_LEFT},
  {events::Key::RIGHT, GLFW_KEY_RIGHT},
  {events::Key::UP, GLFW_KEY_UP},
  {events::Key::DOWN, GLFW_KEY_DOWN},

  // Navigation
  {events::Key::HOME, GLFW_KEY_HOME},
  {events::Key::END, GLFW_KEY_END},
  {events::Key::PAGE_UP, GLFW_KEY_PAGE_UP},
  {events::Key::PAGE_DOWN, GLFW_KEY_PAGE_DOWN},

  // Editing
  {events::Key::BACKSPACE, GLFW_KEY_BACKSPACE},
  {events::Key::DELETE, GLFW_KEY_DELETE},
  {events::Key::INSERT, GLFW_KEY_INSERT},
  {events::Key::ENTER, GLFW_KEY_ENTER},
  {events::Key::TAB, GLFW_KEY_TAB},
  {events::Key::ESCAPE, GLFW_KEY_ESCAPE},

  // Modifiers
  {events::Key::LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT},
  {events::Key::RIGHT_SHIFT, GLFW_KEY_RIGHT_SHIFT},
  {events::Key::LEFT_CONTROL, GLFW_KEY_LEFT_CONTROL},
  {events::Key::RIGHT_CONTROL, GLFW_KEY_RIGHT_CONTROL},
  {events::Key::LEFT_ALT, GLFW_KEY_LEFT_ALT},
  {events::Key::RIGHT_ALT, GLFW_KEY_RIGHT_ALT},
  {events::Key::LEFT_SUPER, GLFW_KEY_LEFT_SUPER},
  {events::Key::RIGHT_SUPER, GLFW_KEY_RIGHT_SUPER},

  // Letters A-Z
  {events::Key::A, GLFW_KEY_A},
  {events::Key::B, GLFW_KEY_B},
  {events::Key::C, GLFW_KEY_C},
  {events::Key::D, GLFW_KEY_D},
  {events::Key::E, GLFW_KEY_E},
  {events::Key::F, GLFW_KEY_F},
  {events::Key::G, GLFW_KEY_G},
  {events::Key::H, GLFW_KEY_H},
  {events::Key::I, GLFW_KEY_I},
  {events::Key::J, GLFW_KEY_J},
  {events::Key::K, GLFW_KEY_K},
  {events::Key::L, GLFW_KEY_L},
  {events::Key::M, GLFW_KEY_M},
  {events::Key::N, GLFW_KEY_N},
  {events::Key::O, GLFW_KEY_O},
  {events::Key::P, GLFW_KEY_P},
  {events::Key::Q, GLFW_KEY_Q},
  {events::Key::R, GLFW_KEY_R},
  {events::Key::S, GLFW_KEY_S},
  {events::Key::T, GLFW_KEY_T},
  {events::Key::U, GLFW_KEY_U},
  {events::Key::V, GLFW_KEY_V},
  {events::Key::W, GLFW_KEY_W},
  {events::Key::X, GLFW_KEY_X},
  {events::Key::Y, GLFW_KEY_Y},
  {events::Key::Z, GLFW_KEY_Z},

  // Numbers 0-9
  {events::Key::NUM_0, GLFW_KEY_0},
  {events::Key::NUM_1, GLFW_KEY_1},
  {events::Key::NUM_2, GLFW_KEY_2},
  {events::Key::NUM_3, GLFW_KEY_3},
  {events::Key::NUM_4, GLFW_KEY_4},
  {events::Key::NUM_5, GLFW_KEY_5},
  {events::Key::NUM_6, GLFW_KEY_6},
  {events::Key::NUM_7, GLFW_KEY_7},
  {events::Key::NUM_8, GLFW_KEY_8},
  {events::Key::NUM_9, GLFW_KEY_9},

  // Function keys
  {events::Key::F1, GLFW_KEY_F1},
  {events::Key::F2, GLFW_KEY_F2},
  {events::Key::F3, GLFW_KEY_F3},
  {events::Key::F4, GLFW_KEY_F4},
  {events::Key::F5, GLFW_KEY_F5},
  {events::Key::F6, GLFW_KEY_F6},
  {events::Key::F7, GLFW_KEY_F7},
  {events::Key::F8, GLFW_KEY_F8},
  {events::Key::F9, GLFW_KEY_F9},
  {events::Key::F10, GLFW_KEY_F10},
  {events::Key::F11, GLFW_KEY_F11},
  {events::Key::F12, GLFW_KEY_F12},

  // Numpad
  {events::Key::KP_0, GLFW_KEY_KP_0},
  {events::Key::KP_1, GLFW_KEY_KP_1},
  {events::Key::KP_2, GLFW_KEY_KP_2},
  {events::Key::KP_3, GLFW_KEY_KP_3},
  {events::Key::KP_4, GLFW_KEY_KP_4},
  {events::Key::KP_5, GLFW_KEY_KP_5},
  {events::Key::KP_6, GLFW_KEY_KP_6},
  {events::Key::KP_7, GLFW_KEY_KP_7},
  {events::Key::KP_8, GLFW_KEY_KP_8},
  {events::Key::KP_9, GLFW_KEY_KP_9},
  {events::Key::KP_DECIMAL, GLFW_KEY_KP_DECIMAL},
  {events::Key::KP_DIVIDE, GLFW_KEY_KP_DIVIDE},
  {events::Key::KP_MULTIPLY, GLFW_KEY_KP_MULTIPLY},
  {events::Key::KP_SUBTRACT, GLFW_KEY_KP_SUBTRACT},
  {events::Key::KP_ADD, GLFW_KEY_KP_ADD},
  {events::Key::KP_ENTER, GLFW_KEY_KP_ENTER},

  // Symbols
  {events::Key::SPACE, GLFW_KEY_SPACE},
  {events::Key::APOSTROPHE, GLFW_KEY_APOSTROPHE},
  {events::Key::COMMA, GLFW_KEY_COMMA},
  {events::Key::MINUS, GLFW_KEY_MINUS},
  {events::Key::PERIOD, GLFW_KEY_PERIOD},
  {events::Key::SLASH, GLFW_KEY_SLASH},
  {events::Key::SEMICOLON, GLFW_KEY_SEMICOLON},
  {events::Key::EQUAL, GLFW_KEY_EQUAL},
  {events::Key::LEFT_BRACKET, GLFW_KEY_LEFT_BRACKET},
  {events::Key::BACKSLASH, GLFW_KEY_BACKSLASH},
  {events::Key::RIGHT_BRACKET, GLFW_KEY_RIGHT_BRACKET},
  {events::Key::GRAVE_ACCENT, GLFW_KEY_GRAVE_ACCENT},

  // Unknown key
  {events::Key::UNKNOWN, GLFW_KEY_UNKNOWN},
};

events::Key GLFWKeyboard::toProngKey(int platformKey) const {
  auto it = glfwToProngMap_.find(platformKey);
  if (it != glfwToProngMap_.end()) {
    return it->second;
  }
  return events::Key::UNKNOWN;
}

int GLFWKeyboard::fromProngKey(events::Key key) const {
  auto it = prongToGlfwMap_.find(key);
  if (it != prongToGlfwMap_.end()) {
    return it->second;
  }
  return GLFW_KEY_UNKNOWN;
}

uint8_t GLFWKeyboard::convertModifiers(int glfwMods) {
  uint8_t mods = static_cast<uint8_t>(events::KeyModifier::NONE);

  if (glfwMods & GLFW_MOD_SHIFT) {
    mods |= static_cast<uint8_t>(events::KeyModifier::SHIFT);
  }
  if (glfwMods & GLFW_MOD_CONTROL) {
    mods |= static_cast<uint8_t>(events::KeyModifier::CONTROL);
  }
  if (glfwMods & GLFW_MOD_ALT) {
    mods |= static_cast<uint8_t>(events::KeyModifier::ALT);
  }
  if (glfwMods & GLFW_MOD_SUPER) {
    mods |= static_cast<uint8_t>(events::KeyModifier::SUPER);
  }
  if (glfwMods & GLFW_MOD_CAPS_LOCK) {
    mods |= static_cast<uint8_t>(events::KeyModifier::CAPS_LOCK);
  }
  if (glfwMods & GLFW_MOD_NUM_LOCK) {
    mods |= static_cast<uint8_t>(events::KeyModifier::NUM_LOCK);
  }

  return mods;
}

events::KeyAction GLFWKeyboard::convertAction(int glfwAction) {
  switch (glfwAction) {
  case GLFW_PRESS:
    return events::KeyAction::PRESS;
  case GLFW_RELEASE:
    return events::KeyAction::RELEASE;
  case GLFW_REPEAT:
    return events::KeyAction::REPEAT;
  default:
    return events::KeyAction::RELEASE;
  }
}

} // namespace bombfork::prong::examples::glfw
