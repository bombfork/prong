#pragma once

#include <cstdint>
#include <optional>

namespace t4c::editor::gui::layout {

enum class SizePolicy {
  FIXED,         // Component has a predetermined, unchanging size
  CONTENT_BASED, // Size adapts to its contents
  EXPAND,        // Takes up all available space in its container
  PROPORTIONAL   // Size is proportional to its container
};

enum class LayoutAlignment {
  START,  // Aligned to the start of the container
  CENTER, // Centered within the container
  END,    // Aligned to the end of the container
  STRETCH // Fills the entire container
};

struct LayoutConstraints {
  // Size policies for width and height
  SizePolicy widthPolicy = SizePolicy::CONTENT_BASED;
  SizePolicy heightPolicy = SizePolicy::CONTENT_BASED;

  // Alignment within the container
  LayoutAlignment horizontalAlignment = LayoutAlignment::START;
  LayoutAlignment verticalAlignment = LayoutAlignment::START;

  // Size boundaries
  std::optional<int32_t> minWidth;
  std::optional<int32_t> maxWidth;
  std::optional<int32_t> minHeight;
  std::optional<int32_t> maxHeight;

  // Proportional sizing (for containers that support it)
  float widthRatio = 1.0f;
  float heightRatio = 1.0f;

  // Margins (left, top, right, bottom)
  int32_t margins[4] = {0, 0, 0, 0};

  // Convenience setters for margins
  void setMargins(int32_t left, int32_t top, int32_t right, int32_t bottom) {
    margins[0] = left;
    margins[1] = top;
    margins[2] = right;
    margins[3] = bottom;
  }
};

} // namespace t4c::editor::gui::layout