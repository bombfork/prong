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
   * @brief Measure required space for components
   */
  virtual Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& components) = 0;

  /**
   * @brief Layout components within available space
   */
  virtual void layout(std::vector<bombfork::prong::Component*>& components, const Dimensions& availableSpace) = 0;

protected:
  LayoutManager() = default;
};

} // namespace bombfork::prong::layout