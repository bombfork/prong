/**
 * @file flow_layout.h
 * @brief Flow Layout Manager for T4C Client UI Framework
 * @copyright (C) 2025 Nazagoth Interactive
 */
#pragma once

#include <bombfork/prong/core/component_traits.h>
#include <bombfork/prong/layout/layout_manager.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

namespace bombfork::prong::layout {

/**
 * @enum FlowAlignment
 * @brief Defines alignment of flow layout
 */
enum class FlowAlignment {
  START,         // Start from top/left
  CENTER,        // Center with balanced distribution
  END,           // End at bottom/right
  SPACE_BETWEEN, // Distribute with max space between
  SPACE_AROUND   // Distribute with equal padding
};

/**
 * @enum FlowOverflow
 * @brief Defines behavior when components exceed available space
 */
enum class FlowOverflow {
  WRAP,   // Wrap to next line/column
  SCROLL, // Enable scrolling
  CLIP,   // Clip excess components
  RESIZE  // Resize container
};

/**
 * @class FlowLayoutManager
 * @brief Dynamically flowing layout of components
 * @tparam DerivedT The CRTP derived class type
 */
template <typename DerivedT>
class FlowLayoutManager : public LayoutManager<DerivedT> {
public:
  struct Configuration {
    bool horizontal = false; // Flow direction
    FlowAlignment mainAlignment = FlowAlignment::START;
    FlowAlignment crossAlignment = FlowAlignment::START;
    FlowOverflow overflowBehavior = FlowOverflow::WRAP;
    float spacing = 4.0f;       // Spacing between items
    float crossSpacing = 4.0f;  // Spacing between rows/columns
    size_t maxItemsPerLine = 0; // 0 means auto
  };

  FlowLayoutManager() : config_{} {}

  /**
   * @brief Configure the flow layout
   * @param config Configuration parameters
   * @return Reference to derived class for chaining
   */
  DerivedT& configure(const Configuration& config) {
    config_ = config;
    return static_cast<DerivedT&>(*this);
  }

  /**
   * @brief Measure total space required by flow components
   * @param components List of components to measure
   * @return Calculated dimensions
   */
  Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& components) override {
    if (components.empty())
      return {0, 0};

    std::vector<Dimensions> lineDimensions;
    size_t currentLineCount = 0;
    float currentLineWidth = 0.0f;
    float currentLineHeight = 0.0f;
    float totalHeight = 0.0f;
    size_t maxItemsPerLine = config_.maxItemsPerLine > 0 ? config_.maxItemsPerLine : std::numeric_limits<size_t>::max();

    for (const auto* component : components) {
      auto componentSize = component->getPreferredSize();

      if (currentLineCount >= maxItemsPerLine ||
          (currentLineWidth + componentSize.width + config_.spacing >
           (config_.horizontal ? std::numeric_limits<float>::max() : maxLineWidth_))) {

        // Finalize current line
        lineDimensions.push_back(
          {static_cast<int>(currentLineWidth - config_.spacing), static_cast<int>(currentLineHeight)});
        totalHeight += currentLineHeight + config_.crossSpacing;

        // Reset line trackers
        currentLineCount = 0;
        currentLineWidth = 0.0f;
        currentLineHeight = 0.0f;
      }

      currentLineCount++;
      currentLineWidth += componentSize.width + config_.spacing;
      currentLineHeight = std::max(currentLineHeight, static_cast<float>(componentSize.height));
    }

    // Handle last line
    if (currentLineCount > 0) {
      lineDimensions.push_back(
        {static_cast<int>(currentLineWidth - config_.spacing), static_cast<int>(currentLineHeight)});
      totalHeight += currentLineHeight;
    }

    // Calculate total width (maximum line width)
    float totalWidth =
      std::max_element(lineDimensions.begin(), lineDimensions.end(), [](const Dimensions& a, const Dimensions& b) {
        return a.width < b.width;
      })->width;

    return {static_cast<int>(totalWidth), static_cast<int>(totalHeight - config_.crossSpacing)};
  }

  /**
   * @brief Layout components in flow configuration
   * @param components List of components to layout
   * @param availableSpace Total available space
   */
  void layout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) override {
    if (components.empty())
      return;

    std::vector<std::vector<bombfork::prong::Component*>> lines;
    std::vector<bombfork::prong::Component*> currentLine;
    float currentLineWidth = 0.0f;
    float currentLineHeight = 0.0f;
    size_t maxItemsPerLine = config_.maxItemsPerLine > 0 ? config_.maxItemsPerLine : std::numeric_limits<size_t>::max();

    // Group components into lines
    for (auto* component : components) {
      auto componentSize = component->getPreferredSize();

      if (currentLine.size() >= maxItemsPerLine ||
          (currentLineWidth + componentSize.width + config_.spacing >
           (config_.horizontal ? std::numeric_limits<float>::max() : maxLineWidth_))) {

        lines.push_back(currentLine);
        currentLine.clear();
        currentLineWidth = 0.0f;
        currentLineHeight = 0.0f;
      }

      currentLine.push_back(component);
      currentLineWidth += componentSize.width + config_.spacing;
      currentLineHeight = std::max(currentLineHeight, static_cast<float>(componentSize.height));
    }

    if (!currentLine.empty()) {
      lines.push_back(currentLine);
    }

    // Position components based on alignment strategies
    float currentY = 0.0f;
    for (auto& line : lines) {
      float lineWidth = std::accumulate(line.begin(), line.end(), 0.0f,
                                        [this](float acc, const auto* component) {
                                          return acc + component->getPreferredSize().width + config_.spacing;
                                        }) -
                        config_.spacing;

      float currentX = 0.0f;
      float lineHeight = (*std::max_element(line.begin(), line.end(),
                                            [](const auto* a, const auto* b) {
                                              return a->getPreferredSize().height < b->getPreferredSize().height;
                                            }))
                           ->getPreferredSize()
                           .height;

      // Horizontal alignment
      switch (config_.mainAlignment) {
      case FlowAlignment::CENTER:
        currentX = (availableSpace.width - lineWidth) / 2.0f;
        break;
      case FlowAlignment::END:
        currentX = availableSpace.width - lineWidth;
        break;
      default:
        currentX = 0.0f;
        break;
      }

      for (auto* component : line) {
        auto componentSize = component->getPreferredSize();
        Rect bounds{currentX, currentY, static_cast<float>(componentSize.width),
                    static_cast<float>(componentSize.height)};

        // Vertical alignment within line
        switch (config_.crossAlignment) {
        case FlowAlignment::CENTER:
          bounds.y += (lineHeight - componentSize.height) / 2.0f;
          break;
        case FlowAlignment::END:
          bounds.y += lineHeight - componentSize.height;
          break;
        default:
          break;
        }

        component->setBounds(static_cast<int>(bounds.x), static_cast<int>(bounds.y), static_cast<int>(bounds.width),
                             static_cast<int>(bounds.height));
        currentX += componentSize.width + config_.spacing;
      }

      currentY += lineHeight + config_.crossSpacing;
    }
  }

private:
  Configuration config_;
  float maxLineWidth_ = 1920.0f; // Default max line width, configurable
};

/**
 * @class FlowLayout
 * @brief Concrete implementation of FlowLayoutManager for general use
 */
class FlowLayout : public FlowLayoutManager<FlowLayout> {
public:
  // Re-export Configuration for clean API
  using Configuration = FlowLayoutManager<FlowLayout>::Configuration;

  FlowLayout() = default;
  ~FlowLayout() = default;
};

} // namespace bombfork::prong::layout

namespace bombfork::prong {

// Forward declare Panel template
template <typename LayoutT>
class Panel;

// Type alias for FlowPanel - hides CRTP implementation
using FlowPanel = Panel<layout::FlowLayoutManager<layout::FlowLayout>>;

} // namespace bombfork::prong