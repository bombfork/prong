#pragma once

#include <bombfork/prong/theming/advanced_theme.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bombfork::prong::theming {

/**
 * @brief Singleton theme manager for application-wide theme management
 *
 * Provides centralized theme management with the following features:
 * - Theme registration and lookup
 * - Runtime theme switching
 * - Theme change event notifications
 * - User preference persistence
 * - Thread-safe access
 */
class ThemeManager {
public:
  using ThemeChangeCallback = std::function<void(const ThemeChangeEvent&)>;

private:
  // Singleton instance
  static std::unique_ptr<ThemeManager> instance;
  static std::mutex instanceMutex;

  // Theme storage and management
  std::unordered_map<std::string, std::unique_ptr<AdvancedTheme>> themes;
  std::string currentThemeId = "light";
  mutable std::mutex themesMutex;

  // Event system
  std::vector<ThemeChangeCallback> changeCallbacks;
  mutable std::mutex callbacksMutex;

  // User preferences
  std::string preferencesFile = "theme_preferences.json";
  bool autoSavePreferences = false;

  /**
   * @brief Private constructor for singleton pattern
   */
  ThemeManager();

public:
  // Disable copy constructor and assignment
  ThemeManager(const ThemeManager&) = delete;
  ThemeManager& operator=(const ThemeManager&) = delete;

  /**
   * @brief Get singleton instance
   */
  static ThemeManager& getInstance();

  /**
   * @brief Destructor - saves preferences if enabled
   */
  ~ThemeManager();

  // === Theme Registration ===

  /**
   * @brief Register a new theme
   * @param theme Unique pointer to theme (ownership transferred)
   * @return true if registration successful
   */
  bool registerTheme(std::unique_ptr<AdvancedTheme> theme);

  /**
   * @brief Register built-in themes (Light, Dark, High Contrast)
   */
  void registerBuiltinThemes();

  /**
   * @brief Unregister a theme by ID
   * @param themeId Theme identifier
   * @return true if theme was found and removed
   */
  bool unregisterTheme(const std::string& themeId);

  /**
   * @brief Check if theme is registered
   * @param themeId Theme identifier
   * @return true if theme exists
   */
  bool hasTheme(const std::string& themeId) const;

  /**
   * @brief Get list of all registered theme IDs
   */
  std::vector<std::string> getAvailableThemeIds() const;

  /**
   * @brief Get list of all registered themes with display names
   * @return vector of pairs (id, displayName)
   */
  std::vector<std::pair<std::string, std::string>> getAvailableThemes() const;

  // === Theme Access ===

  /**
   * @brief Get current theme
   * @return Reference to current theme (guaranteed to be valid)
   */
  const AdvancedTheme& getCurrentTheme() const;

  /**
   * @brief Get theme by ID
   * @param themeId Theme identifier
   * @return Pointer to theme, or nullptr if not found
   */
  const AdvancedTheme* getTheme(const std::string& themeId) const;

  /**
   * @brief Get current theme ID
   */
  const std::string& getCurrentThemeId() const { return currentThemeId; }

  // === Theme Switching ===

  /**
   * @brief Switch to a different theme
   * @param themeId Theme identifier
   * @return true if theme switch successful
   */
  bool setCurrentTheme(const std::string& themeId);

  /**
   * @brief Cycle to next available theme
   * @return ID of new current theme
   */
  std::string cycleToNextTheme();

  /**
   * @brief Cycle to previous available theme
   * @return ID of new current theme
   */
  std::string cycleToPreviousTheme();

  // === Event System ===

  /**
   * @brief Register callback for theme change events
   * @param callback Function to call when theme changes
   * @return Callback ID for unregistration
   */
  size_t onThemeChange(const ThemeChangeCallback& callback);

  /**
   * @brief Unregister theme change callback
   * @param callbackId ID returned by onThemeChange
   */
  void removeThemeChangeCallback(size_t callbackId);

  /**
   * @brief Clear all theme change callbacks
   */
  void clearThemeChangeCallbacks();

  // === Preferences Management ===

  /**
   * @brief Set preferences file path
   * @param filePath Path to preferences file (JSON format)
   */
  void setPreferencesFile(const std::string& filePath);

  /**
   * @brief Enable/disable automatic preference saving
   * @param enabled If true, preferences are saved on theme changes
   */
  void setAutoSavePreferences(bool enabled);

  /**
   * @brief Save current preferences to file
   * @return true if save successful
   */
  bool savePreferences() const;

  /**
   * @brief Load preferences from file
   * @return true if load successful
   */
  bool loadPreferences();

  // === Utility Methods ===

  /**
   * @brief Get theme statistics
   */
  struct ThemeStats {
    size_t totalThemes = 0;
    size_t builtinThemes = 0;
    size_t customThemes = 0;
    std::string currentTheme;
    size_t registeredCallbacks = 0;
  };

  ThemeStats getStatistics() const;

  /**
   * @brief Validate theme configuration
   * @param theme Theme to validate
   * @return Validation error message, empty if valid
   */
  static std::string validateTheme(const AdvancedTheme& theme);

  // === Legacy Compatibility ===

private:
  /**
   * @brief Initialize default themes and load preferences
   */
  void initialize();

  /**
   * @brief Notify all registered callbacks of theme change
   */
  void notifyThemeChange(const AdvancedTheme* oldTheme, const AdvancedTheme* newTheme);

  /**
   * @brief Get default fallback theme
   */
  const AdvancedTheme& getDefaultTheme() const;

  /**
   * @brief Generate next callback ID
   */
  size_t generateCallbackId();

  // Internal state for callback management
  size_t nextCallbackId = 1;
  std::unordered_map<size_t, ThemeChangeCallback> callbackMap;
};

/**
 * @brief RAII helper for theme change notifications
 *
 * Automatically unregisters theme change callback when destroyed.
 * Useful for component lifecycle management.
 */
class ThemeChangeListener {
private:
  size_t callbackId = 0;
  bool active = false;

public:
  /**
   * @brief Constructor - registers callback
   */
  explicit ThemeChangeListener(const ThemeChangeCallback& callback);

  /**
   * @brief Destructor - unregisters callback
   */
  ~ThemeChangeListener();

  // Disable copy, allow move
  ThemeChangeListener(const ThemeChangeListener&) = delete;
  ThemeChangeListener& operator=(const ThemeChangeListener&) = delete;
  ThemeChangeListener(ThemeChangeListener&& other) noexcept;
  ThemeChangeListener& operator=(ThemeChangeListener&& other) noexcept;

  /**
   * @brief Check if listener is active
   */
  bool isActive() const { return active; }

  /**
   * @brief Manually unregister (destructor will not unregister again)
   */
  void unregister();
};

// Convenience macros for theme access
#define THEME_MANAGER ThemeManager::getInstance()
#define CURRENT_THEME THEME_MANAGER.getCurrentTheme()
#define THEMED_BUTTON(variant) CURRENT_THEME.getButtonTraits(variant)
#define THEMED_PANEL(variant) CURRENT_THEME.getPanelTraits(variant)
#define THEMED_SCROLLABLE(variant) CURRENT_THEME.getScrollableTraits(variant)

} // namespace bombfork::prong::theming
