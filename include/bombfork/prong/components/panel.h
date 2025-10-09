#pragma once

#include <bombfork/prong/core/component.h>
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
 * This is the canonical Panel class for t4c::ui namespace components.
 */
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

  static constexpr int TITLE_BAR_HEIGHT = 25;

public:
  explicit Panel(bombfork::prong::rendering::IRenderer* renderer = nullptr, const std::string& debugName = "Panel")
    : Component(renderer, debugName) {}

  virtual ~Panel() = default;

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

    contentX = x + borderOffset + style.padding;
    contentY = y + borderOffset + titleBarOffset + style.padding;
    contentWidth = width - (borderOffset + style.padding) * 2;
    contentHeight = height - (borderOffset + style.padding) * 2 - titleBarOffset;
  }

  // === Update ===

  void update(double deltaTime) override {
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

    // Render background
    renderer->drawRect(x, y, width, height, style.backgroundColor.r, style.backgroundColor.g, style.backgroundColor.b,
                       style.backgroundColor.a);

    // Render title bar if enabled
    if (hasVisibleTitleBar()) {
      renderTitleBar();
    }

    // Render border
    if (style.showBorder && style.borderWidth > 0) {
      renderBorder();
    }

    // Render children in content area
    renderChildren();
  }

  /**
   * @brief Render just the background (legacy API compatibility)
   */
  void renderBackground() {
    if (!renderer)
      return;

    renderer->drawRect(x, y, width, height, style.backgroundColor.r, style.backgroundColor.g, style.backgroundColor.b,
                       style.backgroundColor.a);
  }

protected:
  /**
   * @brief Render title bar
   */
  virtual void renderTitleBar() {
    if (!renderer)
      return;

    int borderOffset = static_cast<int>(style.borderWidth);
    int titleBarX = x + borderOffset;
    int titleBarY = y + borderOffset;
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

    int bw = static_cast<int>(style.borderWidth);

    // Top border
    renderer->drawRect(x, y, width, bw, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);

    // Bottom border
    renderer->drawRect(x, y + height - bw, width, bw, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);

    // Left border
    renderer->drawRect(x, y, bw, height, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);

    // Right border
    renderer->drawRect(x + width - bw, y, bw, height, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);
  }

  /**
   * @brief Render child components
   */
  virtual void renderChildren() {
    for (auto& child : children) {
      if (child && child->isVisible()) {
        child->render();
      }
    }
  }
};

} // namespace bombfork::prong