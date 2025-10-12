/**
 * @file flex_layout.h
 * @brief Flexbox-inspired Layout Manager for T4C Client UI Framework
 * @copyright (C) 2025 Nazagoth Interactive
 */
#pragma once

#include <bombfork/prong/core/component_traits.h>
#include <bombfork/prong/layout/layout_manager.h>

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

namespace bombfork::prong::layout {

enum class FlexDirection {
  ROW,           // Left to right (default)
  ROW_REVERSE,   // Right to left
  COLUMN,        // Top to bottom
  COLUMN_REVERSE // Bottom to top
};

enum class FlexJustify {
  START,         // Pack from start
  END,           // Pack from end
  CENTER,        // Center pack
  SPACE_BETWEEN, // Distribute space between items
  SPACE_AROUND,  // Distribute space around items
  SPACE_EVENLY   // Equal space between/around items
};

enum class FlexAlign {
  STRETCH, // Stretch to fill cross axis
  START,   // Align to start of cross axis
  END,     // Align to end of cross axis
  CENTER,  // Center on cross axis
  BASELINE // Align baselines (text-specific)
};

template <typename DerivedT>
class FlexLayoutManager : public LayoutManager<DerivedT> {
public:
  struct Configuration {
    FlexDirection direction = FlexDirection::ROW;
    FlexJustify justify = FlexJustify::START;
    FlexAlign align = FlexAlign::STRETCH;
    float gap = 0.0f;  // Space between flex items
    bool wrap = false; // Enable wrapping
  };

  struct FlexItemProperties {
    float grow = 0.0f;   // Flex grow factor
    float shrink = 1.0f; // Flex shrink factor
    float basis = 0.0f;  // Initial main axis size
  };

  FlexLayoutManager() : config_{}, itemProperties_{} {}

  DerivedT& configure(const Configuration& config) {
    config_ = config;
    return static_cast<DerivedT&>(*this);
  }

  Configuration getCurrentConfiguration() const { return config_; }

  void setItemProperties(const std::vector<FlexItemProperties>& props) { itemProperties_ = props; }

  Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& components) override {
    // Implement comprehensive flex measurement strategy
    // Consider grow/shrink factors, direction, wrapping
    return calculateFlexDimensions(components);
  }

  void layout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) override {
    // Implement flex layout algorithm
    performFlexLayout(components, availableSpace);
  }

private:
  Dimensions calculateFlexDimensions(const std::vector<bombfork::prong::Component*>& components) {
    if (components.empty())
      return {0, 0};

    float mainAxisSize = 0.0f;
    float crossAxisSize = 0.0f;
    float totalGrowFactor = 0.0f;

    for (const auto* component : components) {
      auto componentDims = component->getPreferredSize();

      if (config_.direction == FlexDirection::ROW || config_.direction == FlexDirection::ROW_REVERSE) {
        mainAxisSize += static_cast<float>(componentDims.width);
        crossAxisSize = std::max(crossAxisSize, static_cast<float>(componentDims.height));
      } else {
        mainAxisSize += static_cast<float>(componentDims.height);
        crossAxisSize = std::max(crossAxisSize, static_cast<float>(componentDims.width));
      }

      totalGrowFactor += (itemProperties_.size() > 0) ? itemProperties_[0].grow : 0.0f;
    }

    // Add gaps between items
    mainAxisSize += std::max(0.0f, config_.gap * (components.size() - 1));

    // Wrapping consideration
    if (config_.wrap) {
      // TODO: Implement more complex wrapping logic
    }

    return (config_.direction == FlexDirection::ROW || config_.direction == FlexDirection::ROW_REVERSE)
             ? Dimensions{static_cast<int>(mainAxisSize), static_cast<int>(crossAxisSize)}
             : Dimensions{static_cast<int>(crossAxisSize), static_cast<int>(mainAxisSize)};
  }

  void performFlexLayout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) {
    if (components.empty())
      return;

    // Determine layout axis and cross-axis based on direction
    bool isHorizontal = config_.direction == FlexDirection::ROW || config_.direction == FlexDirection::ROW_REVERSE;
    float mainAxisTotal = isHorizontal ? availableSpace.width : availableSpace.height;
    float crossAxisTotal = isHorizontal ? availableSpace.height : availableSpace.width;

    // Initial measurement of components
    std::vector<Dimensions> componentDimensions;
    float totalMainAxisSize = 0.0f;
    std::vector<float> growFactors;

    for (size_t i = 0; i < components.size(); ++i) {
      auto dims = components[i]->getPreferredSize();
      componentDimensions.push_back(dims);

      float componentMainAxisSize = isHorizontal ? static_cast<float>(dims.width) : static_cast<float>(dims.height);
      totalMainAxisSize += componentMainAxisSize;

      growFactors.push_back(i < itemProperties_.size() ? itemProperties_[i].grow : 0.0f);
    }

    // Add gap space
    totalMainAxisSize += config_.gap * (components.size() - 1);

    // Distribute extra space based on grow factors
    float extraSpace = std::max(0.0f, mainAxisTotal - totalMainAxisSize);
    float totalGrowFactor = std::accumulate(growFactors.begin(), growFactors.end(), 0.0f);

    // Calculate starting position based on justify content
    float currentPosition = calculateJustifyStartPosition(totalMainAxisSize, mainAxisTotal, components.size());
    for (size_t i = 0; i < components.size(); ++i) {
      auto* component = components[i];
      auto& dims = componentDimensions[i];

      // Determine main axis size
      float componentMainAxisSize = isHorizontal ? static_cast<float>(dims.width) : static_cast<float>(dims.height);
      if (totalGrowFactor > 0) {
        componentMainAxisSize += (growFactors[i] / totalGrowFactor) * extraSpace;
      }

      // Determine cross axis size
      float componentCrossAxisSize = isHorizontal ? static_cast<float>(dims.height) : static_cast<float>(dims.width);
      componentCrossAxisSize = determineCrossAxisSize(componentCrossAxisSize, crossAxisTotal);

      // Set component bounds
      if (isHorizontal) {
        float x = config_.direction == FlexDirection::ROW_REVERSE
                    ? (mainAxisTotal - currentPosition - componentMainAxisSize)
                    : currentPosition;
        float y = determineCrossAxisPosition(componentCrossAxisSize, crossAxisTotal);
        component->setBounds(static_cast<int>(x), static_cast<int>(y), static_cast<int>(componentMainAxisSize),
                             static_cast<int>(componentCrossAxisSize));
      } else {
        float x = determineCrossAxisPosition(componentCrossAxisSize, crossAxisTotal);
        float y = config_.direction == FlexDirection::COLUMN_REVERSE
                    ? (mainAxisTotal - currentPosition - componentMainAxisSize)
                    : currentPosition;
        component->setBounds(static_cast<int>(x), static_cast<int>(y), static_cast<int>(componentCrossAxisSize),
                             static_cast<int>(componentMainAxisSize));
      }

      // Update position for next component based on justify content
      currentPosition +=
        componentMainAxisSize + calculateJustifyGap(totalMainAxisSize, mainAxisTotal, components.size(), i);
    }
  }

private:
  float calculateJustifyStartPosition(float contentSize, float availableSize, size_t itemCount) {
    if (itemCount == 0)
      return 0.0f;

    float extraSpace = availableSize - contentSize;
    switch (config_.justify) {
    case FlexJustify::START:
      return 0.0f;
    case FlexJustify::END:
      return availableSize - contentSize;
    case FlexJustify::CENTER:
      return (availableSize - contentSize) / 2.0f;
    case FlexJustify::SPACE_BETWEEN:
      return 0.0f;
    case FlexJustify::SPACE_AROUND:
      return extraSpace > 0 ? extraSpace / (itemCount * 2) : 0.0f;
    case FlexJustify::SPACE_EVENLY:
      return extraSpace > 0 ? extraSpace / (itemCount + 1) : 0.0f;
    }
    return 0.0f;
  }

  float calculateJustifyGap(float contentSize, float availableSize, size_t itemCount, size_t currentIndex) {
    (void)currentIndex; // Unused - may be used in future for per-item spacing

    if (itemCount <= 1)
      return config_.gap;

    float extraSpace = availableSize - contentSize;
    if (extraSpace <= 0)
      return config_.gap;

    switch (config_.justify) {
    case FlexJustify::START:
    case FlexJustify::END:
    case FlexJustify::CENTER:
      return config_.gap;
    case FlexJustify::SPACE_BETWEEN:
      return config_.gap + (extraSpace / (itemCount - 1));
    case FlexJustify::SPACE_AROUND:
      return config_.gap + (extraSpace / itemCount);
    case FlexJustify::SPACE_EVENLY:
      return config_.gap + (extraSpace / (itemCount + 1));
    }
    return config_.gap;
  }

  float determineCrossAxisSize(float componentSize, float availableSize) {
    switch (config_.align) {
    case FlexAlign::STRETCH:
      return availableSize;
    case FlexAlign::START:
    case FlexAlign::END:
    case FlexAlign::CENTER:
    case FlexAlign::BASELINE:
      return componentSize;
    }
    return componentSize;
  }

  float determineCrossAxisPosition(float componentSize, float availableSize) {
    switch (config_.align) {
    case FlexAlign::START:
      return 0.0f;
    case FlexAlign::END:
      return availableSize - componentSize;
    case FlexAlign::CENTER:
      return (availableSize - componentSize) / 2.0f;
    case FlexAlign::STRETCH:
    case FlexAlign::BASELINE:
      return 0.0f;
    }
    return 0.0f;
  }

  Configuration config_;
  std::vector<FlexItemProperties> itemProperties_;
};

/**
 * @brief Concrete FlexLayout class for use with Panel
 *
 * Provides a clean API for flexbox-style layouts without exposing
 * the CRTP implementation details.
 */
class FlexLayout : public FlexLayoutManager<FlexLayout> {
public:
  // Re-export Configuration at the concrete class level for clean API
  using Configuration = FlexLayoutManager<FlexLayout>::Configuration;
  using FlexItemProperties = FlexLayoutManager<FlexLayout>::FlexItemProperties;

  FlexLayout() = default;
  ~FlexLayout() = default;
};

} // namespace bombfork::prong::layout

namespace bombfork::prong {

// Forward declare Panel template
template <typename LayoutT>
class Panel;

// Type alias for FlexPanel - hides the CRTP implementation
using FlexPanel = Panel<layout::FlexLayoutManager<layout::FlexLayout>>;

} // namespace bombfork::prong