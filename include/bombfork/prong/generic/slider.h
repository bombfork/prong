#pragma once

#include "../layout/layout_measurement.h"
#include "../theming/advanced_theme.h"
#include <bombfork/prong/core/component.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

namespace bombfork::prong {

/**
 * @brief Professional slider component for numeric value adjustment
 *
 * Provides comprehensive slider functionality with:
 * - Horizontal and vertical orientations
 * - Integer and floating-point value support
 * - Configurable value ranges and step sizes
 * - Tick marks and value labels
 * - Smooth animations and easing
 * - Professional theming integration
 * - Keyboard navigation (arrow keys, page up/down)
 * - Mouse wheel support
 * - Logarithmic and linear value scaling
 * - Snap-to-tick behavior
 * - Custom value formatting
 *
 * Note: Planned for future implementation. Not currently used in the codebase.
 */
class Slider : public Component {
public:
  enum class Orientation {
    HORIZONTAL, // Left-to-right value adjustment
    VERTICAL    // Bottom-to-top value adjustment
  };

  enum class ValueType {
    INTEGER, // Integer values only
    FLOATING // Floating-point values
  };

  enum class ScaleType {
    LINEAR,     // Linear value distribution
    LOGARITHMIC // Logarithmic value distribution
  };

  enum class TickPosition {
    NONE,  // No tick marks
    ABOVE, // Ticks above/left of slider
    BELOW, // Ticks below/right of slider
    BOTH   // Ticks on both sides
  };

  using ValueChangedCallback = std::function<void(double value)>;
  using ValueFormatterCallback = std::function<std::string(double value)>;

private:
  static constexpr int DEFAULT_TRACK_SIZE = 4;
  static constexpr int DEFAULT_HANDLE_SIZE = 16;
  static constexpr int DEFAULT_TICK_SIZE = 6;
  static constexpr int MIN_SLIDER_LENGTH = 50;
  static constexpr double ANIMATION_DURATION = 0.15; // seconds

  struct SliderState {
    Orientation orientation = Orientation::HORIZONTAL;
    ValueType valueType = ValueType::FLOATING;
    ScaleType scaleType = ScaleType::LINEAR;
    TickPosition tickPosition = TickPosition::NONE;

    double value = 0.0;
    double minValue = 0.0;
    double maxValue = 100.0;
    double stepSize = 1.0;
    double pageSize = 10.0; // For page up/down keys

    bool snapToTicks = false;
    bool showValue = false;
    bool invertedAppearance = false; // Reverse value direction
    bool tracking = true;            // Emit value changes during drag

    // Animation state
    double targetValue = 0.0;
    double animationStartValue = 0.0;
    double animationStartTime = 0.0;
    bool animating = false;

    // Interaction state
    bool dragging = false;
    bool hovering = false;
    int dragStartPosition = 0;
    double dragStartValue = 0.0;
  };

  struct SliderTheme {
    // Track (groove) colors
    bombfork::prong::theming::Color trackColor;
    bombfork::prong::theming::Color trackFillColor;
    bombfork::prong::theming::Color trackBorderColor;

    // Handle (thumb) colors
    bombfork::prong::theming::Color handleColor;
    bombfork::prong::theming::Color handleHoverColor;
    bombfork::prong::theming::Color handleActiveColor;
    bombfork::prong::theming::Color handleBorderColor;

    // Tick and label colors
    bombfork::prong::theming::Color tickColor;
    bombfork::prong::theming::Color labelColor;

    // Visual properties
    int trackSize = DEFAULT_TRACK_SIZE;
    int handleSize = DEFAULT_HANDLE_SIZE;
    int tickSize = DEFAULT_TICK_SIZE;
    float cornerRadius = 2.0f;
    bool showShadow = true;
    float shadowOpacity = 0.3f;

    SliderTheme() {
      // Professional desktop theme defaults
      trackColor = bombfork::prong::theming::Color(0.3f, 0.3f, 0.3f, 1.0f);
      trackFillColor = bombfork::prong::theming::Color(0.2f, 0.4f, 0.8f, 1.0f);
      trackBorderColor = bombfork::prong::theming::Color(0.2f, 0.2f, 0.2f, 1.0f);

      handleColor = bombfork::prong::theming::Color(0.7f, 0.7f, 0.7f, 1.0f);
      handleHoverColor = bombfork::prong::theming::Color(0.8f, 0.8f, 0.8f, 1.0f);
      handleActiveColor = bombfork::prong::theming::Color(0.9f, 0.9f, 0.9f, 1.0f);
      handleBorderColor = bombfork::prong::theming::Color(0.4f, 0.4f, 0.4f, 1.0f);

      tickColor = bombfork::prong::theming::Color(0.6f, 0.6f, 0.6f, 1.0f);
      labelColor = bombfork::prong::theming::Color(0.8f, 0.8f, 0.8f, 1.0f);
    }
  };

  SliderState state;
  SliderTheme theme;

  // Tick configuration
  double majorTickSpacing = 0.0; // 0 = auto-calculate
  double minorTickSpacing = 0.0; // 0 = no minor ticks
  int maxTickCount = 11;         // Maximum number of ticks to show

  // Callbacks
  ValueChangedCallback valueChangedCallback;
  ValueFormatterCallback valueFormatter;

public:
  explicit Slider();
  ~Slider() override = default;

  // === Configuration ===

  /**
   * @brief Set slider orientation
   */
  void setOrientation(Orientation orientation);

  /**
   * @brief Get slider orientation
   */
  Orientation getOrientation() const { return state.orientation; }

  /**
   * @brief Set value type (integer/floating)
   */
  void setValueType(ValueType type);

  /**
   * @brief Get value type
   */
  ValueType getValueType() const { return state.valueType; }

  /**
   * @brief Set scale type (linear/logarithmic)
   */
  void setScaleType(ScaleType type);

  /**
   * @brief Get scale type
   */
  ScaleType getScaleType() const { return state.scaleType; }

  // === Value Management ===

  /**
   * @brief Set current value
   */
  void setValue(double value, bool animate = false);

  /**
   * @brief Get current value
   */
  double getValue() const { return state.value; }

  /**
   * @brief Set value range
   */
  void setRange(double minValue, double maxValue);

  /**
   * @brief Set minimum value
   */
  void setMinimum(double minValue);

  /**
   * @brief Get minimum value
   */
  double getMinimum() const { return state.minValue; }

  /**
   * @brief Set maximum value
   */
  void setMaximum(double maxValue);

  /**
   * @brief Get maximum value
   */
  double getMaximum() const { return state.maxValue; }

  /**
   * @brief Set step size
   */
  void setStepSize(double stepSize);

  /**
   * @brief Get step size
   */
  double getStepSize() const { return state.stepSize; }

  /**
   * @brief Set page size (for page up/down)
   */
  void setPageSize(double pageSize);

  /**
   * @brief Get page size
   */
  double getPageSize() const { return state.pageSize; }

  // === Appearance ===

  /**
   * @brief Set tick mark position
   */
  void setTickPosition(TickPosition position);

  /**
   * @brief Get tick mark position
   */
  TickPosition getTickPosition() const { return state.tickPosition; }

  /**
   * @brief Set tick spacing
   */
  void setTickSpacing(double majorSpacing, double minorSpacing = 0.0);

  /**
   * @brief Enable/disable snap to ticks
   */
  void setSnapToTicks(bool snap);

  /**
   * @brief Check if snap to ticks is enabled
   */
  bool getSnapToTicks() const { return state.snapToTicks; }

  /**
   * @brief Show/hide value label
   */
  void setShowValue(bool show);

  /**
   * @brief Check if value label is shown
   */
  bool getShowValue() const { return state.showValue; }

  /**
   * @brief Set inverted appearance (reverse direction)
   */
  void setInvertedAppearance(bool inverted);

  /**
   * @brief Check if appearance is inverted
   */
  bool getInvertedAppearance() const { return state.invertedAppearance; }

  /**
   * @brief Enable/disable tracking (emit changes during drag)
   */
  void setTracking(bool tracking);

  /**
   * @brief Check if tracking is enabled
   */
  bool getTracking() const { return state.tracking; }

  // === Callbacks ===

  /**
   * @brief Set value changed callback
   */
  void setValueChangedCallback(ValueChangedCallback callback);

  /**
   * @brief Set value formatter callback
   */
  void setValueFormatter(ValueFormatterCallback formatter);

  // === Utility Methods ===

  /**
   * @brief Increase value by step size
   */
  void stepUp();

  /**
   * @brief Decrease value by step size
   */
  void stepDown();

  /**
   * @brief Increase value by page size
   */
  void pageUp();

  /**
   * @brief Decrease value by page size
   */
  void pageDown();

  /**
   * @brief Get integer value (for integer sliders)
   */
  int getIntValue() const { return static_cast<int>(std::round(state.value)); }

  /**
   * @brief Set integer value (for integer sliders)
   */
  void setIntValue(int value, bool animate = false);

  // === Theming ===

  /**
   * @brief Apply theme from AdvancedTheme system
   */
  void applyTheme(const bombfork::prong::theming::AdvancedTheme& theme);

  /**
   * @brief Set custom theme
   */
  void setSliderTheme(const SliderTheme& customTheme);

  /**
   * @brief Get current theme
   */
  const SliderTheme& getSliderTheme() const { return theme; }

  // === UIComponent Overrides ===

  void render() override;
  bool handleClick(int localX, int localY) override;
  bool handleMousePress(int localX, int localY, int button) override;
  bool handleMouseRelease(int localX, int localY, int button) override;
  bool handleMouseMove(int localX, int localY) override;
  bool handleMouseDrag(int localX, int localY, int deltaX, int deltaY) override;
  bool handleKey(int key, int action, int mods) override;
  bool handleScroll(double xOffset, double yOffset) override;
  void setBounds(int x, int y, int width, int height) override;

  // === Layout Integration ===

  bombfork::prong::layout::LayoutMeasurement measurePreferredSize() const override;

private:
  /**
   * @brief Update animation
   */
  void updateAnimation();

  /**
   * @brief Convert pixel position to value
   */
  double positionToValue(int position) const;

  /**
   * @brief Convert value to pixel position
   */
  int valueToPosition(double value) const;

  /**
   * @brief Get handle rectangle
   */
  layout::Rect getHandleRect() const;

  /**
   * @brief Get track rectangle
   */
  layout::Rect getTrackRect() const;

  /**
   * @brief Check if point is in handle
   */
  bool isPointInHandle(int localX, int localY) const;

  /**
   * @brief Constrain value to valid range
   */
  double constrainValue(double value) const;

  /**
   * @brief Apply step size constraint
   */
  double snapToStep(double value) const;

  /**
   * @brief Apply scale transformation
   */
  double applyScale(double value) const;

  /**
   * @brief Apply inverse scale transformation
   */
  double applyInverseScale(double value) const;

  /**
   * @brief Render track (groove)
   */
  void renderTrack();

  /**
   * @brief Render track fill
   */
  void renderTrackFill();

  /**
   * @brief Render handle (thumb)
   */
  void renderHandle();

  /**
   * @brief Render tick marks
   */
  void renderTicks();

  /**
   * @brief Render value label
   */
  void renderValue();

  /**
   * @brief Get tick positions
   */
  std::vector<double> getTickValues() const;

  /**
   * @brief Format value for display
   */
  std::string formatValue(double value) const;

  /**
   * @brief Calculate tick spacing automatically
   */
  double calculateTickSpacing() const;

  /**
   * @brief Get current time for animations
   */
  double getCurrentTime() const;

  /**
   * @brief Ease animation curve
   */
  double easeInOutCubic(double t) const;

  /**
   * @brief Notify value changed
   */
  void notifyValueChanged();

  /**
   * @brief Start animation to target value
   */
  void animateToValue(double targetValue);
};

} // namespace bombfork::prong