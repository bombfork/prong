#include "bombfork/prong/theming/advanced_theme.h"
#include "bombfork/prong/theming/color.h"

#include <bombfork/prong/theming/theme_manager.h>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bombfork::prong::theming {

// Static member initialization
std::unique_ptr<ThemeManager> ThemeManager::instance = nullptr;
std::mutex ThemeManager::instanceMutex;

ThemeManager::ThemeManager() {
  initialize();
}

ThemeManager& ThemeManager::getInstance() {
  std::lock_guard<std::mutex> lock(instanceMutex);
  if (!instance) {
    instance = std::unique_ptr<ThemeManager>(new ThemeManager());
  }
  return *instance;
}

ThemeManager::~ThemeManager() {
  if (autoSavePreferences) {
    savePreferences();
  }
}

void ThemeManager::initialize() {
  // Register built-in themes
  registerBuiltinThemes();

  // Load user preferences
  loadPreferences();

  // Ensure we have a valid current theme
  if (!hasTheme(currentThemeId)) {
    currentThemeId = "light"; // Fallback to light theme
  }
}

// === Theme Registration ===

bool ThemeManager::registerTheme(std::unique_ptr<AdvancedTheme> theme) {
  if (!theme) {
    return false;
  }

  std::string validation = validateTheme(*theme);
  if (!validation.empty()) {
    std::cerr << "Theme validation failed: " << validation << std::endl;
    return false;
  }

  std::lock_guard<std::mutex> lock(themesMutex);
  std::string themeId = theme->name;

  if (themes.find(themeId) != themes.end()) {
    std::cerr << "Theme already registered: " << themeId << std::endl;
    return false;
  }

  themes[themeId] = std::move(theme);
  return true;
}

void ThemeManager::registerBuiltinThemes() {
  registerTheme(std::make_unique<AdvancedTheme>(AdvancedTheme::Light()));
  registerTheme(std::make_unique<AdvancedTheme>(AdvancedTheme::Dark()));
  registerTheme(std::make_unique<AdvancedTheme>(AdvancedTheme::HighContrast()));
}

bool ThemeManager::unregisterTheme(const std::string& themeId) {
  std::lock_guard<std::mutex> lock(themesMutex);

  // Don't allow unregistering the current theme
  if (themeId == currentThemeId) {
    return false;
  }

  auto it = themes.find(themeId);
  if (it != themes.end()) {
    themes.erase(it);
    return true;
  }
  return false;
}

bool ThemeManager::hasTheme(const std::string& themeId) const {
  std::lock_guard<std::mutex> lock(themesMutex);
  return themes.find(themeId) != themes.end();
}

std::vector<std::string> ThemeManager::getAvailableThemeIds() const {
  std::lock_guard<std::mutex> lock(themesMutex);
  std::vector<std::string> ids;
  ids.reserve(themes.size());

  for (const auto& [themeId, theme] : themes) {
    ids.push_back(themeId);
  }

  std::sort(ids.begin(), ids.end());
  return ids;
}

std::vector<std::pair<std::string, std::string>> ThemeManager::getAvailableThemes() const {
  std::lock_guard<std::mutex> lock(themesMutex);
  std::vector<std::pair<std::string, std::string>> themes_list;
  themes_list.reserve(themes.size());

  for (const auto& [themeId, theme] : themes) {
    themes_list.emplace_back(themeId, theme->displayName);
  }

  std::sort(themes_list.begin(), themes_list.end());
  return themes_list;
}

// === Theme Access ===

const AdvancedTheme& ThemeManager::getCurrentTheme() const {
  std::lock_guard<std::mutex> lock(themesMutex);

  auto it = themes.find(currentThemeId);
  if (it != themes.end()) {
    return *it->second;
  }

  // Fallback to default theme if current theme not found
  return getDefaultTheme();
}

const AdvancedTheme* ThemeManager::getTheme(const std::string& themeId) const {
  std::lock_guard<std::mutex> lock(themesMutex);

  auto it = themes.find(themeId);
  if (it != themes.end()) {
    return it->second.get();
  }

  return nullptr;
}

const AdvancedTheme& ThemeManager::getDefaultTheme() const {
  // Return light theme as default fallback
  static AdvancedTheme fallback = AdvancedTheme::Light();

  auto it = themes.find("light");
  if (it != themes.end()) {
    return *it->second;
  }

  return fallback;
}

// === Theme Switching ===

bool ThemeManager::setCurrentTheme(const std::string& themeId) {
  const AdvancedTheme* oldTheme = nullptr;
  const AdvancedTheme* newTheme = nullptr;

  {
    std::lock_guard<std::mutex> lock(themesMutex);

    if (currentThemeId == themeId) {
      return true; // Already current theme
    }

    auto it = themes.find(themeId);
    if (it == themes.end()) {
      return false; // Theme not found
    }

    // Get theme references for event notification
    oldTheme = &getCurrentTheme();
    newTheme = it->second.get();

    // Update current theme
    currentThemeId = themeId;
  }

  // Notify callbacks outside of lock
  notifyThemeChange(oldTheme, newTheme);

  // Auto-save preferences if enabled
  if (autoSavePreferences) {
    savePreferences();
  }

  return true;
}

std::string ThemeManager::cycleToNextTheme() {
  auto availableIds = getAvailableThemeIds();
  if (availableIds.empty()) {
    return currentThemeId;
  }

  auto it = std::find(availableIds.begin(), availableIds.end(), currentThemeId);
  if (it != availableIds.end()) {
    ++it;
    if (it == availableIds.end()) {
      it = availableIds.begin(); // Wrap to first theme
    }
  } else {
    it = availableIds.begin(); // Current theme not found, start from first
  }

  setCurrentTheme(*it);
  return *it;
}

std::string ThemeManager::cycleToPreviousTheme() {
  auto availableIds = getAvailableThemeIds();
  if (availableIds.empty()) {
    return currentThemeId;
  }

  auto it = std::find(availableIds.begin(), availableIds.end(), currentThemeId);
  if (it != availableIds.end()) {
    if (it == availableIds.begin()) {
      it = availableIds.end() - 1; // Wrap to last theme
    } else {
      --it;
    }
  } else {
    it = availableIds.begin(); // Current theme not found, start from first
  }

  setCurrentTheme(*it);
  return *it;
}

// === Event System ===

size_t ThemeManager::onThemeChange(const ThemeChangeCallback& callback) {
  std::lock_guard<std::mutex> lock(callbacksMutex);

  size_t id = generateCallbackId();
  callbackMap[id] = callback;

  return id;
}

void ThemeManager::removeThemeChangeCallback(size_t callbackId) {
  std::lock_guard<std::mutex> lock(callbacksMutex);
  callbackMap.erase(callbackId);
}

void ThemeManager::clearThemeChangeCallbacks() {
  std::lock_guard<std::mutex> lock(callbacksMutex);
  callbackMap.clear();
}

void ThemeManager::notifyThemeChange(const AdvancedTheme* oldTheme, const AdvancedTheme* newTheme) {
  if (!oldTheme || !newTheme) {
    return;
  }

  ThemeChangeEvent event(oldTheme, newTheme);

  std::lock_guard<std::mutex> lock(callbacksMutex);
  for (const auto& [id, callback] : callbackMap) {
    try {
      callback(event);
    } catch (const std::exception& e) {
      std::cerr << "Error in theme change callback: " << e.what() << std::endl;
    }
  }
}

size_t ThemeManager::generateCallbackId() {
  return nextCallbackId++;
}

// === Preferences Management ===

void ThemeManager::setPreferencesFile(const std::string& filePath) {
  preferencesFile = filePath;
}

void ThemeManager::setAutoSavePreferences(bool enabled) {
  autoSavePreferences = enabled;
}

bool ThemeManager::savePreferences() const {
  try {
    std::ofstream file(preferencesFile);
    if (!file.is_open()) {
      return false;
    }

    // Simple JSON-like format for preferences
    file << "{\n";
    file << "  \"currentTheme\": \"" << currentThemeId << "\",\n";
    file << "  \"autoSave\": " << (autoSavePreferences ? "true" : "false") << "\n";
    file << "}\n";

    return true;
  } catch (const std::exception& e) {
    std::cerr << "Error saving theme preferences: " << e.what() << std::endl;
    return false;
  }
}

bool ThemeManager::loadPreferences() {
  try {
    std::ifstream file(preferencesFile);
    if (!file.is_open()) {
      return false; // File doesn't exist, use defaults
    }

    std::string line;
    while (std::getline(file, line)) {
      // Simple parsing for JSON-like format
      if (line.find("\"currentTheme\":") != std::string::npos) {
        size_t start = line.find('"', line.find(':')) + 1;
        size_t end = line.find('"', start);
        if (start != std::string::npos && end != std::string::npos) {
          currentThemeId = line.substr(start, end - start);
        }
      } else if (line.find("\"autoSave\":") != std::string::npos) {
        autoSavePreferences = line.find("true") != std::string::npos;
      }
    }

    return true;
  } catch (const std::exception& e) {
    std::cerr << "Error loading theme preferences: " << e.what() << std::endl;
    return false;
  }
}

// === Utility Methods ===

ThemeManager::ThemeStats ThemeManager::getStatistics() const {
  std::lock_guard<std::mutex> themeLock(themesMutex);
  std::lock_guard<std::mutex> callbackLock(callbacksMutex);

  ThemeStats stats;
  stats.totalThemes = themes.size();
  stats.currentTheme = currentThemeId;
  stats.registeredCallbacks = callbackMap.size();

  // Count builtin themes
  const std::vector<std::string> builtins = {"light", "dark", "high_contrast"};
  for (const std::string& builtin : builtins) {
    if (themes.find(builtin) != themes.end()) {
      stats.builtinThemes++;
    }
  }

  stats.customThemes = stats.totalThemes - stats.builtinThemes;

  return stats;
}

std::string ThemeManager::validateTheme(const AdvancedTheme& theme) {
  if (theme.name.empty()) {
    return "Theme name cannot be empty";
  }

  if (theme.displayName.empty()) {
    return "Theme display name cannot be empty";
  }

  // Validate colors (basic check for alpha values)
  if (theme.colors.primary.a < 0.0f || theme.colors.primary.a > 1.0f) {
    return "Invalid alpha value in primary color";
  }

  // Add more validation as needed
  return ""; // Valid theme
}

// === ThemeChangeListener Implementation ===

ThemeChangeListener::ThemeChangeListener(const ThemeChangeCallback& callback) {
  callbackId = ThemeManager::getInstance().onThemeChange(callback);
  active = true;
}

ThemeChangeListener::~ThemeChangeListener() {
  unregister();
}

ThemeChangeListener::ThemeChangeListener(ThemeChangeListener&& other) noexcept
  : callbackId(other.callbackId), active(other.active) {
  other.active = false; // Prevent double unregistration
}

ThemeChangeListener& ThemeChangeListener::operator=(ThemeChangeListener&& other) noexcept {
  if (this != &other) {
    unregister(); // Unregister current callback
    callbackId = other.callbackId;
    active = other.active;
    other.active = false;
  }
  return *this;
}

void ThemeChangeListener::unregister() {
  if (active) {
    ThemeManager::getInstance().removeThemeChangeCallback(callbackId);
    active = false;
  }
}

} // namespace bombfork::prong::theming