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
  // Renderer access
  bombfork::prong::rendering::IRenderer* renderer = nullptr;

private:
  // Coordinate storage: position relative to parent
  int localX = 0, localY = 0;

  // Cache for global coordinates (screen space)
  mutable int cachedGlobalX = 0;
  mutable int cachedGlobalY = 0;
  mutable bool globalCacheDirty = true;

protected:
  // Size is accessible to derived classes for rendering
  int width = 0, height = 0;

  /**
   * @brief Get global X coordinate for rendering (protected accessor for derived classes)
   * Use this in render() methods to get screen-space X coordinate
   */
  int getGlobalX() const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    return gx;
  }

  /**
   * @brief Get global Y coordinate for rendering (protected accessor for derived classes)
   * Use this in render() methods to get screen-space Y coordinate
   */
  int getGlobalY() const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    return gy;
  }

  // Component state
  bool visible = true;
  bool enabled = true;
  FocusState focusState = FocusState::NONE;

  // Parent/child relationships
  Component* parent = nullptr;
  std::vector<std::unique_ptr<Component>> children;

  // Layout management
  std::shared_ptr<void> layoutManager; // Type-erased layout manager
  std::function<void(std::vector<bombfork::prong::Component*>&, const layout::Dimensions&)> layoutFunc;
  std::function<layout::Dimensions(const std::vector<bombfork::prong::Component*>&)> measureFunc;
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

  /**
   * @brief Set component bounds (position relative to parent + size)
   * @param newX X position relative to parent's origin
   * @param newY Y position relative to parent's origin
   * @param newWidth Component width
   * @param newHeight Component height
   */
  virtual void setBounds(int newX, int newY, int newWidth, int newHeight) {
    localX = newX;
    localY = newY;
    width = newWidth;
    height = newHeight;
    invalidateGlobalCache();
  }

  /**
   * @brief Get component bounds (local position + size)
   * @param outX Output: X position relative to parent
   * @param outY Output: Y position relative to parent
   * @param outWidth Output: Component width
   * @param outHeight Output: Component height
   */
  void getBounds(int& outX, int& outY, int& outWidth, int& outHeight) const {
    outX = localX;
    outY = localY;
    outWidth = width;
    outHeight = height;
  }

  /**
   * @brief Get component position relative to parent
   * @param outX Output: X position relative to parent
   * @param outY Output: Y position relative to parent
   */
  void getPosition(int& outX, int& outY) const {
    outX = localX;
    outY = localY;
  }

  /**
   * @brief Get component size
   */
  void getSize(int& outWidth, int& outHeight) const {
    outWidth = width;
    outHeight = height;
  }

  /**
   * @brief Set component position relative to parent
   * @param newX X position relative to parent's origin
   * @param newY Y position relative to parent's origin
   */
  void setPosition(int newX, int newY) {
    localX = newX;
    localY = newY;
    invalidateGlobalCache();
  }

  /**
   * @brief Set component size
   */
  void setSize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
  }

  /**
   * @brief Get global (screen-space) position
   * @param outX Output: Absolute screen X coordinate
   * @param outY Output: Absolute screen Y coordinate
   */
  void getGlobalPosition(int& outX, int& outY) const {
    if (globalCacheDirty) {
      updateGlobalCache();
    }
    outX = cachedGlobalX;
    outY = cachedGlobalY;
  }

  /**
   * @brief Get global bounds (screen-space position + size)
   * @param outX Output: Absolute screen X coordinate
   * @param outY Output: Absolute screen Y coordinate
   * @param outWidth Output: Component width
   * @param outHeight Output: Component height
   */
  void getGlobalBounds(int& outX, int& outY, int& outWidth, int& outHeight) const {
    getGlobalPosition(outX, outY);
    outWidth = width;
    outHeight = height;
  }

  /**
   * @brief Check if a global screen point is within this component
   * @param globalX Absolute screen X coordinate
   * @param globalY Absolute screen Y coordinate
   * @return true if point is within component bounds
   */
  bool containsGlobal(int globalX, int globalY) const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    return globalX >= gx && globalX < gx + width && globalY >= gy && globalY < gy + height;
  }

  /**
   * @brief Convert global screen coordinates to component-local coordinates
   * @param globalX Input: Absolute screen X coordinate
   * @param globalY Input: Absolute screen Y coordinate
   * @param localX Output: X coordinate relative to this component
   * @param localY Output: Y coordinate relative to this component
   */
  void globalToLocal(int globalX, int globalY, int& localX, int& localY) const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    localX = globalX - gx;
    localY = globalY - gy;
  }

  /**
   * @brief Convert component-local coordinates to global screen coordinates
   * @param localX Input: X coordinate relative to this component
   * @param localY Input: Y coordinate relative to this component
   * @param globalX Output: Absolute screen X coordinate
   * @param globalY Output: Absolute screen Y coordinate
   */
  void localToGlobal(int localX, int localY, int& globalX, int& globalY) const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    globalX = gx + localX;
    globalY = gy + localY;
  }

private:
  /**
   * @brief Update the global coordinate cache
   * Called automatically when cache is dirty and global coords are needed
   */
  void updateGlobalCache() const {
    if (parent) {
      int parentGlobalX, parentGlobalY;
      parent->getGlobalPosition(parentGlobalX, parentGlobalY);
      cachedGlobalX = parentGlobalX + localX;
      cachedGlobalY = parentGlobalY + localY;
    } else {
      // Root component: local coordinates are global
      cachedGlobalX = localX;
      cachedGlobalY = localY;
    }
    globalCacheDirty = false;
  }

  /**
   * @brief Invalidate the global coordinate cache
   * Automatically cascades to all children
   */
  void invalidateGlobalCache() {
    if (!globalCacheDirty) {
      globalCacheDirty = true;
      // Cascade invalidation to all children
      for (auto& child : children) {
        if (child) {
          child->invalidateGlobalCache();
        }
      }
    }
  }

public:
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
    layoutFunc = [layout](std::vector<bombfork::prong::Component*>& components,
                          const layout::Dimensions& availableSpace) { layout->layout(components, availableSpace); };
    // Store a type-erased function that calls the layout manager's measureLayout method
    measureFunc = [layout](const std::vector<bombfork::prong::Component*>& components) {
      return layout->measureLayout(components);
    };
    invalidateLayout();
  }

  /**
   * @brief Clear the layout manager
   */
  void clearLayout() {
    layoutManager.reset();
    layoutFunc = nullptr;
    measureFunc = nullptr;
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
  virtual void performLayout() {
    if (!layoutInvalid || !layoutFunc) {
      return;
    }

    // Mark layout as valid first to avoid infinite recursion
    layoutInvalid = false;

    // Create vector of raw pointers to children
    std::vector<bombfork::prong::Component*> childPointers;
    childPointers.reserve(children.size());

    for (auto& child : children) {
      if (child) {
        childPointers.push_back(child.get());
      }
    }

    // Create available space from current component dimensions
    layout::Dimensions availableSpace{width, height};

    // Call the layout manager through the type-erased function
    layoutFunc(childPointers, availableSpace);

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

  /**
   * @brief Get the minimum width of this component
   * @return Minimum width in pixels (default: 0)
   */
  virtual int getMinimumWidth() const { return 0; }

  /**
   * @brief Get the minimum height of this component
   * @return Minimum height in pixels (default: 0)
   */
  virtual int getMinimumHeight() const { return 0; }

  // === Event Handling ===

  virtual bool handleClick(int localX, int localY) {
    // Default: delegate to children
    // localX/localY are relative to this component
    // Children's positions are also relative to this component
    for (auto& child : children) {
      if (child && child->isVisible()) {
        int childX, childY, childW, childH;
        child->getBounds(childX, childY, childW, childH);

        // Check if point is within child's bounds
        if (localX >= childX && localX < childX + childW && localY >= childY && localY < childY + childH) {
          // Convert to child-local coordinates
          int childLocalX = localX - childX;
          int childLocalY = localY - childY;
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
        int childX, childY, childW, childH;
        child->getBounds(childX, childY, childW, childH);

        // Check if point is within child's bounds
        if (localX >= childX && localX < childX + childW && localY >= childY && localY < childY + childH) {
          // Convert to child-local coordinates
          int childLocalX = localX - childX;
          int childLocalY = localY - childY;
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
        int childX, childY, childW, childH;
        child->getBounds(childX, childY, childW, childH);

        // Check if point is within child's bounds
        if (localX >= childX && localX < childX + childW && localY >= childY && localY < childY + childH) {
          // Convert to child-local coordinates
          int childLocalX = localX - childX;
          int childLocalY = localY - childY;
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
        int childX, childY;
        child->getPosition(childX, childY);

        // Convert to child-local coordinates
        int childLocalX = localX - childX;
        int childLocalY = localY - childY;
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
        int childX, childY, childW, childH;
        child->getBounds(childX, childY, childW, childH);

        // Check if point is within child's bounds
        if (localX >= childX && localX < childX + childW && localY >= childY && localY < childY + childH) {
          // Convert to child-local coordinates
          int childLocalX = localX - childX;
          int childLocalY = localY - childY;
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
    performLayout();
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

} // namespace bombfork::prong