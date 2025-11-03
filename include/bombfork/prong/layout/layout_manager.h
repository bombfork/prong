#pragma once

#include <memory>
#include <vector>

namespace bombfork::prong {
// Forward declaration of prong::Component
class Component;
} // namespace bombfork::prong

namespace bombfork::prong::layout {

/**
 * @brief Basic dimensions structure
 */
struct Dimensions {
  int width = 0;
  int height = 0;
};

/**
 * @brief Basic rectangle structure
 */
struct Rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
};

/**
 * @brief Base layout manager class using CRTP
 *
 * Layout managers now work directly with prong::Component pointers.
 * Components are owned by their parent via unique_ptr and passed to
 * layout managers as raw pointers for positioning.
 */
template <typename DerivedT>
class LayoutManager {
public:
  virtual ~LayoutManager() = default;

  /**
   * @brief Measure required space for components (Phase 1: Natural/unconstrained measurement)
   * @param components List of components to measure
   * @return Natural dimensions without constraints
   */
  virtual Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& components) = 0;

  /**
   * @brief Measure required space with constraints (Phase 2: Constrained measurement)
   * @param components List of components to measure
   * @param constraints Available space constraints (width/height)
   * @return Dimensions constrained by available space
   *
   * @note This method enables wrapping layouts (like FlowLayout) to accurately predict
   * their height when given a width constraint. The default implementation falls back
   * to unconstrained measurement for layouts that don't need constraint information.
   */
  virtual Dimensions measureLayoutConstrained(const std::vector<bombfork::prong::Component*>& components,
                                              const Dimensions& constraints) {
    // Default implementation: ignore constraints and use natural measurement
    (void)constraints; // Suppress unused parameter warning
    return measureLayout(components);
  }

  /**
   * @brief Layout components within available space
   */
  virtual void layout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) = 0;

protected:
  LayoutManager() = default;
};

} // namespace bombfork::prong::layout