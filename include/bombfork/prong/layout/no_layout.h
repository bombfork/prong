#pragma once

#include <bombfork/prong/layout/layout_manager.h>

#include <vector>

namespace bombfork::prong {
class Component;
} // namespace bombfork::prong

namespace bombfork::prong::layout {

/**
 * @brief No-op layout manager for components without automatic layout
 *
 * This is used as the default template parameter for Panel and other
 * container components that don't require automatic layout positioning.
 * It satisfies the LayoutManager interface but performs no operations.
 */
class NoLayout : public LayoutManager<NoLayout> {
public:
  /**
   * @brief Measure required space (no-op)
   * @return Zero dimensions since no layout is performed
   */
  Dimensions measureLayout(const std::vector<bombfork::prong::Component*>& /* components */) override { return {0, 0}; }

  /**
   * @brief Layout components (no-op)
   *
   * This does nothing - components retain their manually set positions
   */
  void layout(std::vector<bombfork::prong::Component*>& /* components */,
              const Dimensions& /* availableSpace */) override {
    // No-op: Components are positioned manually
  }
};

} // namespace bombfork::prong::layout
