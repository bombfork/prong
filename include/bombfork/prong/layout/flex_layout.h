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
    std::vector<float> minimumSizes;
    float totalMainAxisSize = 0.0f;
    std::vector<float> growFactors;
    std::vector<bool> isAutoGrow; // Track which components are auto-grow

    for (size_t i = 0; i < components.size(); ++i) {
      auto dims = components[i]->getPreferredSize();
      componentDimensions.push_back(dims);

      // Get minimum size for the main axis
      float minSize = isHorizontal ? static_cast<float>(components[i]->getMinimumWidth())
                                   : static_cast<float>(components[i]->getMinimumHeight());
      minimumSizes.push_back(minSize);

      // Get the main axis size from preferred dimensions
      float componentMainAxisSize = isHorizontal ? static_cast<float>(dims.width) : static_cast<float>(dims.height);

      // Detect zero-sized components (auto-grow candidates)
      bool isZeroSized = (componentMainAxisSize == 0.0f);
      isAutoGrow.push_back(isZeroSized);

      // For zero-sized components, use minimum size as the base
      if (isZeroSized) {
        componentMainAxisSize = minSize;
      }

      totalMainAxisSize += componentMainAxisSize;

      // Get explicit grow factor or auto-assign for zero-sized components
      float growFactor = 0.0f;
      if (i < itemProperties_.size()) {
        growFactor = itemProperties_[i].grow;
      }
      // Auto-assign grow=1.0 for zero-sized components if no explicit grow factor
      if (isZeroSized && growFactor == 0.0f) {
        growFactor = 1.0f;
      }
      growFactors.push_back(growFactor);
    }

    // Add gap space
    totalMainAxisSize += config_.gap * (components.size() - 1);

    // Handle overflow (shrinking) or extra space (growing)
    float spaceAvailable = mainAxisTotal - totalMainAxisSize;
    float totalGrowFactor = std::accumulate(growFactors.begin(), growFactors.end(), 0.0f);

    // Calculate actual sizes considering grow/shrink
    std::vector<float> finalSizes;
    if (spaceAvailable >= 0) {
      // Extra space available - grow components
      for (size_t i = 0; i < components.size(); ++i) {
        auto& dims = componentDimensions[i];
        float componentMainAxisSize = isHorizontal ? static_cast<float>(dims.width) : static_cast<float>(dims.height);

        // For zero-sized components, start with minimum size
        if (isAutoGrow[i]) {
          componentMainAxisSize = minimumSizes[i];
        }

        // Add growth space if applicable
        if (totalGrowFactor > 0 && growFactors[i] > 0) {
          componentMainAxisSize += (growFactors[i] / totalGrowFactor) * spaceAvailable;
        }

        finalSizes.push_back(std::max(componentMainAxisSize, minimumSizes[i]));
      }
    } else {
      // Overflow - need to shrink components proportionally
      // Calculate how much we can shrink each component (down to its minimum)
      std::vector<float> shrinkableSpace;
      float totalShrinkable = 0.0f;

      for (size_t i = 0; i < components.size(); ++i) {
        auto& dims = componentDimensions[i];
        float componentMainAxisSize = isHorizontal ? static_cast<float>(dims.width) : static_cast<float>(dims.height);

        // For zero-sized components, use minimum size
        if (isAutoGrow[i]) {
          componentMainAxisSize = minimumSizes[i];
        }

        float shrinkable = std::max(0.0f, componentMainAxisSize - minimumSizes[i]);
        shrinkableSpace.push_back(shrinkable);
        totalShrinkable += shrinkable;
      }

      // Distribute the deficit proportionally based on shrinkable space
      float deficit = -spaceAvailable; // Make it positive
      for (size_t i = 0; i < components.size(); ++i) {
        auto& dims = componentDimensions[i];
        float componentMainAxisSize = isHorizontal ? static_cast<float>(dims.width) : static_cast<float>(dims.height);

        if (isAutoGrow[i]) {
          componentMainAxisSize = minimumSizes[i];
        }

        // Shrink proportionally based on available shrinkable space
        float shrinkAmount = 0.0f;
        if (totalShrinkable > 0 && shrinkableSpace[i] > 0) {
          shrinkAmount = (shrinkableSpace[i] / totalShrinkable) * deficit;
        }

        float finalSize = componentMainAxisSize - shrinkAmount;
        finalSizes.push_back(std::max(finalSize, minimumSizes[i]));
      }
    }

    // TWO-PHASE LAYOUT FOR WRAPPING LAYOUTS (e.g., FlowLayout)
    // Phase 1: Calculate cross-axis sizes
    // Phase 2: For components with auto-sized main-axis, temporarily set cross-axis
    //          and re-query main-axis size (needed for wrapping layouts)

    std::vector<float> crossAxisSizes;
    crossAxisSizes.reserve(components.size());

    // Phase 1: Calculate cross-axis sizes
    for (size_t i = 0; i < components.size(); ++i) {
      auto& dims = componentDimensions[i];
      float componentCrossAxisSize = isHorizontal ? static_cast<float>(dims.height) : static_cast<float>(dims.width);
      componentCrossAxisSize = determineCrossAxisSize(componentCrossAxisSize, crossAxisTotal);
      crossAxisSizes.push_back(componentCrossAxisSize);
    }

    // Phase 2: Re-query main-axis sizes for wrapping layouts
    // For components where main-axis was initially zero (auto-size) AND cross-axis is stretched
    // temporarily set width/height so getMinimumHeight/Width can use constrained measurement
    for (size_t i = 0; i < components.size(); ++i) {
      auto* component = components[i];
      auto& dims = componentDimensions[i];

      // Check if this component might need constrained re-measurement
      // Criteria:
      // 1. Cross-axis is stretched (height for ROW, width for COLUMN)
      // 2. Main-axis was initially zero (width=0 for ROW, height=0 for COLUMN, needs calculation)
      // 3. Not an auto-grow component (those fill remaining space)
      bool wasCrossAxisStretched = (config_.align == FlexAlign::STRETCH && crossAxisSizes[i] > 0);
      bool wasMainAxisZero = isHorizontal ? (dims.width == 0) : (dims.height == 0);

      if (wasCrossAxisStretched && wasMainAxisZero && !isAutoGrow[i]) {
        // Store original size to restore later
        int origWidth = dims.width;
        int origHeight = dims.height;

        // Temporarily set the cross-axis size so getMinimumHeight/Width can use it
        if (isHorizontal) {
          // ROW: temporarily set width (needed for  height calculation, but rare)
          component->setSize(static_cast<int>(finalSizes[i]), 0);
        } else {
          // COLUMN: temporarily set width (needed for wrapping layouts like FlowLayout)
          component->setSize(static_cast<int>(crossAxisSizes[i]), 0);
        }

        // Re-query the main-axis size now that cross-axis is set
        int newMainAxisSize = isHorizontal ? component->getMinimumHeight() : component->getMinimumWidth();

        // Restore original size (we'll set final bounds later)
        component->setSize(origWidth, origHeight);

        if (newMainAxisSize > 0 && newMainAxisSize != static_cast<int>(finalSizes[i])) {
          // Update the final size for this component
          finalSizes[i] = static_cast<float>(newMainAxisSize);
        }
      }
    }

    // Recalculate total size after grow/shrink AND phase 2 adjustments
    float actualTotalSize =
      std::accumulate(finalSizes.begin(), finalSizes.end(), 0.0f) + config_.gap * (components.size() - 1);

    // Calculate starting position based on justify content
    float currentPosition = calculateJustifyStartPosition(actualTotalSize, mainAxisTotal, components.size());
    for (size_t i = 0; i < components.size(); ++i) {
      auto* component = components[i];

      // Use the final size (potentially updated in Phase 2)
      float componentMainAxisSize = finalSizes[i];
      float componentCrossAxisSize = crossAxisSizes[i];

      // Set final component bounds with correct positions
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
        componentMainAxisSize + calculateJustifyGap(actualTotalSize, mainAxisTotal, components.size(), i);
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