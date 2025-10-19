/**
 * @file grid_layout.h
 * @brief Grid Layout Manager for T4C Client UI Framework
 * @copyright (C) 2025 Nazagoth Interactive
 */
#pragma once

#include <bombfork/prong/core/component_traits.h>
#include <bombfork/prong/layout/layout_manager.h>

#include <cmath>
#include <memory>
#include <numeric>
#include <vector>

namespace bombfork::prong::layout {

/**
 * @enum GridAlignment
 * @brief Defines cell alignment within grid
 */
enum class GridAlignment {
  START,  // Align to start of cell
  CENTER, // Center within cell
  END,    // Align to end of cell
  STRETCH // Stretch to fill cell
};

/**
 * @class GridLayoutManager
 * @brief Flexible grid layout manager supporting dynamic rows/columns
 * @tparam DerivedT The CRTP derived class type
 */
template <typename DerivedT>
class GridLayoutManager : public LayoutManager<DerivedT> {
public:
  struct Configuration {
    size_t columns = 1;             // Number of columns
    size_t rows = 0;                // Auto-calculate if 0
    float horizontalSpacing = 0.0f; // Space between columns
    float verticalSpacing = 0.0f;   // Space between rows
    GridAlignment cellAlignment = GridAlignment::STRETCH;
    bool equalCellSize = false; // Force uniform cell sizes
  };

  GridLayoutManager() : config_{} {}

  /**
   * @brief Configure the grid layout
   * @param config Configuration parameters
   * @return Reference to derived class for chaining
   */
  DerivedT& configure(const Configuration& config) {
    config_ = config;
    return static_cast<DerivedT&>(*this);
  }

  /**
   * @brief Measure total space required by grid components
   * @param components List of components to measure
   * @return Calculated dimensions
   */
  Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& components) override {
    size_t rows = calculateRows(components);
    size_t cols = config_.columns;

    std::vector<float> columnWidths(cols, 0.0f);
    std::vector<float> rowHeights(rows, 0.0f);

    // Measure cell sizes, considering minimum sizes
    for (size_t i = 0; i < components.size(); ++i) {
      auto componentSize = components[i]->getPreferredSize();
      size_t row = i / cols;
      size_t col = i % cols;

      // Get minimum sizes
      float minWidth = static_cast<float>(components[i]->getMinimumWidth());
      float minHeight = static_cast<float>(components[i]->getMinimumHeight());

      // Use maximum of preferred size and minimum size
      float effectiveWidth = std::max(static_cast<float>(componentSize.width), minWidth);
      float effectiveHeight = std::max(static_cast<float>(componentSize.height), minHeight);

      columnWidths[col] = std::max(columnWidths[col], effectiveWidth);
      rowHeights[row] = std::max(rowHeights[row], effectiveHeight);
    }

    // Calculate total dimensions
    float totalWidth = std::accumulate(columnWidths.begin(), columnWidths.end(), 0.0f);
    totalWidth += (cols - 1) * config_.horizontalSpacing;

    float totalHeight = std::accumulate(rowHeights.begin(), rowHeights.end(), 0.0f);
    totalHeight += (rows - 1) * config_.verticalSpacing;

    return {static_cast<int>(totalWidth), static_cast<int>(totalHeight)};
  }

  /**
   * @brief Layout components in grid configuration
   * @param components List of components to layout
   * @param availableSpace Total available space
   */
  void layout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) override {
    (void)availableSpace; // Unused parameter - grid uses measured component sizes
    size_t rows = calculateRows(components);
    size_t cols = config_.columns;

    std::vector<float> columnWidths(cols, 0.0f);
    std::vector<float> rowHeights(rows, 0.0f);

    // First pass: determine cell sizes, considering minimum sizes
    for (size_t i = 0; i < components.size(); ++i) {
      auto componentSize = components[i]->getPreferredSize();
      size_t row = i / cols;
      size_t col = i % cols;

      // Get minimum sizes
      float minWidth = static_cast<float>(components[i]->getMinimumWidth());
      float minHeight = static_cast<float>(components[i]->getMinimumHeight());

      // Use maximum of preferred size and minimum size
      float effectiveWidth = std::max(static_cast<float>(componentSize.width), minWidth);
      float effectiveHeight = std::max(static_cast<float>(componentSize.height), minHeight);

      columnWidths[col] = std::max(columnWidths[col], effectiveWidth);
      rowHeights[row] = std::max(rowHeights[row], effectiveHeight);
    }

    // Optional: Normalize cell sizes if requested
    if (config_.equalCellSize) {
      float maxWidth = *std::max_element(columnWidths.begin(), columnWidths.end());
      float maxHeight = *std::max_element(rowHeights.begin(), rowHeights.end());

      std::fill(columnWidths.begin(), columnWidths.end(), maxWidth);
      std::fill(rowHeights.begin(), rowHeights.end(), maxHeight);
    }

    // Position components
    float currentY = 0.0f;
    for (size_t row = 0; row < rows; ++row) {
      float currentX = 0.0f;
      for (size_t col = 0; col < cols; ++col) {
        size_t index = row * cols + col;
        if (index >= components.size())
          break;

        auto* component = components[index];
        auto componentSize = component->getPreferredSize();

        // Get minimum sizes
        float minWidth = static_cast<float>(component->getMinimumWidth());
        float minHeight = static_cast<float>(component->getMinimumHeight());

        // For zero-sized components, use minimum size as starting point
        float componentWidth = componentSize.width > 0 ? static_cast<float>(componentSize.width) : minWidth;
        float componentHeight = componentSize.height > 0 ? static_cast<float>(componentSize.height) : minHeight;

        // Ensure component respects minimum size
        componentWidth = std::max(componentWidth, minWidth);
        componentHeight = std::max(componentHeight, minHeight);

        Rect cellBounds{currentX, currentY, columnWidths[col], rowHeights[row]};

        // Apply cell alignment
        switch (config_.cellAlignment) {
        case GridAlignment::START:
          // Use component size, not cell size, for START alignment
          cellBounds.width = componentWidth;
          cellBounds.height = componentHeight;
          break;
        case GridAlignment::CENTER:
          cellBounds.x += (columnWidths[col] - componentWidth) / 2.0f;
          cellBounds.y += (rowHeights[row] - componentHeight) / 2.0f;
          cellBounds.width = componentWidth;
          cellBounds.height = componentHeight;
          break;
        case GridAlignment::END:
          cellBounds.x += columnWidths[col] - componentWidth;
          cellBounds.y += rowHeights[row] - componentHeight;
          cellBounds.width = componentWidth;
          cellBounds.height = componentHeight;
          break;
        case GridAlignment::STRETCH:
          // STRETCH uses full cell size, ensuring minimum is respected
          cellBounds.width = std::max(columnWidths[col], minWidth);
          cellBounds.height = std::max(rowHeights[row], minHeight);
          break;
        }

        component->setBounds(static_cast<int>(cellBounds.x), static_cast<int>(cellBounds.y),
                             static_cast<int>(cellBounds.width), static_cast<int>(cellBounds.height));
        currentX += columnWidths[col] + config_.horizontalSpacing;
      }
      currentY += rowHeights[row] + config_.verticalSpacing;
    }
  }

private:
  /**
   * @brief Calculate number of rows based on components and columns
   * @param components List of components
   * @return Number of rows
   */
  size_t calculateRows(const std::vector<bombfork::prong::Component*>& components) const {
    if (config_.rows > 0)
      return config_.rows;
    return std::ceil(static_cast<float>(components.size()) / config_.columns);
  }

  Configuration config_;
};

/**
 * @brief Concrete GridLayout class for use with Panel
 */
class GridLayout : public GridLayoutManager<GridLayout> {
public:
  // Re-export Configuration for clean API
  using Configuration = GridLayoutManager<GridLayout>::Configuration;

  GridLayout() = default;
  ~GridLayout() = default;
};

} // namespace bombfork::prong::layout

namespace bombfork::prong {

// Forward declare Panel template
template <typename LayoutT>
class Panel;

// Type alias for GridPanel - hides CRTP implementation
using GridPanel = Panel<layout::GridLayoutManager<layout::GridLayout>>;

} // namespace bombfork::prong