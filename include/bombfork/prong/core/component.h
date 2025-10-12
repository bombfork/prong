#pragma once

#include <bombfork/prong/layout/layout_manager.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bombfork::prong::rendering {
class IRenderer;
}

namespace bombfork::prong {

// Forward declaration
class Component;

namespace detail {
/**
 * @brief Adapter class that wraps a prong::Component to provide layout::Component interface
 */
class ComponentAdapter : public layout::Component {
public:
  explicit ComponentAdapter(bombfork::prong::Component* component) : prongComponent_(component) {}

  layout::Dimensions measure() const override;
  layout::Dimensions measureLayout() const override { return measure(); }
  void setBounds(const layout::Rect& bounds) override;

private:
  bombfork::prong::Component* prongComponent_;
};
} // namespace detail

/**
 * @brief Base class for all new UI system components
 *
 * This is the foundation for the t4c::ui namespace component hierarchy.
 * It provides essential functionality for rendering, event handling,
 * geometry management, and parent/child relationships.
 *
 * This replaces t4c::client::files::UIComponent for the new UI system.
 */
class Component {
public:
  using FocusCallback = std::function<void(bool)>;
  using VisibilityCallback = std::function<void(bool)>;

  enum class FocusState {
    NONE,     // Component has no focus
    HOVERING, // Mouse is over component
    FOCUSED,  // Component has keyboard focus
    ACTIVE    // Component is being interacted with
  };

public:
  // Make these public for new UI components to access directly
  bombfork::prong::rendering::IRenderer* renderer = nullptr;
  int x = 0, y = 0, width = 0, height = 0;

protected:
  // Component state
  bool visible = true;
  bool enabled = true;
  FocusState focusState = FocusState::NONE;

  // Parent/child relationships
  Component* parent = nullptr;
  std::vector<std::unique_ptr<Component>> children;

  // Layout management
  std::shared_ptr<void> layoutManager; // Type-erased layout manager
  std::function<void(std::vector<std::shared_ptr<layout::Component>>&, const layout::Dimensions&)> layoutFunc;
  bool layoutInvalid = true;

  // Callbacks
  FocusCallback focusCallback;
  VisibilityCallback visibilityCallback;

  // Debug information
  std::string debugName;

public:
  explicit Component(bombfork::prong::rendering::IRenderer* renderer = nullptr,
                     const std::string& debugName = "Component")
    : renderer(renderer), debugName(debugName) {}

  virtual ~Component() = default;

  // === Renderer Management ===

  virtual void setRenderer(bombfork::prong::rendering::IRenderer* newRenderer) {
    renderer = newRenderer;
    for (auto& child : children) {
      if (child) {
        child->setRenderer(newRenderer);
      }
    }
  }

  bombfork::prong::rendering::IRenderer* getRenderer() const { return renderer; }

  // === Geometry Management ===

  virtual void setBounds(int newX, int newY, int newWidth, int newHeight) {
    x = newX;
    y = newY;
    width = newWidth;
    height = newHeight;
  }

  void getBounds(int& outX, int& outY, int& outWidth, int& outHeight) const {
    outX = x;
    outY = y;
    outWidth = width;
    outHeight = height;
  }

  void getPosition(int& outX, int& outY) const {
    outX = x;
    outY = y;
  }

  void getSize(int& outWidth, int& outHeight) const {
    outWidth = width;
    outHeight = height;
  }

  void setPosition(int newX, int newY) {
    x = newX;
    y = newY;
  }

  void setSize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
  }

  virtual bool contains(int pointX, int pointY) const {
    return pointX >= x && pointX < x + width && pointY >= y && pointY < y + height;
  }

  // Alias for EventDispatcher compatibility
  bool containsGlobal(int globalX, int globalY) const { return contains(globalX, globalY); }

  // Convert global coordinates to component-local coordinates
  void globalToLocal(int globalX, int globalY, int& localX, int& localY) const {
    localX = globalX - x;
    localY = globalY - y;
  }

  // === Visibility and State Management ===

  virtual void setVisible(bool isVisible) {
    if (visible != isVisible) {
      visible = isVisible;
      if (visibilityCallback) {
        visibilityCallback(visible);
      }
    }
  }

  bool isVisible() const { return visible; }

  virtual void setEnabled(bool isEnabled) { enabled = isEnabled; }

  bool isEnabled() const { return enabled; }

  // === Focus Management ===

  virtual void setFocusState(FocusState state) {
    if (focusState != state) {
      focusState = state;
      if (focusCallback) {
        focusCallback(state == FocusState::FOCUSED);
      }
    }
  }

  FocusState getFocusState() const { return focusState; }

  bool hasFocus() const { return focusState == FocusState::FOCUSED; }

  virtual void requestFocus() { setFocusState(FocusState::FOCUSED); }

  virtual void releaseFocus() { setFocusState(FocusState::NONE); }

  void setFocusCallback(FocusCallback callback) { focusCallback = std::move(callback); }

  virtual bool canReceiveFocus() const { return enabled && visible; }

  // === Parent/Child Management ===

  void addChild(std::unique_ptr<Component> child) {
    if (child) {
      child->parent = this;
      child->setRenderer(renderer);
      children.push_back(std::move(child));
      invalidateLayout();
    }
  }

  bool removeChild(Component* child) {
    auto it = std::find_if(children.begin(), children.end(),
                           [child](const std::unique_ptr<Component>& c) { return c.get() == child; });

    if (it != children.end()) {
      (*it)->parent = nullptr;
      children.erase(it);
      return true;
    }
    return false;
  }

  Component* getParent() const { return parent; }

  const std::vector<std::unique_ptr<Component>>& getChildren() const { return children; }

  // === Layout Management ===

  /**
   * @brief Set the layout manager for this component
   * @tparam LayoutT The layout manager type (must derive from LayoutManager<LayoutT>)
   * @param layout Shared pointer to the layout manager
   */
  template <typename LayoutT>
  void setLayout(std::shared_ptr<LayoutT> layout) {
    layoutManager = layout;
    // Store a type-erased function that calls the layout manager's layout method
    layoutFunc = [layout](std::vector<std::shared_ptr<layout::Component>>& components,
                          const layout::Dimensions& availableSpace) { layout->layout(components, availableSpace); };
    invalidateLayout();
  }

  /**
   * @brief Clear the layout manager
   */
  void clearLayout() {
    layoutManager.reset();
    layoutFunc = nullptr;
    invalidateLayout();
  }

  /**
   * @brief Check if this component has a layout manager
   */
  bool hasLayout() const { return layoutManager != nullptr; }

  /**
   * @brief Mark the layout as invalid, requiring recalculation
   */
  void invalidateLayout() { layoutInvalid = true; }

  /**
   * @brief Perform layout on children if a layout manager is set
   */
  void performLayout() {
    if (!layoutInvalid || !layoutFunc) {
      return;
    }

    // Mark layout as valid first to avoid infinite recursion
    layoutInvalid = false;

    // Create adapter wrappers for children to match layout::Component interface
    std::vector<std::shared_ptr<layout::Component>> adaptedChildren;
    adaptedChildren.reserve(children.size());

    for (auto& child : children) {
      if (child) {
        adaptedChildren.push_back(std::make_shared<detail::ComponentAdapter>(child.get()));
      }
    }

    // Create available space from current component dimensions
    layout::Dimensions availableSpace{width, height};

    // Call the layout manager through the type-erased function
    layoutFunc(adaptedChildren, availableSpace);

    // Recursively perform layout on children
    for (auto& child : children) {
      if (child) {
        child->performLayout();
      }
    }
  }

  /**
   * @brief Get the preferred size of this component
   * @return Dimensions struct with preferred width and height
   */
  virtual layout::Dimensions getPreferredSize() const { return {width, height}; }

  // === Event Handling ===

  virtual bool handleClick(int localX, int localY) {
    // Default: delegate to children
    // localX/localY are relative to this component, but children's positions are also relative
    // to this component So we need to convert to child-local coordinates
    for (auto& child : children) {
      if (child && child->isVisible()) {
        // Check if point is within child's bounds (using parent-local coordinates)
        if (localX >= child->x && localX < child->x + child->width && localY >= child->y &&
            localY < child->y + child->height) {
          // Convert to child-local coordinates
          int childLocalX = localX - child->x;
          int childLocalY = localY - child->y;
          if (child->handleClick(childLocalX, childLocalY)) {
            return true;
          }
        }
      }
    }
    return false;
  }

  virtual bool handleMousePress(int localX, int localY, int button) {
    for (auto& child : children) {
      if (child && child->isVisible()) {
        // Check if point is within child's bounds (using parent-local coordinates)
        if (localX >= child->x && localX < child->x + child->width && localY >= child->y &&
            localY < child->y + child->height) {
          // Convert to child-local coordinates
          int childLocalX = localX - child->x;
          int childLocalY = localY - child->y;
          if (child->handleMousePress(childLocalX, childLocalY, button)) {
            return true;
          }
        }
      }
    }
    return false;
  }

  virtual bool handleMouseRelease(int localX, int localY, int button) {
    for (auto& child : children) {
      if (child && child->isVisible()) {
        // Check if point is within child's bounds (using parent-local coordinates)
        if (localX >= child->x && localX < child->x + child->width && localY >= child->y &&
            localY < child->y + child->height) {
          // Convert to child-local coordinates
          int childLocalX = localX - child->x;
          int childLocalY = localY - child->y;
          if (child->handleMouseRelease(childLocalX, childLocalY, button)) {
            return true;
          }
        }
      }
    }
    return false;
  }

  virtual bool handleMouseMove(int localX, int localY) {
    for (auto& child : children) {
      if (child && child->isVisible()) {
        // Convert to child-local coordinates
        int childLocalX = localX - child->x;
        int childLocalY = localY - child->y;
        if (child->handleMouseMove(childLocalX, childLocalY)) {
          return true;
        }
      }
    }
    return false;
  }

  virtual void handleMouseEnter() {}
  virtual void handleMouseLeave() {}

  virtual bool handleScroll(int localX, int localY, double xoffset, double yoffset) {
    for (auto& child : children) {
      if (child && child->isVisible()) {
        // Check if point is within child's bounds (using parent-local coordinates)
        if (localX >= child->x && localX < child->x + child->width && localY >= child->y &&
            localY < child->y + child->height) {
          // Convert to child-local coordinates
          int childLocalX = localX - child->x;
          int childLocalY = localY - child->y;
          if (child->handleScroll(childLocalX, childLocalY, xoffset, yoffset)) {
            return true;
          }
        }
      }
    }
    return false;
  }

  virtual bool handleKey(int key, int action, int mods) {
    for (auto& child : children) {
      if (child && child->hasFocus()) {
        if (child->handleKey(key, action, mods)) {
          return true;
        }
      }
    }
    return false;
  }

  virtual bool handleChar(unsigned int codepoint) {
    for (auto& child : children) {
      if (child && child->hasFocus()) {
        if (child->handleChar(codepoint)) {
          return true;
        }
      }
    }
    return false;
  }

  // === Update ===

  void updateAll(double deltaTime) {
    if (!visible)
      return;
    update(deltaTime);
    for (auto& child : children) {
      if (child && child->isVisible()) {
        child->updateAll(deltaTime);
      }
    }
  }

  virtual void update(double deltaTime) = 0; // Pure virtual - must be implemented

  // === Rendering ===

  void renderAll() {
    if (!visible)
      return;
    render();
    for (auto& child : children) {
      if (child && child->isVisible()) {
        child->renderAll();
      }
    }
  }

  virtual void render() = 0; // Pure virtual - must be implemented

  // === Debug ===

  const std::string& getDebugName() const { return debugName; }
  void setDebugName(const std::string& name) { debugName = name; }
};

// === Inline implementations for detail::ComponentAdapter ===

inline layout::Dimensions detail::ComponentAdapter::measure() const {
  if (prongComponent_) {
    auto preferred = prongComponent_->getPreferredSize();
    return {static_cast<int>(preferred.width), static_cast<int>(preferred.height)};
  }
  return {0, 0};
}

inline void detail::ComponentAdapter::setBounds(const layout::Rect& bounds) {
  if (prongComponent_) {
    // bombfork::prong::Component::setBounds takes 4 ints, not a Rect
    prongComponent_->setBounds(static_cast<int>(bounds.x), static_cast<int>(bounds.y), static_cast<int>(bounds.width),
                               static_cast<int>(bounds.height));
  }
}

} // namespace bombfork::prong