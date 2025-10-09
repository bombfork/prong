#pragma once

#include <cstdint>
#include <optional>

namespace bombfork::prong::layout {

// Represents a single dimension measurement (width or height)
struct Measurement {
  int32_t size; // The actual measured size
  bool isExact; // Whether the size is an exact measurement or an approximation

  // Constructors
  Measurement() : size(0), isExact(false) {}
  explicit Measurement(int32_t s, bool exact = true) : size(s), isExact(exact) {}

  // Implicit conversion to size (returns size)
  operator int32_t() const { return size; }
};

// Represents the complete layout measurement for a component
struct LayoutMeasurement {
  Measurement width;               // Width measurement
  Measurement height;              // Height measurement
  std::optional<int32_t> baseline; // Optional baseline for text components

  // Default constructor
  LayoutMeasurement() = default;

  // Constructors
  LayoutMeasurement(Measurement w, Measurement h, std::optional<int32_t> base = std::nullopt)
    : width(w), height(h), baseline(base) {}

  // Check if the measurement is valid
  bool isValid() const { return width.size >= 0 && height.size >= 0; }

  // Static factory method for creating exact measurements
  static LayoutMeasurement exact(int32_t width, int32_t height, std::optional<int32_t> baseline = std::nullopt) {
    return LayoutMeasurement(Measurement(width, true), Measurement(height, true), baseline);
  }

  // Static factory method for creating approximate measurements
  static LayoutMeasurement approximate(int32_t width, int32_t height, std::optional<int32_t> baseline = std::nullopt) {
    return LayoutMeasurement(Measurement(width, false), Measurement(height, false), baseline);
  }
};

} // namespace bombfork::prong::layout