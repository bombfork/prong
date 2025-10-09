#pragma once

#include "../generic/button.h"
#include "../layout/layout_measurement.h"
#include "../layout/stack_layout.h"
#include "../theming/advanced_theme.h"
#include <bombfork/prong/core/component.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace bombfork::prong {

/**
 * @brief Professional context menu component for right-click functionality
 *
 * Provides comprehensive context menu functionality with:
 * - Hierarchical menu structure (submenus)
 * - Multiple item types (action, separator, submenu, checkbox, radio)
 * - Keyboard navigation with mnemonics and shortcuts
 * - Smart positioning to avoid screen edges
 * - Professional theming integration
 * - Icon support for menu items
 * - Disabled/enabled state management
 * - Click-outside-to-close behavior
 * - Fade in/out animations
 * - Global context menu manager
 *
 * Note: Planned for future implementation. Not currently used in the codebase.
 */
class ContextMenu : public Component {
public:
  enum class MenuItemType {
    ACTION,    // Standard clickable menu item
    SEPARATOR, // Visual separator line
    SUBMENU,   // Menu item that opens a submenu
    CHECKBOX,  // Checkable menu item (independent)
    RADIO      // Radio menu item (mutually exclusive group)
  };

  struct MenuItem {
    int id = -1;
    MenuItemType type = MenuItemType::ACTION;
    std::string text;
    std::string shortcut;
    std::string iconPath;
    std::string mnemonic; // Alt+key shortcut
    bool enabled = true;
    bool checked = false;
    bool visible = true;
    int radioGroup = -1; // For radio items (same group = mutually exclusive)

    // For submenus
    std::unique_ptr<ContextMenu> submenu;

    // UI representation
    std::unique_ptr<Button> button;
  };

  using MenuActionCallback = std::function<void(int itemId)>;
  using MenuStateCallback = std::function<void(int itemId, bool checked)>;

private:
  static constexpr int DEFAULT_ITEM_HEIGHT = 24;
  static constexpr int SEPARATOR_HEIGHT = 8;
  static constexpr int DEFAULT_MIN_WIDTH = 120;
  static constexpr int ICON_SIZE = 16;
  static constexpr int ITEM_PADDING = 8;
  static constexpr int SUBMENU_ARROW_SIZE = 8;
  static constexpr int FADE_DURATION = 150; // milliseconds

  struct ContextMenuTheme {
    // Background and border
    bombfork::prong::theming::Color backgroundColor;
    bombfork::prong::theming::Color borderColor;
    bombfork::prong::theming::Color shadowColor;
    bombfork::prong::theming::Color separatorColor;

    // Item states
    bombfork::prong::theming::Color itemNormalColor;
    bombfork::prong::theming::Color itemHoverColor;
    bombfork::prong::theming::Color itemSelectedColor;
    bombfork::prong::theming::Color itemDisabledColor;

    // Text colors
    bombfork::prong::theming::Color textColor;
    bombfork::prong::theming::Color disabledTextColor;
    bombfork::prong::theming::Color shortcutTextColor;

    // Visual properties
    float borderWidth = 1.0f;
    float cornerRadius = 4.0f;
    bool showShadow = true;
    float shadowOpacity = 0.4f;
    int shadowOffset = 3;

    ContextMenuTheme() {
      // Professional desktop theme defaults
      backgroundColor = bombfork::prong::theming::Color(0.18f, 0.18f, 0.18f, 0.98f);
      borderColor = bombfork::prong::theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
      shadowColor = bombfork::prong::theming::Color(0.0f, 0.0f, 0.0f, 0.6f);
      separatorColor = bombfork::prong::theming::Color(0.3f, 0.3f, 0.3f, 1.0f);

      itemNormalColor = bombfork::prong::theming::Color(0.0f, 0.0f, 0.0f, 0.0f);
      itemHoverColor = bombfork::prong::theming::Color(0.3f, 0.5f, 0.8f, 0.4f);
      itemSelectedColor = bombfork::prong::theming::Color(0.2f, 0.4f, 0.7f, 0.6f);
      itemDisabledColor = bombfork::prong::theming::Color(0.1f, 0.1f, 0.1f, 0.2f);

      textColor = bombfork::prong::theming::Color(0.95f, 0.95f, 0.95f, 1.0f);
      disabledTextColor = bombfork::prong::theming::Color(0.5f, 0.5f, 0.5f, 1.0f);
      shortcutTextColor = bombfork::prong::theming::Color(0.7f, 0.7f, 0.7f, 1.0f);
    }
  };

  struct MenuState {
    bool visible = false;
    bool animating = false;
    float opacity = 0.0f;
    std::chrono::steady_clock::time_point animationStartTime;
    int selectedIndex = -1;
    bool keyboardNavigation = false;

    // Positioning
    int posX = 0, posY = 0;
    int calculatedWidth = 0, calculatedHeight = 0;

    // Parent menu reference (for submenus)
    ContextMenu* parentMenu = nullptr;
    int parentItemIndex = -1;
  };

  ContextMenuTheme theme;
  MenuState state;

  // Menu items
  std::vector<std::unique_ptr<MenuItem>> items;
  std::unordered_map<int, size_t> itemIndexMap; // ID to index mapping
  int nextItemId = 1;

  // Layout
  std::unique_ptr<bombfork::prong::layout::StackLayout> layout;
  int minWidth = DEFAULT_MIN_WIDTH;
  int itemHeight = DEFAULT_ITEM_HEIGHT;

  // Callbacks
  MenuActionCallback actionCallback;
  MenuStateCallback stateCallback;

  // Global menu manager
  static ContextMenu* activeMenu;

public:
  explicit ContextMenu(bombfork::prong::rendering::IRenderer* renderer = nullptr);
  ~ContextMenu() override;

  // === Menu Construction ===

  /**
   * @brief Add action menu item
   */
  int addAction(const std::string& text, const std::string& shortcut = "", const std::string& iconPath = "",
                const std::string& mnemonic = "");

  /**
   * @brief Add submenu item
   */
  int addSubmenu(const std::string& text, std::unique_ptr<ContextMenu> submenu, const std::string& iconPath = "",
                 const std::string& mnemonic = "");

  /**
   * @brief Add checkbox menu item
   */
  int addCheckbox(const std::string& text, bool initiallyChecked = false, const std::string& shortcut = "",
                  const std::string& iconPath = "", const std::string& mnemonic = "");

  /**
   * @brief Add radio button menu item
   */
  int addRadio(const std::string& text, int radioGroup, bool initiallyChecked = false, const std::string& shortcut = "",
               const std::string& iconPath = "", const std::string& mnemonic = "");

  /**
   * @brief Add separator
   */
  void addSeparator();

  /**
   * @brief Remove menu item
   */
  void removeItem(int itemId);

  /**
   * @brief Clear all menu items
   */
  void clear();

  // === Item Management ===

  /**
   * @brief Set item enabled state
   */
  void setItemEnabled(int itemId, bool enabled);

  /**
   * @brief Check if item is enabled
   */
  bool isItemEnabled(int itemId) const;

  /**
   * @brief Set item checked state
   */
  void setItemChecked(int itemId, bool checked);

  /**
   * @brief Check if item is checked
   */
  bool isItemChecked(int itemId) const;

  /**
   * @brief Set item visible state
   */
  void setItemVisible(int itemId, bool visible);

  /**
   * @brief Check if item is visible
   */
  bool isItemVisible(int itemId) const;

  /**
   * @brief Set item text
   */
  void setItemText(int itemId, const std::string& text);

  /**
   * @brief Get item text
   */
  std::string getItemText(int itemId) const;

  // === Display Management ===

  /**
   * @brief Show context menu at position
   */
  void showAt(int x, int y);

  /**
   * @brief Show context menu for component (at component position)
   */
  void showFor(Component* component, int offsetX = 0, int offsetY = 0);

  /**
   * @brief Hide context menu
   */
  void hide();

  /**
   * @brief Check if menu is visible
   */
  bool isVisible() const { return state.visible; }

  /**
   * @brief Update menu animation and state
   */
  void update();

  // === Keyboard Navigation ===

  /**
   * @brief Move selection up
   */
  void selectPrevious();

  /**
   * @brief Move selection down
   */
  void selectNext();

  /**
   * @brief Activate selected item
   */
  void activateSelected();

  /**
   * @brief Process mnemonic key
   */
  bool processMnemonic(char key);

  // === Configuration ===

  /**
   * @brief Set minimum menu width
   */
  void setMinWidth(int width);

  /**
   * @brief Get minimum menu width
   */
  int getMinWidth() const { return minWidth; }

  /**
   * @brief Set item height
   */
  void setItemHeight(int height);

  /**
   * @brief Get item height
   */
  int getItemHeight() const { return itemHeight; }

  // === Callbacks ===

  /**
   * @brief Set action callback (called when item is clicked)
   */
  void setActionCallback(MenuActionCallback callback);

  /**
   * @brief Set state callback (called when checkbox/radio state changes)
   */
  void setStateCallback(MenuStateCallback callback);

  // === Global Menu Management ===

  /**
   * @brief Close all open context menus
   */
  static void closeAll();

  /**
   * @brief Get currently active menu
   */
  static ContextMenu* getActiveMenu() { return activeMenu; }

  // === Theming ===

  /**
   * @brief Apply theme from AdvancedTheme system
   */
  void applyTheme(const bombfork::prong::theming::AdvancedTheme& theme);

  /**
   * @brief Set custom theme
   */
  void setContextMenuTheme(const ContextMenuTheme& customTheme);

  /**
   * @brief Get current theme
   */
  const ContextMenuTheme& getContextMenuTheme() const { return theme; }

  // === UIComponent Overrides ===

  void render() override;
  bool handleClick(int localX, int localY) override;
  bool handleMouseMove(int localX, int localY) override;
  bool handleKey(int key, int action, int mods) override;
  void setFocus(bool focused) override;
  void setBounds(int x, int y, int width, int height) override;

  // === Layout Integration ===

  bombfork::prong::layout::LayoutMeasurement measurePreferredSize() const override;

private:
  /**
   * @brief Initialize menu layout
   */
  void initializeLayout();

  /**
   * @brief Calculate menu size
   */
  void calculateSize();

  /**
   * @brief Calculate optimal position to avoid screen edges
   */
  void calculatePosition(int requestedX, int requestedY);

  /**
   * @brief Update layout after changes
   */
  void updateLayout();

  /**
   * @brief Create button for menu item
   */
  std::unique_ptr<Button> createItemButton(MenuItem* item);

  /**
   * @brief Update item button appearance
   */
  void updateItemButton(MenuItem* item, int index);

  /**
   * @brief Handle item click
   */
  void handleItemClick(int itemId);

  /**
   * @brief Update animation
   */
  void updateAnimation();

  /**
   * @brief Get item at index
   */
  MenuItem* getItem(int itemId);

  /**
   * @brief Get item at index (const version)
   */
  const MenuItem* getItem(int itemId) const;

  /**
   * @brief Find next selectable item
   */
  int findNextSelectableItem(int startIndex, bool forward = true) const;

  /**
   * @brief Set selected item
   */
  void setSelectedItem(int index);

  /**
   * @brief Check radio button group
   */
  void checkRadioGroup(int itemId, int radioGroup);

  /**
   * @brief Render menu background
   */
  void renderBackground();

  /**
   * @brief Render menu shadow
   */
  void renderShadow();

  /**
   * @brief Render menu border
   */
  void renderBorder();

  /**
   * @brief Render menu items
   */
  void renderItems();

  /**
   * @brief Render submenu arrows
   */
  void renderSubmenuArrows();

  /**
   * @brief Get screen dimensions
   */
  void getScreenDimensions(int& width, int& height) const;

  /**
   * @brief Handle submenu display
   */
  void showSubmenu(int itemIndex);

  /**
   * @brief Hide submenu
   */
  void hideSubmenu();

  /**
   * @brief Get current time for animations
   */
  std::chrono::steady_clock::time_point getCurrentTime() const;

  /**
   * @brief Calculate fade opacity
   */
  float calculateOpacity() const;

  /**
   * @brief Set as active menu
   */
  void setAsActive();

  /**
   * @brief Remove from active state
   */
  void removeFromActive();

  /**
   * @brief Get next available item ID
   */
  int getNextItemId() { return nextItemId++; }

  /**
   * @brief Rebuild item index map
   */
  void rebuildIndexMap();
};

// === Utility Functions for Easy Context Menu Integration ===

/**
 * @brief Add context menu to any Component
 */
void addContextMenu(Component* component, std::unique_ptr<ContextMenu> menu);

/**
 * @brief Remove context menu from Component
 */
void removeContextMenu(Component* component);

/**
 * @brief Create standard edit context menu (Cut, Copy, Paste, etc.)
 */
std::unique_ptr<ContextMenu> createEditMenu();

/**
 * @brief Create standard file context menu (Open, Save, etc.)
 */
std::unique_ptr<ContextMenu> createFileMenu();

} // namespace bombfork::prong