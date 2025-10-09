#pragma once

#include <bombfork/prong/core/component.h>
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
  explicit Button(bombfork::prong::rendering::IRenderer* renderer = nullptr, const std::string& label = "Button")
    : Component(renderer, label), text(label) {}

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

  // === Event Handling ===

  bool handleClick(int localX, int localY) override {
    if (!enabled || state == State::DISABLED) {
      return false;
    }

    if (isPointInside(localX, localY)) {
      // Toggle mode: flip active state
      if (buttonType == ButtonType::TOGGLE) {
        isActive = !isActive;
      }

      // Trigger callback
      if (clickCallback) {
        clickCallback();
      }
      return true;
    }
    return false;
  }

  bool handleMousePress(int localX, int localY, int button) override {
    if (!enabled || state == State::DISABLED) {
      return false;
    }

    if (isPointInside(localX, localY) && button == 0) { // Left click
      isPressed = true;
      state = State::PRESSED;
      return true;
    }
    return false;
  }

  bool handleMouseRelease(int localX, int localY, int button) override {
    if (!enabled || state == State::DISABLED) {
      return false;
    }

    if (isPressed && button == 0) {
      isPressed = false;
      state = isPointInside(localX, localY) ? State::HOVER : State::NORMAL;
      return true;
    }
    return false;
  }

  bool handleMouseMove(int localX, int localY) override {
    if (!enabled || state == State::DISABLED) {
      return false;
    }

    bool inside = isPointInside(localX, localY);

    if (isPressed) {
      state = inside ? State::PRESSED : State::NORMAL;
    } else {
      state = inside ? State::HOVER : State::NORMAL;
    }

    return inside;
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
    renderer->drawRect(x, y, width, height, bgColor.r, bgColor.g, bgColor.b, bgColor.a);

    // Render border
    if (style.showBorder && style.borderWidth > 0) {
      renderBorder(borderColor);
    }

    // Render text
    if (!text.empty()) {
      // Measure text for proper centering
      auto [textWidth, textHeight] = renderer->measureText(text);
      int textX = x + (width - textWidth) / 2;
      int textY = y + (height - textHeight) / 2;

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

    int bw = static_cast<int>(style.borderWidth);

    // Top border
    renderer->drawRect(x, y, width, bw, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Bottom border
    renderer->drawRect(x, y + height - bw, width, bw, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Left border
    renderer->drawRect(x, y, bw, height, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Right border
    renderer->drawRect(x + width - bw, y, bw, height, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
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
