#pragma once

#include "../layout/flow_layout.h"
#include "../layout/layout_measurement.h"
#include "../theming/advanced_theme.h"
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/theming/color.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace bombfork::prong {

/**
 * @brief Professional toolbar component for desktop applications
 *
 * Provides comprehensive toolbar functionality with:
 * - Horizontal and vertical orientations
 * - Multiple button types (push, toggle, dropdown)
 * - Separators and spacers for logical grouping
 * - Icon support with configurable sizes
 * - Overflow handling with dropdown menu
 * - Keyboard navigation and shortcuts
 * - Professional theming integration
 * - Tooltip integration for all tools
 * - Customizable button sizes and spacing
 * - Tool state management (enabled/disabled, checked/unchecked)
 */
class ToolBar : public bombfork::prong::Component {
public:
  enum class Orientation {
    HORIZONTAL, // Tools arranged left to right
    VERTICAL    // Tools arranged top to bottom
  };

  enum class ToolType {
    PUSH_BUTTON,     // Standard clickable button
    TOGGLE_BUTTON,   // Button with checked/unchecked state
    DROPDOWN_BUTTON, // Button with dropdown menu
    SEPARATOR,       // Visual separator line
    SPACER,          // Flexible space
    FIXED_SPACER,    // Fixed-size space
    TEXT_LABEL       // Text label
  };

  enum class ToolSize {
    SMALL = 16,  // 16x16 icons
    MEDIUM = 24, // 24x24 icons
    LARGE = 32   // 32x32 icons
  };

  struct ToolItem {
    int id = -1;
    ToolType type = ToolType::PUSH_BUTTON;
    std::string text;
    std::string tooltip;
    std::string iconPath;
    std::string shortcut;
    bool enabled = true;
    bool checked = false;
    bool visible = true;
    std::unique_ptr<bombfork::prong::Button> button;

    // For separators and spacers
    int separatorSize = 1; // Width for vertical separator, height for horizontal
    int spacerSize = 0;    // 0 = flexible, >0 = fixed size
  };

  using ToolCallback = std::function<void(int toolId)>;
  using ToolStateCallback = std::function<void(int toolId, bool checked)>;

private:
  static constexpr int DEFAULT_TOOL_SIZE = 24;
  static constexpr int DEFAULT_SPACING = 2;
  static constexpr int DEFAULT_PADDING = 4;
  static constexpr int SEPARATOR_SIZE = 1;
  static constexpr int MIN_OVERFLOW_WIDTH = 32;

  struct ToolBarState {
    Orientation orientation = Orientation::HORIZONTAL;
    ToolSize toolSize = ToolSize::MEDIUM;
    int toolSpacing = DEFAULT_SPACING;
    int toolPadding = DEFAULT_PADDING;
    bool showText = false;     // Show text labels
    bool showTooltips = true;  // Enable tooltips
    bool allowOverflow = true; // Show overflow menu when needed
    int overflowThreshold = 0; // Width/height before overflow kicks in
    bool wrapTools = false;    // Wrap to new line/column
  };

  struct ToolBarTheme {
    // Background and borders
    bombfork::prong::theming::Color backgroundColor;
    bombfork::prong::theming::Color borderColor;
    bombfork::prong::theming::Color separatorColor;

    // Tool button states
    bombfork::prong::theming::Color toolNormalColor;
    bombfork::prong::theming::Color toolHoverColor;
    bombfork::prong::theming::Color toolPressedColor;
    bombfork::prong::theming::Color toolCheckedColor;
    bombfork::prong::theming::Color toolDisabledColor;

    // Text colors
    bombfork::prong::theming::Color textColor;
    bombfork::prong::theming::Color disabledTextColor;

    // Visual properties
    float borderWidth = 1.0f;
    float cornerRadius = 4.0f;
    bool showBorder = true;
    bool showBackground = true;

    ToolBarTheme() {
      // Professional desktop theme defaults
      backgroundColor = bombfork::prong::theming::Color(0.22f, 0.22f, 0.22f, 1.0f);
      borderColor = bombfork::prong::theming::Color(0.3f, 0.3f, 0.3f, 1.0f);
      separatorColor = bombfork::prong::theming::Color(0.4f, 0.4f, 0.4f, 1.0f);

      toolNormalColor = bombfork::prong::theming::Color(0.0f, 0.0f, 0.0f, 0.0f);
      toolHoverColor = bombfork::prong::theming::Color(0.3f, 0.3f, 0.3f, 0.3f);
      toolPressedColor = bombfork::prong::theming::Color(0.2f, 0.2f, 0.2f, 0.5f);
      toolCheckedColor = bombfork::prong::theming::Color(0.2f, 0.4f, 0.7f, 0.4f);
      toolDisabledColor = bombfork::prong::theming::Color(0.1f, 0.1f, 0.1f, 0.2f);

      textColor = bombfork::prong::theming::Color(0.9f, 0.9f, 0.9f, 1.0f);
      disabledTextColor = bombfork::prong::theming::Color(0.5f, 0.5f, 0.5f, 1.0f);
    }
  };

  ToolBarState state;
  ToolBarTheme theme;

  // Tool management
  std::vector<std::unique_ptr<ToolItem>> tools;
  std::unordered_map<int, size_t> toolIndexMap; // ID to index mapping
  int nextToolId = 1;

  // Layout
  std::unique_ptr<bombfork::prong::layout::FlowLayout> layout;

  // Overflow handling
  std::unique_ptr<bombfork::prong::Button> overflowButton;
  std::vector<int> overflowTools; // Tools that don't fit

  // Callbacks
  ToolCallback toolCallback;
  ToolStateCallback toolStateCallback;

public:
  explicit ToolBar(bombfork::prong::rendering::IRenderer* renderer = nullptr);
  ~ToolBar() override = default;

  // === Configuration ===

  /**
   * @brief Set toolbar orientation
   */
  void setOrientation(Orientation orientation);

  /**
   * @brief Get toolbar orientation
   */
  Orientation getOrientation() const { return state.orientation; }

  /**
   * @brief Set tool size
   */
  void setToolSize(ToolSize size);

  /**
   * @brief Get tool size
   */
  ToolSize getToolSize() const { return state.toolSize; }

  /**
   * @brief Set tool spacing
   */
  void setToolSpacing(int spacing);

  /**
   * @brief Get tool spacing
   */
  int getToolSpacing() const { return state.toolSpacing; }

  /**
   * @brief Show/hide text labels
   */
  void setShowText(bool show);

  /**
   * @brief Check if text labels are shown
   */
  bool getShowText() const { return state.showText; }

  /**
   * @brief Enable/disable tooltips
   */
  void setShowTooltips(bool show);

  /**
   * @brief Check if tooltips are enabled
   */
  bool getShowTooltips() const { return state.showTooltips; }

  // === Tool Management ===

  /**
   * @brief Add push button tool
   */
  int addTool(const std::string& text, const std::string& iconPath = "", const std::string& tooltip = "",
              const std::string& shortcut = "");

  /**
   * @brief Add toggle button tool
   */
  int addToggleTool(const std::string& text, const std::string& iconPath = "", const std::string& tooltip = "",
                    bool initiallyChecked = false, const std::string& shortcut = "");

  /**
   * @brief Add dropdown button tool
   */
  int addDropdownTool(const std::string& text, const std::string& iconPath = "", const std::string& tooltip = "",
                      const std::string& shortcut = "");

  /**
   * @brief Add separator
   */
  void addSeparator(int size = SEPARATOR_SIZE);

  /**
   * @brief Add spacer (flexible or fixed size)
   */
  void addSpacer(int size = 0); // 0 = flexible, >0 = fixed

  /**
   * @brief Add text label
   */
  int addLabel(const std::string& text);

  /**
   * @brief Remove tool by ID
   */
  void removeTool(int toolId);

  /**
   * @brief Remove all tools
   */
  void clearTools();

  /**
   * @brief Get tool by ID
   */
  ToolItem* getTool(int toolId);

  /**
   * @brief Get tool by ID (const version)
   */
  const ToolItem* getTool(int toolId) const;

  // === Tool State Management ===

  /**
   * @brief Set tool enabled state
   */
  void setToolEnabled(int toolId, bool enabled);

  /**
   * @brief Check if tool is enabled
   */
  bool isToolEnabled(int toolId) const;

  /**
   * @brief Set tool checked state (for toggle tools)
   */
  void setToolChecked(int toolId, bool checked);

  /**
   * @brief Check if tool is checked
   */
  bool isToolChecked(int toolId) const;

  /**
   * @brief Set tool visible state
   */
  void setToolVisible(int toolId, bool visible);

  /**
   * @brief Check if tool is visible
   */
  bool isToolVisible(int toolId) const;

  /**
   * @brief Set tool text
   */
  void setToolText(int toolId, const std::string& text);

  /**
   * @brief Get tool text
   */
  std::string getToolText(int toolId) const;

  /**
   * @brief Set tool tooltip
   */
  void setToolTooltip(int toolId, const std::string& tooltip);

  /**
   * @brief Get tool tooltip
   */
  std::string getToolTooltip(int toolId) const;

  // === Callbacks ===

  /**
   * @brief Set tool callback (called when tool is clicked)
   */
  void setToolCallback(ToolCallback callback);

  /**
   * @brief Set tool state callback (called when toggle state changes)
   */
  void setToolStateCallback(ToolStateCallback callback);

  // === Theming ===

  /**
   * @brief Apply theme from AdvancedTheme system
   */
  void applyTheme(const bombfork::prong::theming::AdvancedTheme& theme);

  /**
   * @brief Set custom theme
   */
  void setToolBarTheme(const ToolBarTheme& customTheme);

  /**
   * @brief Get current theme
   */
  const ToolBarTheme& getToolBarTheme() const { return theme; }

  // === UIComponent Overrides ===

  void update(double deltaTime) override {
    (void)deltaTime; // Unused - state changes are event-driven
  }

  void render() override;
  bool handleClick(int localX, int localY) override;
  bool handleMousePress(int localX, int localY, int button) override;
  bool handleMouseRelease(int localX, int localY, int button) override;
  bool handleMouseMove(int localX, int localY) override;
  bool handleKey(int key, int action, int mods) override;
  void setBounds(int x, int y, int width, int height) override;

  // === Layout Integration ===

  bombfork::prong::layout::LayoutMeasurement measurePreferredSize() const;

private:
  /**
   * @brief Initialize toolbar layout
   */
  void initializeLayout();

  /**
   * @brief Update layout after changes
   */
  void updateLayout();

  /**
   * @brief Create button for tool item
   */
  std::unique_ptr<bombfork::prong::Button> createToolButton(ToolItem* tool);

  /**
   * @brief Update tool button appearance
   */
  void updateToolButton(ToolItem* tool);

  /**
   * @brief Handle tool button click
   */
  void handleToolClick(int toolId);

  /**
   * @brief Calculate tool dimensions
   */
  void getToolDimensions(int& toolWidth, int& toolHeight) const;

  /**
   * @brief Check if tools need overflow handling
   */
  bool needsOverflow() const;

  /**
   * @brief Update overflow state
   */
  void updateOverflow();

  /**
   * @brief Render toolbar background
   */
  void renderBackground();

  /**
   * @brief Render tool separators
   */
  void renderSeparators();

  /**
   * @brief Process keyboard shortcuts
   */
  bool processShortcut(const std::string& shortcut);

  /**
   * @brief Find tool by shortcut
   */
  int findToolByShortcut(const std::string& shortcut) const;

  /**
   * @brief Get next available tool ID
   */
  int getNextToolId() { return nextToolId++; }

  /**
   * @brief Rebuild tool index map
   */
  void rebuildIndexMap();

  /**
   * @brief Calculate required size for all tools
   */
  int calculateRequiredSize() const;
};

} // namespace bombfork::prong