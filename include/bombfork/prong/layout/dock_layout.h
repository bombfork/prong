/**
 * @file dock_layout.h
 * @brief Dockable Panel Layout Manager for T4C Client UI Framework
 * @copyright (C) 2025 Nazagoth Interactive
 */
#pragma once

#include <bombfork/prong/core/component_traits.h>
#include <bombfork/prong/layout/layout_manager.h>

#include <memory>
#include <optional>
#include <vector>

namespace bombfork::prong::layout {

enum class DockArea { LEFT, RIGHT, TOP, BOTTOM, CENTER, FLOATING };

struct DockSplitterConfig {
  float minSize = 50.0f;     // Minimum panel size
  float maxSize = 500.0f;    // Maximum panel size
  bool resizable = true;     // Can user resize?
  float initialRatio = 0.2f; // Default size ratio
};

template <typename DerivedT>
class DockLayoutManager : public LayoutManager<DerivedT> {
public:
  struct DockConfiguration {
    bool allowFloating = true;
    bool showTabs = true;
    float splitterThickness = 5.0f;
  };

  struct DockRegion {
    DockArea area;
    std::vector<bombfork::prong::Component*> components;
    DockSplitterConfig splitterConfig;
    std::optional<size_t> activeComponentIndex;
  };

  DockLayoutManager() : config_{}, regions_{} {}

  DerivedT& configure(const DockConfiguration& config) {
    config_ = config;
    return static_cast<DerivedT&>(*this);
  }

  void addRegion(const DockRegion& region) { regions_.push_back(region); }

  Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& components) override {
    return calculateDockDimensions(components);
  }

  void layout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) override {
    performDockLayout(components, availableSpace);
  }

private:
  Dimensions calculateDockDimensions(const std::vector<bombfork::prong::Component*>& components) {
    Dimensions totalDimensions{0, 0};

    // Measure region components
    for (const auto& region : regions_) {
      for (const auto* component : region.components) {
        auto componentDims = component->getPreferredSize();
        totalDimensions.width = std::max(totalDimensions.width, componentDims.width);
        totalDimensions.height = std::max(totalDimensions.height, componentDims.height);
      }
    }

    // Also measure any components passed directly
    for (const auto* component : components) {
      auto componentDims = component->getPreferredSize();
      totalDimensions.width = std::max(totalDimensions.width, componentDims.width);
      totalDimensions.height = std::max(totalDimensions.height, componentDims.height);
    }

    return totalDimensions;
  }

  void performDockLayout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) {
    // If no regions configured and we have components, create a center region
    if (regions_.empty() && !components.empty()) {
      DockRegion centerRegion;
      centerRegion.area = DockArea::CENTER;
      centerRegion.components = components;
      centerRegion.activeComponentIndex = 0;
      regions_.push_back(centerRegion);
    }

    float leftWidth = 0.0f, rightWidth = 0.0f;
    float topHeight = 0.0f, bottomHeight = 0.0f;
    float centerWidth = availableSpace.width, centerHeight = availableSpace.height;

    // First pass: Allocate space for side regions
    for (auto& region : regions_) {
      switch (region.area) {
      case DockArea::LEFT: {
        leftWidth = std::min(region.splitterConfig.initialRatio * availableSpace.width, region.splitterConfig.maxSize);
        centerWidth -= leftWidth + config_.splitterThickness;
        break;
      }
      case DockArea::RIGHT: {
        rightWidth = std::min(region.splitterConfig.initialRatio * availableSpace.width, region.splitterConfig.maxSize);
        centerWidth -= rightWidth + config_.splitterThickness;
        break;
      }
      case DockArea::TOP: {
        topHeight = std::min(region.splitterConfig.initialRatio * availableSpace.height, region.splitterConfig.maxSize);
        centerHeight -= topHeight + config_.splitterThickness;
        break;
      }
      case DockArea::BOTTOM: {
        bottomHeight =
          std::min(region.splitterConfig.initialRatio * availableSpace.height, region.splitterConfig.maxSize);
        centerHeight -= bottomHeight + config_.splitterThickness;
        break;
      }
      default:
        break;
      }
    }

    // Position components in each region
    for (auto& region : regions_) {
      float regionX = 0.0f, regionY = 0.0f;
      float regionWidth = 0.0f, regionHeight = 0.0f;

      switch (region.area) {
      case DockArea::LEFT:
        regionX = 0.0f;
        regionY = 0.0f;
        regionWidth = leftWidth;
        regionHeight = availableSpace.height;
        break;
      case DockArea::RIGHT:
        regionX = availableSpace.width - rightWidth;
        regionY = 0.0f;
        regionWidth = rightWidth;
        regionHeight = availableSpace.height;
        break;
      case DockArea::TOP:
        regionX = 0.0f;
        regionY = 0.0f;
        regionWidth = availableSpace.width;
        regionHeight = topHeight;
        break;
      case DockArea::BOTTOM:
        regionX = 0.0f;
        regionY = availableSpace.height - bottomHeight;
        regionWidth = availableSpace.width;
        regionHeight = bottomHeight;
        break;
      case DockArea::CENTER:
        regionX = leftWidth + config_.splitterThickness;
        regionY = topHeight + config_.splitterThickness;
        regionWidth = centerWidth;
        regionHeight = centerHeight;
        break;
      default:
        continue;
      }

      // Position active component(s) in this region
      if (region.activeComponentIndex.has_value() && *region.activeComponentIndex < region.components.size()) {
        auto* component = region.components[*region.activeComponentIndex];
        component->setBounds(static_cast<int>(regionX), static_cast<int>(regionY), static_cast<int>(regionWidth),
                             static_cast<int>(regionHeight));
      } else if (!region.components.empty()) {
        // If no active component specified, use the first one
        auto* component = region.components[0];
        component->setBounds(static_cast<int>(regionX), static_cast<int>(regionY), static_cast<int>(regionWidth),
                             static_cast<int>(regionHeight));
      }
    }
  }

  DockConfiguration config_;
  std::vector<DockRegion> regions_;
};

} // namespace bombfork::prong::layout