#pragma once

#include <bombfork/prong/theming/color.h>

#include <string>

namespace bombfork::prong {

// Use the canonical Color from the UI theming system
using Color = bombfork::prong::theming::Color;

/**
 * @brief Spacing/padding specification
 */
struct Padding {
  int top = 0, right = 0, bottom = 0, left = 0;

  constexpr Padding() = default;
  constexpr Padding(int uniform) : top(uniform), right(uniform), bottom(uniform), left(uniform) {}
  constexpr Padding(int vertical, int horizontal)
    : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
  constexpr Padding(int top, int right, int bottom, int left) : top(top), right(right), bottom(bottom), left(left) {}
};

/**
 * @brief Border specification
 */
struct Border {
  int width = 0;
  Color color = Color::BLACK();

  constexpr Border() = default;
  constexpr Border(int width, Color color = Color::BLACK()) : width(width), color(color) {}
};

} // namespace bombfork::prong
