#pragma once

#include <bombfork/prong/core/event.h>
#include <bombfork/prong/layout/layout_manager.h>

#include <algorithm>
#include <climits>
#include <cmath>
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

  /**
   * @brief Resize behavior controls how component responds to parent resize
   */
  enum class ResizeBehavior {
    FIXED,          // Keep original size and position
    SCALE,          // Scale proportionally with parent
    FILL,           // Fill available parent space
    MAINTAIN_ASPECT // Scale while maintaining aspect ratio
  };

  /**
   * @brief Constraints for responsive sizing
   */
  struct ResponsiveConstraints {
    int minWidth = 0;
    int minHeight = 0;
    int maxWidth = INT_MAX;
    int maxHeight = INT_MAX;
    float aspectRatio = 0.0f; // 0 = no constraint

    ResponsiveConstraints() = default;
    ResponsiveConstraints(int minW, int minH, int maxW = INT_MAX, int maxH = INT_MAX, float ar = 0.0f)
      : minWidth(minW), minHeight(minH), maxWidth(maxW), maxHeight(maxH), aspectRatio(ar) {}
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
  bool isCurrentlyHovered = false; // Track if mouse is currently over this component

  // Resize handling
  ResizeBehavior resizeBehavior = ResizeBehavior::FIXED;
  ResponsiveConstraints constraints;
  int originalParentWidth = 0;
  int originalParentHeight = 0;
  // Original component dimensions (for SCALE and MAINTAIN_ASPECT behaviors)
  int originalLocalX = 0;
  int originalLocalY = 0;
  int originalWidth = 0;
  int originalHeight = 0;

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
   *
   * Returns the absolute screen coordinates of this component by walking up
   * the parent chain and summing all local positions. The result is cached
   * for performance and automatically invalidated when positions change.
   *
   * For root components (no parent), global position equals local position.
   *
   * @param outX Output: Absolute screen X coordinate
   * @param outY Output: Absolute screen Y coordinate
   *
   * @note This method is const but updates internal cache (mutable members)
   * @see getPosition() for local coordinates
   * @see getGlobalBounds() for position and size together
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
   *
   * Convenience method that returns both global position and size in one call.
   * Equivalent to calling getGlobalPosition() and getSize() separately.
   *
   * @param outX Output: Absolute screen X coordinate
   * @param outY Output: Absolute screen Y coordinate
   * @param outWidth Output: Component width
   * @param outHeight Output: Component height
   *
   * @see getGlobalPosition() for position only
   * @see getBounds() for local coordinates
   */
  void getGlobalBounds(int& outX, int& outY, int& outWidth, int& outHeight) const {
    getGlobalPosition(outX, outY);
    outWidth = width;
    outHeight = height;
  }

  /**
   * @brief Check if a global screen point is within this component
   *
   * Used primarily for hit testing. Tests whether the given screen-space
   * coordinates fall within this component's rectangular bounds.
   *
   * @param globalX Absolute screen X coordinate to test
   * @param globalY Absolute screen Y coordinate to test
   * @return true if point is within component bounds, false otherwise
   *
   * @note Override this method for custom hit testing (e.g., circular bounds)
   */
  bool containsGlobal(int globalX, int globalY) const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    return globalX >= gx && globalX < gx + width && globalY >= gy && globalY < gy + height;
  }

  /**
   * @brief Convert global screen coordinates to component-local coordinates
   *
   * Transforms absolute screen coordinates into coordinates relative to this
   * component's origin. The result represents where the point is within this
   * component's coordinate space (0,0 = top-left corner of component).
   *
   * @param globalX Input: Absolute screen X coordinate
   * @param globalY Input: Absolute screen Y coordinate
   * @param localX Output: X coordinate relative to this component's origin
   * @param localY Output: Y coordinate relative to this component's origin
   *
   * @note Useful for custom event handling or sub-region calculations
   * @see localToGlobal() for the reverse transformation
   */
  void globalToLocal(int globalX, int globalY, int& localX, int& localY) const {
    int gx, gy;
    getGlobalPosition(gx, gy);
    localX = globalX - gx;
    localY = globalY - gy;
  }

  /**
   * @brief Convert component-local coordinates to global screen coordinates
   *
   * Transforms coordinates relative to this component into absolute screen
   * coordinates. Useful for positioning tooltips, context menus, or other
   * UI elements that need screen-space positions.
   *
   * @param localX Input: X coordinate relative to this component's origin
   * @param localY Input: Y coordinate relative to this component's origin
   * @param globalX Output: Absolute screen X coordinate
   * @param globalY Output: Absolute screen Y coordinate
   *
   * @note Useful for positioning external UI elements (tooltips, dialogs)
   * @see globalToLocal() for the reverse transformation
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

  // === Resize Management ===

  /**
   * @brief Set resize behavior for this component
   * @param behavior How component should respond to parent resize
   */
  void setResizeBehavior(ResizeBehavior behavior) { resizeBehavior = behavior; }

  /**
   * @brief Get current resize behavior
   */
  ResizeBehavior getResizeBehavior() const { return resizeBehavior; }

  /**
   * @brief Set responsive constraints for sizing
   * @param newConstraints Min/max size and aspect ratio constraints
   */
  void setConstraints(const ResponsiveConstraints& newConstraints) { constraints = newConstraints; }

  /**
   * @brief Get current constraints
   */
  const ResponsiveConstraints& getConstraints() const { return constraints; }

  /**
   * @brief Called when parent component is resized
   *
   * Override this to implement custom resize behavior. The default
   * implementation uses the ResizeBehavior setting to automatically
   * handle common resize patterns.
   *
   * @param parentWidth New parent width
   * @param parentHeight New parent height
   */
  virtual void onParentResize(int parentWidth, int parentHeight) {
    // Initialize original dimensions if not set (first resize call)
    if (originalParentWidth == 0 && originalParentHeight == 0) {
      originalParentWidth = parentWidth;
      originalParentHeight = parentHeight;
      // Store original component dimensions for SCALE and MAINTAIN_ASPECT
      originalLocalX = localX;
      originalLocalY = localY;
      originalWidth = width;
      originalHeight = height;
    }

    // Apply resize behavior
    switch (resizeBehavior) {
    case ResizeBehavior::FILL: {
      // Fill available parent space
      setBounds(0, 0, parentWidth, parentHeight);
      break;
    }
    case ResizeBehavior::SCALE: {
      // Scale proportionally from ORIGINAL dimensions
      if (originalParentWidth > 0 && originalParentHeight > 0) {
        float scaleX = parentWidth / static_cast<float>(originalParentWidth);
        float scaleY = parentHeight / static_cast<float>(originalParentHeight);

        // Scale from original dimensions, not current ones!
        int newX = static_cast<int>(originalLocalX * scaleX);
        int newY = static_cast<int>(originalLocalY * scaleY);
        int newWidth = static_cast<int>(originalWidth * scaleX);
        int newHeight = static_cast<int>(originalHeight * scaleY);

        setBounds(newX, newY, newWidth, newHeight);
      }
      break;
    }
    case ResizeBehavior::MAINTAIN_ASPECT: {
      // Scale while maintaining aspect ratio from ORIGINAL dimensions
      if (originalParentWidth > 0 && originalParentHeight > 0 && originalWidth > 0 && originalHeight > 0) {
        float currentAspect = originalWidth / static_cast<float>(originalHeight);
        float scale = std::min(parentWidth / static_cast<float>(originalParentWidth),
                               parentHeight / static_cast<float>(originalParentHeight));

        int newWidth = static_cast<int>(originalWidth * scale);
        int newHeight = static_cast<int>(newWidth / currentAspect);

        // Center in available space
        int newX = (parentWidth - newWidth) / 2;
        int newY = (parentHeight - newHeight) / 2;

        setBounds(newX, newY, newWidth, newHeight);
      }
      break;
    }
    case ResizeBehavior::FIXED:
      // Do nothing - keep original size and position
      break;
    }

    // Apply constraints
    applyConstraints();

    // Mark layout as invalid to trigger re-layout
    invalidateLayout();

    // Propagate to children
    for (auto& child : children) {
      if (child) {
        child->onParentResize(width, height);
      }
    }
  }

protected:
  /**
   * @brief Apply responsive constraints to current size
   */
  void applyConstraints() {
    if (constraints.minWidth > 0 || constraints.maxWidth < INT_MAX || constraints.minHeight > 0 ||
        constraints.maxHeight < INT_MAX || constraints.aspectRatio > 0.0f) {

      int newWidth = width;
      int newHeight = height;

      // Apply min/max constraints
      newWidth = std::clamp(newWidth, constraints.minWidth, constraints.maxWidth);
      newHeight = std::clamp(newHeight, constraints.minHeight, constraints.maxHeight);

      // Apply aspect ratio constraint
      if (constraints.aspectRatio > 0.0f) {
        newHeight = static_cast<int>(newWidth / constraints.aspectRatio);
        // Re-check height constraints after aspect ratio adjustment
        newHeight = std::clamp(newHeight, constraints.minHeight, constraints.maxHeight);
      }

      if (newWidth != width || newHeight != height) {
        setSize(newWidth, newHeight);
      }
    }
  }

public:
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

  // === Hierarchical Event API ===

  /**
   * @brief Check if an event is positional (requires coordinate checking)
   *
   * Helper method to determine if an event type requires position-based
   * hit testing via containsEvent().
   *
   * @param type Event type to check
   * @return true if event is positional (mouse events), false otherwise
   */
  bool isPositionalEvent(core::Event::Type type) const {
    return type == core::Event::Type::MOUSE_PRESS || type == core::Event::Type::MOUSE_RELEASE ||
           type == core::Event::Type::MOUSE_MOVE || type == core::Event::Type::MOUSE_SCROLL;
  }

  /**
   * @brief Check if this component contains the event's position
   *
   * Used for hit testing during event propagation. Only relevant for
   * positional events (mouse events). The default implementation checks
   * if the event's local coordinates fall within the component's bounds.
   *
   * Override this method for custom hit testing (e.g., Panel checks content
   * area, circular buttons check radius, etc.).
   *
   * @param event Event to check (localX, localY are relative to this component)
   * @return true if event position is within component bounds, false otherwise
   *
   * @note Only called for positional events (checked via isPositionalEvent())
   * @see Panel::containsEvent() for content area checking example
   */
  virtual bool containsEvent(const core::Event& event) const {
    // Default: check if point is within component's rectangular bounds
    return event.localX >= 0 && event.localX < width && event.localY >= 0 && event.localY < height;
  }

  /**
   * @brief Handle event at this component level (before propagating to children)
   *
   * This method is called by handleEvent() after position checking but before
   * propagating to children. Override this method to implement component-specific
   * event handling without interfering with the default propagation logic.
   *
   * The default implementation returns false, allowing events to propagate to
   * children. Return true to consume the event and prevent propagation.
   *
   * @param event Event to handle (coordinates in local space)
   * @return true if event was handled (stops propagation), false to propagate
   *
   * @note This is the primary method to override for custom event handling
   * @note Coordinates are already converted to local space
   * @see handleEvent() for the full propagation logic
   */
  virtual bool handleEventSelf(const core::Event& event) {
    (void)event; // Unused by default
    return false;
  }

  /**
   * @brief Hierarchical event handling with automatic propagation
   *
   * This is the main entry point for the event API. It implements the
   * complete event propagation logic:
   * 1. Check if component is enabled and visible
   * 2. For positional events, check if event is within bounds via containsEvent()
   * 3. Try handleEventSelf() first (component-specific handling)
   * 4. If not handled, propagate to children in reverse order (z-order)
   * 5. Automatically convert coordinates to child-local space during propagation
   *
   * Most components should override handleEventSelf() instead of this method.
   * Only override handleEvent() if you need custom propagation logic.
   *
   * @param event Event to handle (coordinates in local space)
   * @return true if event was handled by this component or any child, false otherwise
   *
   * @note This is a non-breaking addition - existing event handlers still work
   * @note Children rendered last (topmost) receive events first (reverse order)
   * @see handleEventSelf() for component-specific handling
   * @see containsEvent() for custom hit testing
   */
  virtual bool handleEvent(const core::Event& event) {
    // Step 1: Check if component can receive events
    if (!enabled || !visible) {
      return false;
    }

    // Step 2: For positional events, check if event is within bounds
    bool isWithinBounds = !isPositionalEvent(event.type) || containsEvent(event);

    // Special case for MOUSE_MOVE: deliver hover-out events
    // If mouse moved outside bounds but component was previously hovered,
    // deliver the event so component can reset its hover state
    if (event.type == core::Event::Type::MOUSE_MOVE) {
      if (!isWithinBounds && isCurrentlyHovered) {
        // Mouse moved out - deliver event and reset hover state
        isCurrentlyHovered = false;
        handleEventSelf(event); // Let component handle hover-out
        // Don't return true - this event should continue propagating
      } else if (isWithinBounds) {
        // Mouse is within bounds - mark as hovered
        isCurrentlyHovered = true;
      }
    }

    // For other positional events, reject if outside bounds
    if (!isWithinBounds && event.type != core::Event::Type::MOUSE_MOVE) {
      return false;
    }

    // Step 3: Try handling event at this component level first
    if (isWithinBounds && handleEventSelf(event)) {
      return true;
    }

    // Step 4: Propagate to children in reverse order (z-order: last rendered = topmost)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      auto& child = *it;
      if (child && child->isVisible()) {
        // Get child's local position within this component
        int childX, childY;
        child->getPosition(childX, childY);

        // Create a copy of the event with coordinates converted to child-local space
        core::Event childEvent = event;
        if (isPositionalEvent(event.type)) {
          childEvent.localX = event.localX - childX;
          childEvent.localY = event.localY - childY;
        }

        // Propagate to child
        if (child->handleEvent(childEvent)) {
          // Child handled event - for MOUSE_MOVE, mark this component as not hovered
          // since the child is the one actually under the mouse
          if (event.type == core::Event::Type::MOUSE_MOVE) {
            isCurrentlyHovered = false;
          }
          return true;
        }
      }
    }

    // Event not handled by this component or any children
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