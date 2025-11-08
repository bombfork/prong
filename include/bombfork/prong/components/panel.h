#pragma once

#include <bombfork/prong/core/component.h>
#include <bombfork/prong/layout/layout_manager.h>
#include <bombfork/prong/layout/no_layout.h>
#include <bombfork/prong/rendering/irenderer.h>
#include <bombfork/prong/theming/color.h>

#include <memory>
#include <string>
#include <vector>

namespace bombfork::prong {

/**
 * @brief Modern Panel component for the new UI system
 *
 * A container component that can hold child components with optional
 * title bar, borders, padding, and background styling.
 *
 * Template parameter LayoutT specifies the layout manager type.
 * Use NoLayout (default) for manual positioning, or any layout manager
 * (FlexLayout, GridLayout, etc.) for automatic child positioning.
 *
 * @tparam LayoutT Layout manager type (default: NoLayout for manual positioning)
 */
template <typename LayoutT = layout::NoLayout>
class Panel : public Component {
public:
  struct BorderStyle {
    theming::Color color = theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
    float width = 1.0f;
  };

  struct Style {
    theming::Color backgroundColor = theming::Color(0.2f, 0.2f, 0.2f, 1.0f);
    theming::Color borderColor = theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
    theming::Color titleBarColor = theming::Color(0.15f, 0.15f, 0.15f, 1.0f);
    theming::Color titleTextColor = theming::Color::WHITE();

    BorderStyle border; // Legacy API compatibility
    float borderWidth = 1.0f;
    float cornerRadius = 0.0f;
    int padding = 0;

    bool showBorder = true;
    bool showTitleBar = false;

    // Constructor to keep border in sync
    Style() {
      border.color = borderColor;
      border.width = borderWidth;
    }
  };

public:
  // Legacy API compatibility alias
  Style& styleTraits = style;

private:
  Style style;
  std::string title;
  std::shared_ptr<LayoutT> layoutManager;
  bool autoFillParent = false;

  static constexpr int TITLE_BAR_HEIGHT = 25;

public:
  explicit Panel(const std::string& debugName = "Panel") : Component(nullptr, debugName) {}

  virtual ~Panel() = default;

  // === Layout Management ===

  /**
   * @brief Set the layout manager for this panel
   *
   * This configures how children will be automatically positioned.
   * Only available if LayoutT is not NoLayout.
   */
  template <typename T = LayoutT>
  typename std::enable_if<!std::is_same<T, layout::NoLayout>::value>::type
  setLayoutManager(std::shared_ptr<LayoutT> layout) {
    layoutManager = layout;
    Component::setLayout(layout);
  }

  /**
   * @brief Configure the layout manager
   *
   * Provides access to the layout manager for configuration.
   * Returns nullptr if no layout manager is set or if using NoLayout.
   */
  std::shared_ptr<LayoutT> getLayoutManager() { return layoutManager; }

  /**
   * @brief Enable auto-fill to parent's content area
   *
   * When enabled, the panel will automatically resize to fill its parent's
   * content area (accounting for padding) when width or height is 0.
   */
  void setAutoFillParent(bool enable) { autoFillParent = enable; }

  /**
   * @brief Check if auto-fill parent is enabled
   */
  bool isAutoFillParent() const { return autoFillParent; }

  // === Style Management ===

  /**
   * @brief Set panel style
   */
  void setStyle(const Style& newStyle) { style = newStyle; }

  /**
   * @brief Get current style
   */
  const Style& getStyle() const { return style; }

  /**
   * @brief Set background color
   */
  void setBackgroundColor(const theming::Color& color) { style.backgroundColor = color; }

  /**
   * @brief Set border color
   */
  void setBorderColor(const theming::Color& color) { style.borderColor = color; }

  /**
   * @brief Set border width
   */
  void setBorderWidth(float width) { style.borderWidth = width; }

  /**
   * @brief Set padding
   */
  void setPadding(int padding) { style.padding = padding; }

  // === Title Bar Management ===

  /**
   * @brief Set panel title (enables title bar)
   */
  void setTitle(const std::string& newTitle) {
    title = newTitle;
    style.showTitleBar = !title.empty();
  }

  /**
   * @brief Get panel title
   */
  const std::string& getTitle() const { return title; }

  /**
   * @brief Show/hide title bar
   */
  void setShowTitleBar(bool show) { style.showTitleBar = show; }

  /**
   * @brief Enable/disable title bar (legacy API compatibility)
   */
  void setTitleBarEnabled(bool enabled) { setShowTitleBar(enabled); }

  /**
   * @brief Check if title bar is visible
   */
  bool hasVisibleTitleBar() const { return style.showTitleBar && !title.empty(); }

  // === Content Area ===

  /**
   * @brief Get content area bounds (accounting for borders, padding, title bar)
   */
  void getContentBounds(int& contentX, int& contentY, int& contentWidth, int& contentHeight) const {
    int borderOffset = static_cast<int>(style.borderWidth);
    int titleBarOffset = hasVisibleTitleBar() ? TITLE_BAR_HEIGHT : 0;

    int gx = getGlobalX();
    int gy = getGlobalY();
    contentX = gx + borderOffset + style.padding;
    contentY = gy + borderOffset + titleBarOffset + style.padding;
    contentWidth = width - (borderOffset + style.padding) * 2;
    contentHeight = height - (borderOffset + style.padding) * 2 - titleBarOffset;
  }

  // === Parent/Child Management ===

  /**
   * @brief Add a child component
   *
   * Overridden to ensure renderer is inherited and auto-fill is applied
   */
  void addChild(std::unique_ptr<Component> child) {
    if (child) {
      // Set renderer on child before adding
      child->setRenderer(renderer);

      // Call parent implementation
      Component::addChild(std::move(child));
    }
  }

  // === Event Handling ===

  /**
   * @brief Check if event is within content area (not border/padding/titlebar)
   *
   * Override of Component::containsEvent() to account for Panel's border,
   * padding, and title bar. Events outside the content area are rejected.
   *
   * @param event Event to check (localX, localY are relative to Panel's origin)
   * @return true if event is within content area, false otherwise
   */
  bool containsEvent(const core::Event& event) const override {
    // Calculate content area offset and size
    int borderOffset = static_cast<int>(style.borderWidth);
    int titleBarOffset = hasVisibleTitleBar() ? TITLE_BAR_HEIGHT : 0;
    int contentOffsetX = borderOffset + style.padding;
    int contentOffsetY = borderOffset + titleBarOffset + style.padding;
    int contentWidth = width - (borderOffset + style.padding) * 2;
    int contentHeight = height - (borderOffset + style.padding) * 2 - titleBarOffset;

    // Check if event is within content area (local coordinates)
    return event.localX >= contentOffsetX && event.localX < contentOffsetX + contentWidth &&
           event.localY >= contentOffsetY && event.localY < contentOffsetY + contentHeight;
  }

  /**
   * @brief Handle events for Panel itself
   *
   * Panels are containers that propagate events to children.
   * This method returns false to allow all events to propagate.
   *
   * @param event Event to handle (coordinates in local space)
   * @return false - panels always propagate events
   */
  bool handleEventSelf(const core::Event& event) override {
    (void)event;  // Suppress unused parameter warning
    return false; // Always propagate to children
  }

  // === Minimum Size ===

  /**
   * @brief Get minimum width for panel
   * @return Minimum width based on layout manager measurements plus borders and padding
   */
  int getMinimumWidth() const override {
    if (layoutManager || Component::hasLayout()) {
      // Create vector of raw pointers to children (const-correct)
      std::vector<bombfork::prong::Component*> childPointers;
      childPointers.reserve(children.size());

      for (const auto& child : children) {
        if (child) {
          childPointers.push_back(const_cast<bombfork::prong::Component*>(child.get()));
        }
      }

      // Measure layout and add border + padding
      layout::Dimensions layoutSize{0, 0};

      if (layoutManager) {
        // Use Panel's typed layout manager
        layoutSize = layoutManager->measureLayout(childPointers);
      } else if (Component::measureFunc) {
        // Use Component's type-erased measurement function
        layoutSize = Component::measureFunc(childPointers);
      }

      int borderWidth = static_cast<int>(style.borderWidth);
      return layoutSize.width + (borderWidth + style.padding) * 2;
    }
    return 0;
  }

  /**
   * @brief Get minimum height for panel
   * @return Minimum height based on layout manager measurements plus borders, padding, and title bar
   *
   * @note For wrapping layouts (like FlowLayout), if the panel's width is already set,
   * this uses constrained measurement to calculate accurate wrapped height.
   */
  int getMinimumHeight() const override {
    if (layoutManager || Component::hasLayout()) {
      // Create vector of raw pointers to children (const-correct)
      std::vector<bombfork::prong::Component*> childPointers;
      childPointers.reserve(children.size());

      for (const auto& child : children) {
        if (child) {
          childPointers.push_back(const_cast<bombfork::prong::Component*>(child.get()));
        }
      }

      // Measure layout and add border + padding + title bar
      layout::Dimensions layoutSize{0, 0};

      // Calculate available content width (if panel width is set)
      int borderWidth = static_cast<int>(style.borderWidth);
      int contentWidth = width - (borderWidth + style.padding) * 2;

      // Use constrained measurement if width is set and positive and layout manager exists
      // This is critical for wrapping layouts (like FlowLayout) that need width to calculate height
      if (width > 0 && contentWidth > 0 && layoutManager) {
        layout::Dimensions constraints{contentWidth, 0}; // Height is unknown, that's what we're calculating

        // Use constrained measurement (will fall back to unconstrained if not supported)
        layoutSize = layoutManager->measureLayoutConstrained(childPointers, constraints);
      } else {
        // Fall back to unconstrained measurement
        if (layoutManager) {
          // Use Panel's typed layout manager
          layoutSize = layoutManager->measureLayout(childPointers);
        } else if (Component::measureFunc) {
          // Use Component's type-erased measurement function
          layoutSize = Component::measureFunc(childPointers);
        }
      }

      int titleBarHeight = hasVisibleTitleBar() ? TITLE_BAR_HEIGHT : 0;
      return layoutSize.height + (borderWidth + style.padding) * 2 + titleBarHeight;
    }
    return 0;
  }

  // === Layout ===

  /**
   * @brief Perform layout on children, accounting for content area
   *
   * Overrides Component::performLayout() to ensure children are positioned
   * relative to the content area (after borders, padding, and title bar).
   */
  void performLayout() override {
    // Check if we have a layout function (set via Component::setLayout or Panel::setLayoutManager)
    // Use layoutFunc from Component base class, not Panel's layoutManager member
    if (!layoutFunc) {
      // No layout manager, skip layout
      return;
    }

    if (!layoutInvalid) {
      return;
    }

    // Mark layout as valid first to avoid infinite recursion
    layoutInvalid = false;

    // Calculate content area offset RELATIVE to this Panel's origin
    int borderOffset = static_cast<int>(style.borderWidth);
    int titleBarOffset = hasVisibleTitleBar() ? TITLE_BAR_HEIGHT : 0;
    int contentOffsetX = borderOffset + style.padding;
    int contentOffsetY = borderOffset + titleBarOffset + style.padding;
    int contentWidth = width - (borderOffset + style.padding) * 2;
    int contentHeight = height - (borderOffset + style.padding) * 2 - titleBarOffset;

    // Create vector of raw pointers to children
    std::vector<bombfork::prong::Component*> childPointers;
    childPointers.reserve(children.size());

    for (auto& child : children) {
      if (child) {
        childPointers.push_back(child.get());
      }
    }

    // Create available space from content area dimensions
    layout::Dimensions availableSpace{contentWidth, contentHeight};

    // Call the layout manager through the type-erased function
    // Layout managers position children relative to (0,0) and call setBounds()
    // which now sets LOCAL coordinates (relative to parent's origin).
    // Layout managers produce coordinates relative to content area (0,0),
    // so we need to offset them by the content area's position within this Panel.
    layoutFunc(childPointers, availableSpace);

    // Adjust child positions to account for content area offset within the Panel
    // Layout managers position children relative to content area (0,0),
    // but setBounds() expects coordinates relative to Panel's origin (0,0)
    for (auto& child : children) {
      if (child) {
        int childX, childY, childW, childH;
        child->getBounds(childX, childY, childW, childH);
        // Offset by content area position within the Panel
        child->setBounds(childX + contentOffsetX, childY + contentOffsetY, childW, childH);
      }
    }

    // Recursively perform layout on children
    for (auto& child : children) {
      if (child) {
        child->performLayout();
      }
    }
  }

  // === Resize Handling ===

  /**
   * @brief Handle parent resize events
   *
   * Panels can automatically fill parent or use standard resize behavior.
   * This override checks for auto-fill first, then delegates to Component.
   *
   * @param parentWidth New parent width
   * @param parentHeight New parent height
   */
  void onParentResize(int parentWidth, int parentHeight) override {
    // If auto-fill is enabled, use FILL behavior regardless of setting
    if (autoFillParent) {
      // Try to get content bounds from parent if it's a Panel
      Panel* parentPanel = dynamic_cast<Panel*>(parent);
      if (parentPanel) {
        int contentX, contentY, contentWidth, contentHeight;
        parentPanel->getContentBounds(contentX, contentY, contentWidth, contentHeight);

        // Convert parent's global content bounds to local coordinates
        int parentGx, parentGy;
        parent->getGlobalPosition(parentGx, parentGy);
        int localContentX = contentX - parentGx;
        int localContentY = contentY - parentGy;

        setBounds(localContentX, localContentY, contentWidth, contentHeight);
      } else {
        // Parent is not a Panel, fill entire parent space
        setBounds(0, 0, parentWidth, parentHeight);
      }

      // Mark layout as invalid to trigger re-layout
      invalidateLayout();

      // Propagate to children
      for (auto& child : children) {
        if (child) {
          child->onParentResize(width, height);
        }
      }
    } else {
      // Use standard resize behavior from Component
      Component::onParentResize(parentWidth, parentHeight);
    }
  }

  // === Update ===

  void update(double deltaTime) override {
    // Apply auto-fill if enabled
    if (autoFillParent && parent) {
      applyAutoFill();
    }

    (void)deltaTime; // Unused - panels are static containers
  }

  // === Rendering ===

  /**
   * @brief Render the panel
   */
  void render() override {
    if (!visible || !renderer) {
      return;
    }

    // Note: performLayout() is called by Component::renderAll() before render()
    // so we don't need to call it here

    int gx = getGlobalX();
    int gy = getGlobalY();

    // Render background
    renderer->drawRect(gx, gy, width, height, style.backgroundColor.r, style.backgroundColor.g, style.backgroundColor.b,
                       style.backgroundColor.a);

    // Render title bar if enabled
    if (hasVisibleTitleBar()) {
      renderTitleBar();
    }

    // Render border
    if (style.showBorder && style.borderWidth > 0) {
      renderBorder();
    }

    // Note: Children are rendered by Component::renderAll() after render()
    // so we don't need to call renderChildren() here
  }

  /**
   * @brief Render just the background (legacy API compatibility)
   */
  void renderBackground() {
    if (!renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();
    renderer->drawRect(gx, gy, width, height, style.backgroundColor.r, style.backgroundColor.g, style.backgroundColor.b,
                       style.backgroundColor.a);
  }

protected:
  /**
   * @brief Apply auto-fill behavior to match parent's content area
   */
  void applyAutoFill() {
    if (!parent || (width != 0 && height != 0)) {
      return; // Only fill if width or height is 0
    }

    int currentX, currentY;
    getPosition(currentX, currentY);

    // Try to get content bounds from parent if it's a Panel
    Panel* parentPanel = dynamic_cast<Panel*>(parent);
    if (parentPanel) {
      int contentX, contentY, contentWidth, contentHeight;
      parentPanel->getContentBounds(contentX, contentY, contentWidth, contentHeight);

      // Convert parent's global content bounds to local coordinates
      int parentGx, parentGy;
      parent->getGlobalPosition(parentGx, parentGy);
      int localContentX = contentX - parentGx;
      int localContentY = contentY - parentGy;

      if (width == 0) {
        currentX = localContentX;
        width = contentWidth;
      }
      if (height == 0) {
        currentY = localContentY;
        height = contentHeight;
      }
    } else {
      // Parent is not a Panel, use parent's full bounds (position 0,0 relative to parent)
      if (width == 0) {
        currentX = 0;
        int pw, ph;
        parent->getSize(pw, ph);
        width = pw;
      }
      if (height == 0) {
        currentY = 0;
        int pw, ph;
        parent->getSize(pw, ph);
        height = ph;
      }
    }

    setPosition(currentX, currentY);
  }

  /**
   * @brief Render title bar
   */
  virtual void renderTitleBar() {
    if (!renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();
    int borderOffset = static_cast<int>(style.borderWidth);
    int titleBarX = gx + borderOffset;
    int titleBarY = gy + borderOffset;
    int titleBarWidth = width - borderOffset * 2;

    // Title bar background
    renderer->drawRect(titleBarX, titleBarY, titleBarWidth, TITLE_BAR_HEIGHT, style.titleBarColor.r,
                       style.titleBarColor.g, style.titleBarColor.b, style.titleBarColor.a);

    // Title text
    if (!title.empty()) {
      auto [textWidth, textHeight] = renderer->measureText(title);
      renderer->drawText(title, titleBarX + 8, titleBarY + (TITLE_BAR_HEIGHT - textHeight) / 2, style.titleTextColor.r,
                         style.titleTextColor.g, style.titleTextColor.b, style.titleTextColor.a);
    }
  }

  /**
   * @brief Render border
   */
  virtual void renderBorder() {
    if (!renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();
    int bw = static_cast<int>(style.borderWidth);

    // Top border
    renderer->drawRect(gx, gy, width, bw, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);

    // Bottom border
    renderer->drawRect(gx, gy + height - bw, width, bw, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);

    // Left border
    renderer->drawRect(gx, gy, bw, height, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);

    // Right border
    renderer->drawRect(gx + width - bw, gy, bw, height, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);
  }
};

} // namespace bombfork::prong
