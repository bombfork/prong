#pragma once

#include <algorithm>

namespace bombfork::prong::theming {

/**
 * @brief Color specification for UI theming
 *
 * This is the canonical Color class for the new UI system.
 * Replaces the legacy t4c::client::files::Color from component_traits.h
 */
struct Color {
  float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

  constexpr Color() = default;
  constexpr Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

  // Equality operator
  constexpr bool operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }

  constexpr bool operator!=(const Color& other) const { return !(*this == other); }

  // Common colors
  static constexpr Color WHITE() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
  static constexpr Color BLACK() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
  static constexpr Color TRANSPARENT() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
  static constexpr Color GRAY() { return {0.5f, 0.5f, 0.5f, 1.0f}; }
  static constexpr Color LIGHT_GRAY() { return {0.8f, 0.8f, 0.8f, 1.0f}; }
  static constexpr Color DARK_GRAY() { return {0.3f, 0.3f, 0.3f, 1.0f}; }
  static constexpr Color BLUE() { return {0.3f, 0.5f, 0.8f, 1.0f}; }
  static constexpr Color GREEN() { return {0.3f, 0.8f, 0.3f, 1.0f}; }
  static constexpr Color RED() { return {0.8f, 0.3f, 0.3f, 1.0f}; }
  static constexpr Color YELLOW() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
  static constexpr Color CYAN() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
  static constexpr Color MAGENTA() { return {1.0f, 0.0f, 1.0f, 1.0f}; }

  /**
   * @brief Blend this color with another using alpha compositing
   */
  constexpr Color blend(const Color& other, float t) const {
    return {r + (other.r - r) * t, g + (other.g - g) * t, b + (other.b - b) * t, a + (other.a - a) * t};
  }

  /**
   * @brief Adjust brightness by adding/subtracting a value
   */
  constexpr Color adjustBrightness(float amount) const {
    return {std::min(1.0f, std::max(0.0f, r + amount)), std::min(1.0f, std::max(0.0f, g + amount)),
            std::min(1.0f, std::max(0.0f, b + amount)), a};
  }

  /**
   * @brief Multiply brightness by a factor
   */
  constexpr Color adjustBrightnessFactor(float factor) const {
    return {std::min(1.0f, r * factor), std::min(1.0f, g * factor), std::min(1.0f, b * factor), a};
  }

  /**
   * @brief Adjust alpha transparency
   */
  constexpr Color withAlpha(float newAlpha) const { return {r, g, b, newAlpha}; }
};

} // namespace bombfork::prong::theming