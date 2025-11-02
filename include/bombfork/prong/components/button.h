#pragma once

#include <bombfork/prong/core/component.h>
#include <bombfork/prong/rendering/irenderer.h>
#include <bombfork/prong/theming/color.h>

#include <algorithm>
#include <functional>
#include <string>

namespace bombfork::prong {

/**
 * @brief Modern Button component for the new UI system
 *
 * A clickable button with text, hover states, and callback support.
 */
class Button : public Component {
public:
  using ClickCallback = std::function<void()>;

  enum class ButtonType {
    NORMAL, // Regular button - triggers callback on click
    TOGGLE  // Toggle button - maintains active/inactive state
  };

  struct Style {
    theming::Color backgroundColor = theming::Color(0.3f, 0.3f, 0.3f, 1.0f);
    theming::Color hoverColor = theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
    theming::Color pressedColor = theming::Color(0.25f, 0.25f, 0.25f, 1.0f);
    theming::Color activeColor = theming::Color(0.2f, 0.6f, 0.3f, 1.0f); // Green for active/toggled state
    theming::Color disabledColor = theming::Color(0.2f, 0.2f, 0.2f, 1.0f);
    theming::Color textColor = theming::Color::WHITE();
    theming::Color disabledTextColor = theming::Color(0.5f, 0.5f, 0.5f, 1.0f);
    theming::Color borderColor = theming::Color(0.5f, 0.5f, 0.5f, 1.0f);
    theming::Color activeBorderColor = theming::Color(0.3f, 0.8f, 0.4f, 1.0f); // Brighter green for active border

    float borderWidth = 1.0f;
    float cornerRadius = 0.0f;
    int paddingX = 16;
    int paddingY = 8;

    bool showBorder = true;
  };

  enum class State { NORMAL, HOVER, PRESSED, DISABLED };

private:
  Style style;
  std::string text;
  State state = State::NORMAL;
  bool isPressed = false;
  bool isActive = false; // For toggle buttons
  ButtonType buttonType = ButtonType::NORMAL;
  ClickCallback clickCallback;

public:
  explicit Button(const std::string& label = "Button") : Component(nullptr, label), text(label) {}

  virtual ~Button() = default;

  // === Text Management ===

  /**
   * @brief Set button text
   */
  void setText(const std::string& newText) { text = newText; }

  /**
   * @brief Get button text
   */
  const std::string& getText() const { return text; }

  // === Style Management ===

  /**
   * @brief Set button style
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
   * @brief Set text color
   */
  void setTextColor(const theming::Color& color) { style.textColor = color; }

  // === State Management ===

  /**
   * @brief Get current button state
   */
  State getState() const { return state; }

  /**
   * @brief Set button state
   */
  void setState(State newState) { state = newState; }

  /**
   * @brief Set button type (NORMAL or TOGGLE)
   */
  void setButtonType(ButtonType type) { buttonType = type; }

  /**
   * @brief Get button type
   */
  ButtonType getButtonType() const { return buttonType; }

  /**
   * @brief Set active state (for toggle buttons)
   */
  void setActive(bool active) { isActive = active; }

  /**
   * @brief Get active state (for toggle buttons)
   */
  bool isToggleActive() const { return isActive; }

  // === Callback Management ===

  /**
   * @brief Set click callback
   */
  void setClickCallback(ClickCallback callback) { clickCallback = std::move(callback); }

  // === Minimum Size ===

  /**
   * @brief Get minimum width for button
   * @return Minimum width based on text size or fallback for icon-only buttons
   */
  int getMinimumWidth() const override {
    if (!text.empty() && renderer) {
      auto [textWidth, textHeight] = renderer->measureText(text);
      int borderWidth = style.showBorder ? static_cast<int>(style.borderWidth) : 0;
      return textWidth + (style.paddingX * 2) + (borderWidth * 2);
    }
    return 50; // Fallback for icon-only buttons
  }

  /**
   * @brief Get minimum height for button
   * @return Minimum height based on text size or fallback for icon-only buttons
   */
  int getMinimumHeight() const override {
    if (!text.empty() && renderer) {
      auto [textWidth, textHeight] = renderer->measureText(text);
      int borderWidth = style.showBorder ? static_cast<int>(style.borderWidth) : 0;
      return textHeight + (style.paddingY * 2) + (borderWidth * 2);
    }
    return 30; // Fallback for icon-only buttons
  }

  // === Event Handling ===

  /**
   * @brief Handle events using the hierarchical event API
   * @param event Event to handle (coordinates in local space)
   * @return true if event was consumed, false to allow propagation
   */
  bool handleEventSelf(const core::Event& event) override {
    if (!enabled || state == State::DISABLED) {
      return false;
    }

    switch (event.type) {
    case core::Event::Type::MOUSE_PRESS:
      if (event.button == 0) { // Left click
        isPressed = true;
        state = State::PRESSED;
        return true; // Consume the press event
      }
      break;

    case core::Event::Type::MOUSE_RELEASE:
      if (isPressed && event.button == 0) {
        isPressed = false;
        bool stillInside = isPointInside(event.localX, event.localY);
        state = stillInside ? State::HOVER : State::NORMAL;

        // Trigger click callback if mouse was released while still over button
        if (stillInside) {
          // Toggle mode: flip active state
          if (buttonType == ButtonType::TOGGLE) {
            isActive = !isActive;
          }

          // Trigger callback
          if (clickCallback) {
            clickCallback();
          }
        }
        return true; // Consume the release event
      }
      break;

    case core::Event::Type::MOUSE_MOVE: {
      bool inside = isPointInside(event.localX, event.localY);

      if (isPressed) {
        // If button is pressed, update visual state based on whether mouse is inside
        state = inside ? State::PRESSED : State::NORMAL;
      } else {
        // Update hover state
        state = inside ? State::HOVER : State::NORMAL;
      }

      return inside; // Consume move events while over button
    }

    default:
      break;
    }

    return false;
  }

  // === Update ===

  void update(double deltaTime) override {
    // Button doesn't need per-frame updates
    // State changes are handled by event handlers
    (void)deltaTime; // Suppress unused parameter warning
  }

  // === Rendering ===

  void render() override {
    if (!visible || !renderer) {
      return;
    }

    // Determine button color based on state
    // Priority: disabled > pressed > hover > active > normal
    theming::Color bgColor;
    theming::Color txtColor;
    theming::Color borderColor = style.borderColor;

    if (!enabled || state == State::DISABLED) {
      bgColor = style.disabledColor;
      txtColor = style.disabledTextColor;
    } else if (state == State::PRESSED) {
      // Pressed state - use darker version of current base color
      if (isActive) {
        bgColor = theming::Color(style.activeColor.r * 0.75f, style.activeColor.g * 0.75f, style.activeColor.b * 0.75f,
                                 style.activeColor.a);
        borderColor = style.activeBorderColor;
      } else {
        bgColor = style.pressedColor;
      }
      txtColor = style.textColor;
    } else if (state == State::HOVER) {
      // Hover state - brighten the current base color
      if (isActive) {
        // Active button being hovered - brighten the active color
        bgColor = theming::Color(std::min(1.0f, style.activeColor.r * 1.3f), std::min(1.0f, style.activeColor.g * 1.3f),
                                 std::min(1.0f, style.activeColor.b * 1.3f), style.activeColor.a);
        borderColor = style.activeBorderColor;
      } else {
        // Inactive button being hovered
        bgColor = style.hoverColor;
      }
      txtColor = style.textColor;
    } else if (isActive) {
      // Active/toggled state (not hovering or pressed)
      bgColor = style.activeColor;
      txtColor = style.textColor;
      borderColor = style.activeBorderColor;
    } else {
      // Normal state
      bgColor = style.backgroundColor;
      txtColor = style.textColor;
    }

    // Render background
    int gx = getGlobalX();
    int gy = getGlobalY();
    renderer->drawRect(gx, gy, width, height, bgColor.r, bgColor.g, bgColor.b, bgColor.a);

    // Render border
    if (style.showBorder && style.borderWidth > 0) {
      renderBorder(borderColor);
    }

    // Render text
    if (!text.empty()) {
      // Measure text for proper centering
      auto [textWidth, textHeight] = renderer->measureText(text);
      int textX = gx + (width - textWidth) / 2;
      int textY = gy + (height - textHeight) / 2;

      renderer->drawText(text, textX, textY, txtColor.r, txtColor.g, txtColor.b, txtColor.a);
    }
  }

protected:
  /**
   * @brief Render button border with specific color
   */
  virtual void renderBorder(const theming::Color& borderColor) {
    if (!renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();
    int bw = static_cast<int>(style.borderWidth);

    // Top border
    renderer->drawRect(gx, gy, width, bw, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Bottom border
    renderer->drawRect(gx, gy + height - bw, width, bw, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Left border
    renderer->drawRect(gx, gy, bw, height, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Right border
    renderer->drawRect(gx + width - bw, gy, bw, height, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
  }

  /**
   * @brief Check if point is inside button
   * @param localX X coordinate relative to the button's coordinate system
   * @param localY Y coordinate relative to the button's coordinate system
   * @note Since event handlers receive coordinates already converted to local
   * space, we check against (0,0) origin, not the button's absolute (x,y)
   * position
   */
  bool isPointInside(int localX, int localY) const {
    return localX >= 0 && localX < width && localY >= 0 && localY < height;
  }
};

} // namespace bombfork::prong
