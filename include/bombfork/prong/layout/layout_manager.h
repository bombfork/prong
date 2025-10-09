#pragma once

#include <memory>
#include <vector>

namespace bombfork::prong::layout {

// Forward declarations
class Component;

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
 */
template <typename DerivedT>
class LayoutManager {
public:
  virtual ~LayoutManager() = default;

  /**
   * @brief Measure required space for components
   */
  virtual Dimensions measureLayout(const std::vector<std::shared_ptr<Component>>& components) = 0;

  /**
   * @brief Layout components within available space
   */
  virtual void layout(std::vector<std::shared_ptr<Component>>& components, const Dimensions& availableSpace) = 0;

protected:
  LayoutManager() = default;
};

/**
 * @brief Minimal component interface for layout testing
 */
class Component {
public:
  virtual ~Component() = default;
  virtual Dimensions measure() const { return {100, 30}; }
  virtual Dimensions measureLayout() const { return measure(); }
  virtual void setBounds(const Rect& bounds) { (void)bounds; }
  virtual void setPosition(const Rect& position) { setBounds(position); }
  virtual void setSize(const Rect& size) { setBounds(size); }
};

} // namespace bombfork::prong::layout