#pragma once

#include <bombfork/prong/core/component_traits.h>

// Backward compatibility - import new namespace types
namespace t4c::client::files {
using Padding = bombfork::prong::Padding;
using Border = bombfork::prong::Border;
using Color = bombfork::prong::theming::Color;
} // namespace t4c::client::files
#include <bombfork/prong/theming/color.h>

#include <functional>
#include <string>
#include <unordered_map>

namespace bombfork::prong::theming {

/**
 * @brief Font configuration for theme system
 */
struct FontConfig {
  std::string family = "Arial";
  int baseSize = 12;
  int weight = 400; // 400 = normal, 600 = semibold, 700 = bold

  constexpr FontConfig() = default;
  constexpr FontConfig(const std::string& family, int size, int weight = 400)
    : family(family), baseSize(size), weight(weight) {}

  // Scale font size based on DPI
  int getScaledSize(float dpiScale) const { return static_cast<int>(baseSize * dpiScale); }
};

/**
 * @brief Animation configuration for theme system
 */
struct AnimationConfig {
  float hoverDuration = 0.15f;          // Hover transition duration
  float clickDuration = 0.1f;           // Click feedback duration
  float themeTransitionDuration = 0.3f; // Theme switching duration

  // Easing functions (future extension point)
  enum class EasingType { LINEAR, EASE_IN, EASE_OUT, EASE_IN_OUT };
  EasingType defaultEasing = EasingType::EASE_OUT;

  constexpr AnimationConfig() = default;
};

/**
 * @brief Shadow and elevation definitions
 */
struct ShadowConfig {
  bombfork::prong::theming::Color color = {0.0f, 0.0f, 0.0f, 0.2f};
  float offsetX = 0.0f;
  float offsetY = 2.0f;
  float blurRadius = 4.0f;
  float spreadRadius = 0.0f;

  constexpr ShadowConfig() = default;
  constexpr ShadowConfig(const bombfork::prong::theming::Color& color, float offsetX, float offsetY, float blur)
    : color(color), offsetX(offsetX), offsetY(offsetY), blurRadius(blur) {}

  // Predefined shadow levels
  static ShadowConfig None() { return {bombfork::prong::theming::Color::TRANSPARENT(), 0, 0, 0}; }
  static ShadowConfig Light() { return {{0.0f, 0.0f, 0.0f, 0.1f}, 0, 1, 3}; }
  static ShadowConfig Medium() { return {{0.0f, 0.0f, 0.0f, 0.15f}, 0, 2, 6}; }
  static ShadowConfig Heavy() { return {{0.0f, 0.0f, 0.0f, 0.25f}, 0, 4, 8}; }
};

/**
 * @brief Semantic color system for consistent theming
 */
struct SemanticColors {
  // Base colors
  bombfork::prong::theming::Color primary = bombfork::prong::theming::Color::BLUE();         // Primary brand color
  bombfork::prong::theming::Color secondary = bombfork::prong::theming::Color::LIGHT_GRAY(); // Secondary accent color
  bombfork::prong::theming::Color success = bombfork::prong::theming::Color::GREEN();        // Success/positive actions
  bombfork::prong::theming::Color warning = {1.0f, 0.8f, 0.0f, 1.0f};                        // Warning/caution
  bombfork::prong::theming::Color danger = bombfork::prong::theming::Color::RED(); // Error/destructive actions
  bombfork::prong::theming::Color info = {0.2f, 0.6f, 1.0f, 1.0f};                 // Information/neutral

  // Surface colors
  bombfork::prong::theming::Color surface = bombfork::prong::theming::Color::WHITE(); // Main surface background
  bombfork::prong::theming::Color background = {0.98f, 0.98f, 0.98f, 1.0f};           // App background
  bombfork::prong::theming::Color overlay = {0.0f, 0.0f, 0.0f, 0.5f};                 // Modal overlays

  // Text colors
  bombfork::prong::theming::Color textPrimary = bombfork::prong::theming::Color::BLACK();       // Primary text
  bombfork::prong::theming::Color textSecondary = bombfork::prong::theming::Color::DARK_GRAY(); // Secondary text
  bombfork::prong::theming::Color textDisabled = bombfork::prong::theming::Color::GRAY();       // Disabled text
  bombfork::prong::theming::Color textOnPrimary = bombfork::prong::theming::Color::WHITE();     // Text on primary color

  // Border and separator colors
  bombfork::prong::theming::Color border = bombfork::prong::theming::Color::LIGHT_GRAY(); // Default borders
  bombfork::prong::theming::Color borderFocus = bombfork::prong::theming::Color::BLUE();  // Focused element borders
  bombfork::prong::theming::Color separator = {0.9f, 0.9f, 0.9f, 1.0f};                   // Visual separators

  // Interactive states
  bombfork::prong::theming::Color hover = {0.0f, 0.0f, 0.0f, 0.05f};   // Hover overlay
  bombfork::prong::theming::Color pressed = {0.0f, 0.0f, 0.0f, 0.1f};  // Pressed overlay
  bombfork::prong::theming::Color selected = {0.2f, 0.4f, 1.0f, 0.1f}; // Selection background
  bombfork::prong::theming::Color focus = {0.2f, 0.4f, 1.0f, 0.2f};    // Focus highlight

  // Predefined semantic color schemes
  static SemanticColors Light() {
    SemanticColors colors;
    // Light theme uses defaults above
    return colors;
  }

  static SemanticColors Dark() {
    SemanticColors colors;
    colors.surface = {0.18f, 0.18f, 0.18f, 1.0f};
    colors.background = {0.13f, 0.13f, 0.13f, 1.0f};
    colors.textPrimary = {0.95f, 0.95f, 0.95f, 1.0f};
    colors.textSecondary = {0.7f, 0.7f, 0.7f, 1.0f};
    colors.textDisabled = {0.5f, 0.5f, 0.5f, 1.0f};
    colors.border = {0.3f, 0.3f, 0.3f, 1.0f};
    colors.separator = {0.25f, 0.25f, 0.25f, 1.0f};
    colors.hover = {1.0f, 1.0f, 1.0f, 0.05f};
    colors.pressed = {1.0f, 1.0f, 1.0f, 0.1f};
    return colors;
  }

  static SemanticColors HighContrast() {
    SemanticColors colors;
    colors.surface = bombfork::prong::theming::Color::WHITE();
    colors.background = bombfork::prong::theming::Color::WHITE();
    colors.textPrimary = bombfork::prong::theming::Color::BLACK();
    colors.textSecondary = bombfork::prong::theming::Color::BLACK();
    colors.border = bombfork::prong::theming::Color::BLACK();
    colors.separator = bombfork::prong::theming::Color::BLACK();
    colors.primary = {0.0f, 0.0f, 1.0f, 1.0f}; // Pure blue
    colors.danger = {1.0f, 0.0f, 0.0f, 1.0f};  // Pure red
    colors.success = {0.0f, 0.8f, 0.0f, 1.0f}; // Pure green
    return colors;
  }
};

/**
 * @brief Advanced theme definition extending the basic ComponentTheme
 */
class AdvancedTheme {
public:
  // Theme identification
  std::string name;
  std::string displayName;
  std::string description;
  std::string author = "T4C Editor";
  std::string version = "1.0";

  // Core theme components
  SemanticColors colors;
  FontConfig fonts;
  AnimationConfig animations;

  // Spacing and sizing
  t4c::client::files::Padding basePadding{8};
  t4c::client::files::Border baseBorder{1};
  int baseRadius = 4;          // Border radius for rounded corners
  float baseLineHeight = 1.4f; // Text line height multiplier

  // Shadow definitions for different elevations
  ShadowConfig elevation[6] = {
    ShadowConfig::None(),   // Level 0: No elevation
    ShadowConfig::Light(),  // Level 1: Subtle elevation
    ShadowConfig::Light(),  // Level 2: Card elevation
    ShadowConfig::Medium(), // Level 3: Modal elevation
    ShadowConfig::Medium(), // Level 4: Dropdown elevation
    ShadowConfig::Heavy()   // Level 5: Tooltip elevation
  };

public:
  /**
   * @brief Constructor with theme name
   */
  explicit AdvancedTheme(const std::string& name = "Default", const std::string& displayName = "Default Theme")
    : name(name), displayName(displayName) {}

  // Predefined themes
  static AdvancedTheme Light() {
    AdvancedTheme theme("light", "Light Professional");
    theme.description = "Clean light theme for professional development";
    theme.colors = SemanticColors::Light();
    return theme;
  }

  static AdvancedTheme Dark() {
    AdvancedTheme theme("dark", "Dark Professional");
    theme.description = "Modern dark theme inspired by VS Code";
    theme.colors = SemanticColors::Dark();
    theme.colors.primary = {0.0f, 0.47f, 0.8f, 1.0f}; // VS Code blue
    return theme;
  }

  static AdvancedTheme HighContrast() {
    AdvancedTheme theme("high_contrast", "High Contrast");
    theme.description = "High contrast theme for accessibility";
    theme.colors = SemanticColors::HighContrast();
    theme.baseBorder = {2}; // Thicker borders for visibility
    return theme;
  }

private:
  /**
   * @brief Utility function to adjust color brightness
   */
  bombfork::prong::theming::Color adjustColorBrightness(const bombfork::prong::theming::Color& color,
                                                        float adjustment) const {
    return {std::min(1.0f, color.r + adjustment), std::min(1.0f, color.g + adjustment),
            std::min(1.0f, color.b + adjustment), color.a};
  }
};

/**
 * @brief Theme change event for component notification system
 */
struct ThemeChangeEvent {
  const AdvancedTheme* oldTheme;
  const AdvancedTheme* newTheme;

  ThemeChangeEvent(const AdvancedTheme* oldTheme, const AdvancedTheme* newTheme)
    : oldTheme(oldTheme), newTheme(newTheme) {}
};

// Type definitions for theme event handling
using ThemeChangeCallback = std::function<void(const ThemeChangeEvent&)>;

} // namespace bombfork::prong::theming