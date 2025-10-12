/**
 * @file stack_layout.h
 * @brief Stack Layout Manager for T4C Client UI Framework
 * @copyright (C) 2025 Nazagoth Interactive
 */
#pragma once

#include <bombfork/prong/core/component_traits.h>
#include <bombfork/prong/layout/layout_manager.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace bombfork::prong::layout {

/**
 * @enum StackOrientation
 * @brief Defines the orientation of stacked components
 */
enum class StackOrientation {
  VERTICAL,  // Components stacked vertically
  HORIZONTAL // Components stacked horizontally
};

/**
 * @enum StackAlignment
 * @brief Defines alignment of components within the stack
 */
enum class StackAlignment {
  START,  // Align to start (top/left)
  CENTER, // Centered
  END,    // Align to end (bottom/right)
  STRETCH // Stretch to fill available space
};

/**
 * @class StackLayoutManager
 * @brief Layout manager for stacking components vertically or horizontally
 * @tparam DerivedT The CRTP derived class type
 */
template <typename DerivedT>
class StackLayoutManager : public LayoutManager<DerivedT> {
public:
  struct Configuration {
    StackOrientation orientation = StackOrientation::VERTICAL;
    StackAlignment alignment = StackAlignment::START;
    float spacing = 0.0f;
    bool expandMain = false;  // Expand main axis
    bool expandCross = false; // Expand cross axis
  };

  StackLayoutManager() : config_{} {}

  /**
   * @brief Configure the stack layout
   * @param config Configuration parameters
   * @return Reference to derived class for chaining
   */
  DerivedT& configure(const Configuration& config) {
    config_ = config;
    return static_cast<DerivedT&>(*this);
  }

  /**
   * @brief Measure total space required by components
   * @param components List of components to measure
   * @return Calculated dimensions
   */
  Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& components) override {
    Dimensions totalSize{0, 0};

    for (const auto* component : components) {
      auto componentSize = component->getPreferredSize();

      if (config_.orientation == StackOrientation::VERTICAL) {
        totalSize.height += componentSize.height + config_.spacing;
        totalSize.width = std::max(totalSize.width, componentSize.width);
      } else {
        totalSize.width += componentSize.width + config_.spacing;
        totalSize.height = std::max(totalSize.height, componentSize.height);
      }
    }

    // Remove last spacing, adjust for expand modes
    if (!components.empty()) {
      totalSize.height -= config_.orientation == StackOrientation::VERTICAL ? config_.spacing : 0;
      totalSize.width -= config_.orientation == StackOrientation::HORIZONTAL ? config_.spacing : 0;
    }

    return totalSize;
  }

  /**
   * @brief Layout components within available space
   * @param components List of components to layout
   * @param availableSpace Total available space
   */
  void layout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) override {
    if (components.empty())
      return;

    (void)measureLayout(components); // Unused but kept for potential future use
    float currentOffset = 0.0f;

    for (auto* component : components) {
      auto componentSize = component->getPreferredSize();
      Rect componentBounds{0, 0, static_cast<float>(componentSize.width), static_cast<float>(componentSize.height)};

      // Determine main and cross axis sizes
      float& mainSize =
        (config_.orientation == StackOrientation::VERTICAL) ? componentBounds.height : componentBounds.width;
      float& crossSize =
        (config_.orientation == StackOrientation::VERTICAL) ? componentBounds.width : componentBounds.height;
      float& mainPos = (config_.orientation == StackOrientation::VERTICAL) ? componentBounds.y : componentBounds.x;
      float& crossPos = (config_.orientation == StackOrientation::VERTICAL) ? componentBounds.x : componentBounds.y;

      // Position on main axis
      mainPos = currentOffset;

      // Handle main axis expansion
      if (config_.expandMain) {
        mainSize = (config_.orientation == StackOrientation::VERTICAL) ? availableSpace.height / components.size()
                                                                       : availableSpace.width / components.size();
      }

      // Handle cross axis alignment and expansion
      switch (config_.alignment) {
      case StackAlignment::START:
        crossPos = 0;
        break;
      case StackAlignment::CENTER:
        crossPos =
          (config_.orientation == StackOrientation::VERTICAL ? availableSpace.width - componentBounds.width
                                                             : availableSpace.height - componentBounds.height) /
          2.0f;
        break;
      case StackAlignment::END:
        crossPos = (config_.orientation == StackOrientation::VERTICAL ? availableSpace.width - componentBounds.width
                                                                      : availableSpace.height - componentBounds.height);
        break;
      case StackAlignment::STRETCH:
        crossSize = (config_.orientation == StackOrientation::VERTICAL) ? availableSpace.width : availableSpace.height;
        crossPos = 0;
        break;
      }

      // Set component bounds and advance offset
      component->setBounds(static_cast<int>(componentBounds.x), static_cast<int>(componentBounds.y),
                           static_cast<int>(componentBounds.width), static_cast<int>(componentBounds.height));
      currentOffset += mainSize + config_.spacing;
    }
  }

private:
  Configuration config_;
};

/**
 * @class StackLayout
 * @brief Concrete stack layout implementation (CRTP instantiation)
 *
 * This is the default instantiation of StackLayoutManager for general use.
 * Provides vertical or horizontal stacking of components with configurable
 * spacing and alignment.
 */
class StackLayout : public StackLayoutManager<StackLayout> {
public:
  // Re-export Configuration for clean API
  using Configuration = StackLayoutManager<StackLayout>::Configuration;

  StackLayout() = default;
  ~StackLayout() = default;
};

} // namespace bombfork::prong::layout

namespace bombfork::prong {

// Forward declare Panel template
template <typename LayoutT>
class Panel;

// Type alias for StackPanel - hides CRTP implementation
using StackPanel = Panel<layout::StackLayoutManager<layout::StackLayout>>;

} // namespace bombfork::prong