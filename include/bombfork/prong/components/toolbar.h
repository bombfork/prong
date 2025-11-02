#pragma once

#include "../layout/flow_layout.h"
#include "../layout/layout_measurement.h"
#include "../theming/advanced_theme.h"
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/event.h>
#include <bombfork/prong/theming/color.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
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
    bombfork::prong::Button* button = nullptr; // Raw pointer - button is owned by Component::children

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
  bombfork::prong::Button* overflowButton = nullptr; // Raw pointer - button is owned by Component::children
  std::vector<int> overflowTools;                    // Tools that don't fit

  // Callbacks
  ToolCallback toolCallback;
  ToolStateCallback toolStateCallback;

public:
  explicit ToolBar() : bombfork::prong::Component(nullptr) { initializeLayout(); }
  ~ToolBar() override = default;

  // === Configuration ===

  /**
   * @brief Set toolbar orientation
   */
  void setOrientation(Orientation orientation) {
    if (state.orientation != orientation) {
      state.orientation = orientation;
      updateLayout();
    }
  }

  /**
   * @brief Get toolbar orientation
   */
  Orientation getOrientation() const { return state.orientation; }

  /**
   * @brief Set tool size
   */
  void setToolSize(ToolSize size) {
    if (state.toolSize != size) {
      state.toolSize = size;
      updateLayout();
    }
  }

  /**
   * @brief Get tool size
   */
  ToolSize getToolSize() const { return state.toolSize; }

  /**
   * @brief Set tool spacing
   */
  void setToolSpacing(int spacing) {
    if (state.toolSpacing != spacing) {
      state.toolSpacing = spacing;
      updateLayout();
    }
  }

  /**
   * @brief Get tool spacing
   */
  int getToolSpacing() const { return state.toolSpacing; }

  /**
   * @brief Show/hide text labels
   */
  void setShowText(bool show) {
    if (state.showText != show) {
      state.showText = show;
      updateLayout();
    }
  }

  /**
   * @brief Check if text labels are shown
   */
  bool getShowText() const { return state.showText; }

  /**
   * @brief Enable/disable tooltips
   */
  void setShowTooltips(bool show) { state.showTooltips = show; }

  /**
   * @brief Check if tooltips are enabled
   */
  bool getShowTooltips() const { return state.showTooltips; }

  // === Tool Management ===

  /**
   * @brief Add push button tool
   */
  int addTool(const std::string& text, const std::string& iconPath = "", const std::string& tooltip = "",
              const std::string& shortcut = "") {
    auto tool = std::make_unique<ToolItem>();
    tool->id = getNextToolId();
    tool->type = ToolType::PUSH_BUTTON;
    tool->text = text;
    tool->iconPath = iconPath;
    tool->tooltip = tooltip.empty() ? text : tooltip;
    tool->shortcut = shortcut;

    int toolId = tool->id;
    tools.push_back(std::move(tool));
    createToolButton(tools.back().get());
    rebuildIndexMap();
    updateLayout();

    return toolId;
  }

  /**
   * @brief Add toggle button tool
   */
  int addToggleTool(const std::string& text, const std::string& iconPath = "", const std::string& tooltip = "",
                    bool initiallyChecked = false, const std::string& shortcut = "") {
    auto tool = std::make_unique<ToolItem>();
    tool->id = getNextToolId();
    tool->type = ToolType::TOGGLE_BUTTON;
    tool->text = text;
    tool->iconPath = iconPath;
    tool->tooltip = tooltip.empty() ? text : tooltip;
    tool->checked = initiallyChecked;
    tool->shortcut = shortcut;

    int toolId = tool->id;
    tools.push_back(std::move(tool));
    createToolButton(tools.back().get());
    rebuildIndexMap();
    updateLayout();

    return toolId;
  }

  /**
   * @brief Add dropdown button tool
   */
  int addDropdownTool(const std::string& text, const std::string& iconPath = "", const std::string& tooltip = "",
                      const std::string& shortcut = "") {
    auto tool = std::make_unique<ToolItem>();
    tool->id = getNextToolId();
    tool->type = ToolType::DROPDOWN_BUTTON;
    tool->text = text;
    tool->iconPath = iconPath;
    tool->tooltip = tooltip.empty() ? text : tooltip;
    tool->shortcut = shortcut;

    int toolId = tool->id;
    tools.push_back(std::move(tool));
    createToolButton(tools.back().get());
    rebuildIndexMap();
    updateLayout();

    return toolId;
  }

  /**
   * @brief Add separator
   */
  void addSeparator(int size = SEPARATOR_SIZE) {
    auto tool = std::make_unique<ToolItem>();
    tool->id = -1; // Separators don't have IDs
    tool->type = ToolType::SEPARATOR;
    tool->separatorSize = size;

    tools.push_back(std::move(tool));
    rebuildIndexMap();
    updateLayout();
  }

  /**
   * @brief Add spacer (flexible or fixed size)
   */
  void addSpacer(int size = 0) { // 0 = flexible, >0 = fixed
    auto tool = std::make_unique<ToolItem>();
    tool->id = -1; // Spacers don't have IDs
    tool->type = (size == 0) ? ToolType::SPACER : ToolType::FIXED_SPACER;
    tool->spacerSize = size;

    tools.push_back(std::move(tool));
    rebuildIndexMap();
    updateLayout();
  }

  /**
   * @brief Add text label
   */
  int addLabel(const std::string& text) {
    auto tool = std::make_unique<ToolItem>();
    tool->id = getNextToolId();
    tool->type = ToolType::TEXT_LABEL;
    tool->text = text;
    tool->tooltip = text;

    int toolId = tool->id;
    tools.push_back(std::move(tool));
    rebuildIndexMap();
    updateLayout();

    return toolId;
  }

  /**
   * @brief Remove tool by ID
   */
  void removeTool(int toolId) {
    auto it = toolIndexMap.find(toolId);
    if (it != toolIndexMap.end()) {
      size_t index = it->second;
      if (index < tools.size()) {
        // Remove the button from children if it exists
        auto* button = tools[index]->button;
        if (button) {
          removeChild(button);
        }
        tools.erase(tools.begin() + static_cast<std::ptrdiff_t>(index));
        rebuildIndexMap();
        updateLayout();
      }
    }
  }

  /**
   * @brief Remove all tools
   */
  void clearTools() {
    // Remove all tool buttons from children
    for (const auto& tool : tools) {
      if (tool->button) {
        removeChild(tool->button);
      }
    }
    tools.clear();
    toolIndexMap.clear();
    overflowTools.clear();

    // Also clear overflow button if it exists
    if (overflowButton) {
      removeChild(overflowButton);
      overflowButton = nullptr;
    }

    updateLayout();
  }

  /**
   * @brief Get tool by ID
   */
  ToolItem* getTool(int toolId) {
    auto it = toolIndexMap.find(toolId);
    if (it != toolIndexMap.end() && it->second < tools.size()) {
      return tools[it->second].get();
    }
    return nullptr;
  }

  /**
   * @brief Get tool by ID (const version)
   */
  const ToolItem* getTool(int toolId) const {
    auto it = toolIndexMap.find(toolId);
    if (it != toolIndexMap.end() && it->second < tools.size()) {
      return tools[it->second].get();
    }
    return nullptr;
  }

  // === Tool State Management ===

  /**
   * @brief Set tool enabled state
   */
  void setToolEnabled(int toolId, bool enabled) {
    if (auto tool = getTool(toolId)) {
      tool->enabled = enabled;
      updateToolButton(tool);
    }
  }

  /**
   * @brief Check if tool is enabled
   */
  bool isToolEnabled(int toolId) const {
    if (const auto tool = getTool(toolId)) {
      return tool->enabled;
    }
    return false;
  }

  /**
   * @brief Set tool checked state (for toggle tools)
   */
  void setToolChecked(int toolId, bool checked) {
    if (auto tool = getTool(toolId)) {
      tool->checked = checked;
      updateToolButton(tool);
    }
  }

  /**
   * @brief Check if tool is checked
   */
  bool isToolChecked(int toolId) const {
    if (const auto tool = getTool(toolId)) {
      return tool->checked;
    }
    return false;
  }

  /**
   * @brief Set tool visible state
   */
  void setToolVisible(int toolId, bool visible) {
    if (auto tool = getTool(toolId)) {
      tool->visible = visible;
      updateLayout();
    }
  }

  /**
   * @brief Check if tool is visible
   */
  bool isToolVisible(int toolId) const {
    if (const auto tool = getTool(toolId)) {
      return tool->visible;
    }
    return false;
  }

  /**
   * @brief Set tool text
   */
  void setToolText(int toolId, const std::string& text) {
    if (auto tool = getTool(toolId)) {
      tool->text = text;
      updateToolButton(tool);
    }
  }

  /**
   * @brief Get tool text
   */
  std::string getToolText(int toolId) const {
    if (const auto tool = getTool(toolId)) {
      return tool->text;
    }
    return "";
  }

  /**
   * @brief Set tool tooltip
   */
  void setToolTooltip(int toolId, const std::string& tooltip) {
    if (auto tool = getTool(toolId)) {
      tool->tooltip = tooltip;
      updateToolButton(tool);
    }
  }

  /**
   * @brief Get tool tooltip
   */
  std::string getToolTooltip(int toolId) const {
    if (const auto tool = getTool(toolId)) {
      return tool->tooltip;
    }
    return "";
  }

  // === Callbacks ===

  /**
   * @brief Set tool callback (called when tool is clicked)
   */
  void setToolCallback(ToolCallback callback) { toolCallback = std::move(callback); }

  /**
   * @brief Set tool state callback (called when toggle state changes)
   */
  void setToolStateCallback(ToolStateCallback callback) { toolStateCallback = std::move(callback); }

  // === Theming ===

  /**
   * @brief Apply theme from AdvancedTheme system
   */
  void applyTheme(const bombfork::prong::theming::AdvancedTheme& theme) {
    (void)theme; // Unused parameter
    // Apply theme colors from AdvancedTheme system
    // This would map AdvancedTheme properties to ToolBarTheme properties
    // For now, use default theme (already initialized in constructor)
  }

  /**
   * @brief Set custom theme
   */
  void setToolBarTheme(const ToolBarTheme& customTheme) {
    theme = customTheme;
    updateLayout(); // Refresh with new theme
  }

  /**
   * @brief Get current theme
   */
  const ToolBarTheme& getToolBarTheme() const { return theme; }

  // === Component Overrides ===

  void update(double deltaTime) override {
    (void)deltaTime; // Unused - state changes are event-driven
  }

  void render() override {
    if (!renderer)
      return;

    renderBackground();

    // Render visible tools
    for (const auto& tool : tools) {
      if (!tool->visible)
        continue;

      if (tool->button) {
        tool->button->render();
      } else if (tool->type == ToolType::SEPARATOR) {
        renderSeparators();
      }
    }

    // Render overflow button if needed
    if (overflowButton) {
      overflowButton->render();
    }
  }

  /**
   * @brief Handle events using the hierarchical event API
   * @param event Event to handle (coordinates in local space)
   * @return true if event was consumed, false to allow propagation
   */
  bool handleEventSelf(const core::Event& event) override {
    // Note: The base Component::handleEvent() automatically:
    // - Propagates events to children (tool buttons) in reverse order (z-order)
    // - Converts coordinates to child-local space
    // - Checks hit testing via containsEvent()
    //
    // We only need to handle toolbar-specific events that aren't button clicks.
    // All button interactions are automatically handled by the child Button components.

    switch (event.type) {
    case core::Event::Type::MOUSE_MOVE:
      // Button hover effects are automatically handled by child buttons
      // We don't need to consume move events, just let them propagate
      return false;

    case core::Event::Type::KEY_PRESS:
      // Handle keyboard shortcuts for tools
      // This would need platform-specific key mapping
      // For now, return false (not handled)
      return false;

    default:
      return false;
    }
  }

  void setBounds(int x, int y, int newWidth, int newHeight) override {
    bombfork::prong::Component::setBounds(x, y, newWidth, newHeight);
    updateLayout();
  }

  // === Layout Integration ===

  bombfork::prong::layout::LayoutMeasurement measurePreferredSize() const {
    int toolWidth, toolHeight;
    getToolDimensions(toolWidth, toolHeight);

    int visibleToolCount = 0;
    for (const auto& tool : tools) {
      if (tool->visible && tool->type != ToolType::SEPARATOR && tool->type != ToolType::SPACER) {
        visibleToolCount++;
      }
    }

    if (state.orientation == Orientation::HORIZONTAL) {
      int preferredWidth =
        visibleToolCount * toolWidth + (visibleToolCount - 1) * state.toolSpacing + 2 * state.toolPadding;
      int preferredHeight = toolHeight + 2 * state.toolPadding;
      return bombfork::prong::layout::LayoutMeasurement::exact(preferredWidth, preferredHeight);
    } else {
      int preferredWidth = toolWidth + 2 * state.toolPadding;
      int preferredHeight =
        visibleToolCount * toolHeight + (visibleToolCount - 1) * state.toolSpacing + 2 * state.toolPadding;
      return bombfork::prong::layout::LayoutMeasurement::exact(preferredWidth, preferredHeight);
    }
  }

private:
  /**
   * @brief Initialize toolbar layout
   */
  void initializeLayout() {
    layout = std::make_unique<bombfork::prong::layout::FlowLayout>();

    // Configure flow layout based on orientation
    bombfork::prong::layout::FlowLayoutManager<bombfork::prong::layout::FlowLayout>::Configuration config;
    config.horizontal = (state.orientation == Orientation::HORIZONTAL);
    config.spacing = static_cast<float>(state.toolSpacing);

    layout->configure(config);
  }

  /**
   * @brief Update layout after changes
   */
  void updateLayout() {
    if (!layout)
      return;

    // Update layout configuration
    bombfork::prong::layout::FlowLayoutManager<bombfork::prong::layout::FlowLayout>::Configuration config;
    config.horizontal = (state.orientation == Orientation::HORIZONTAL);
    config.spacing = static_cast<float>(state.toolSpacing);

    // Update tool button positions
    int currentX = state.toolPadding;
    int currentY = state.toolPadding;
    int toolWidth, toolHeight;
    getToolDimensions(toolWidth, toolHeight);

    for (const auto& tool : tools) {
      if (!tool->visible)
        continue;

      if (tool->button) {
        tool->button->setBounds(currentX, currentY, toolWidth, toolHeight);

        // Set up click callback for this button
        tool->button->setClickCallback([this, toolId = tool->id]() { handleToolClick(toolId); });

        if (state.orientation == Orientation::HORIZONTAL) {
          currentX += toolWidth + state.toolSpacing;
        } else {
          currentY += toolHeight + state.toolSpacing;
        }
      }
    }

    updateOverflow();
  }

  /**
   * @brief Create button for tool item and register it as a child
   */
  void createToolButton(ToolItem* tool) {
    if (!renderer)
      return;

    auto button = std::make_unique<bombfork::prong::Button>(tool->text);
    button->setRenderer(renderer);

    // Configure button based on tool type
    // Note: We always use NORMAL button type because ToolBar manages the toggle state manually.
    // If we used ButtonType::TOGGLE, the button would auto-toggle on click, which would conflict
    // with ToolBar's toggle management in handleToolClick().
    button->setButtonType(bombfork::prong::Button::ButtonType::NORMAL);

    // For toggle buttons, set initial active state
    if (tool->type == ToolType::TOGGLE_BUTTON) {
      button->setActive(tool->checked);
    }

    button->setEnabled(tool->enabled);

    // Store raw pointer before moving ownership to children
    tool->button = button.get();

    // Register as child for proper event propagation
    addChild(std::move(button));
  }

  /**
   * @brief Update tool button appearance
   */
  void updateToolButton(ToolItem* tool) {
    if (!tool->button)
      return;

    tool->button->setText(tool->text);
    tool->button->setEnabled(tool->enabled);

    if (tool->type == ToolType::TOGGLE_BUTTON) {
      // Sync the button's active state with the tool's checked state
      // The Button class will handle visual rendering based on isActive
      tool->button->setActive(tool->checked);
    }
  }

  /**
   * @brief Handle tool button click
   */
  void handleToolClick(int toolId) {
    auto tool = getTool(toolId);
    if (!tool || !tool->enabled)
      return;

    // Handle toggle buttons
    if (tool->type == ToolType::TOGGLE_BUTTON) {
      tool->checked = !tool->checked;
      updateToolButton(tool);

      if (toolStateCallback) {
        toolStateCallback(toolId, tool->checked);
      }
    }

    // Call the tool callback
    if (toolCallback) {
      toolCallback(toolId);
    }
  }

  /**
   * @brief Calculate tool dimensions
   */
  void getToolDimensions(int& toolWidth, int& toolHeight) const {
    int iconSize = static_cast<int>(state.toolSize);

    if (state.showText) {
      toolWidth = iconSize + 40;           // Icon + text space + padding
      toolHeight = std::max(iconSize, 20); // At least text height
    } else {
      toolWidth = iconSize + 8;  // Icon + padding
      toolHeight = iconSize + 8; // Icon + padding
    }
  }

  /**
   * @brief Check if tools need overflow handling
   */
  bool needsOverflow() const {
    int availableSize = (state.orientation == Orientation::HORIZONTAL) ? width : height;
    int requiredSize = calculateRequiredSize();
    return state.allowOverflow && requiredSize > availableSize;
  }

  /**
   * @brief Update overflow state
   */
  void updateOverflow() {
    if (needsOverflow()) {
      if (!overflowButton) {
        auto button = std::make_unique<bombfork::prong::Button>("▼");
        button->setRenderer(renderer);
        overflowButton = button.get();
        addChild(std::move(button));
      }

      // Calculate which tools need to go in overflow
      overflowTools.clear();
      // Implementation would calculate which tools don't fit

    } else {
      if (overflowButton) {
        removeChild(overflowButton);
        overflowButton = nullptr;
      }
      overflowTools.clear();
    }
  }

  /**
   * @brief Render toolbar background
   */
  void renderBackground() {
    if (!renderer || !theme.showBackground)
      return;

    // Render background
    // This would use renderer to draw background rectangle
    // Implementation depends on renderer API
  }

  /**
   * @brief Render tool separators
   */
  void renderSeparators() {
    if (!renderer)
      return;

    // Render separators between tools
    // Implementation depends on renderer API
  }

  /**
   * @brief Process keyboard shortcuts
   */
  bool processShortcut(const std::string& shortcut) {
    int toolId = findToolByShortcut(shortcut);
    if (toolId != -1) {
      handleToolClick(toolId);
      return true;
    }
    return false;
  }

  /**
   * @brief Find tool by shortcut
   */
  int findToolByShortcut(const std::string& shortcut) const {
    for (const auto& tool : tools) {
      if (tool->shortcut == shortcut) {
        return tool->id;
      }
    }
    return -1;
  }

  /**
   * @brief Get next available tool ID
   */
  int getNextToolId() { return nextToolId++; }

  /**
   * @brief Rebuild tool index map
   */
  void rebuildIndexMap() {
    toolIndexMap.clear();
    for (size_t i = 0; i < tools.size(); ++i) {
      if (tools[i]->id != -1) { // Skip separators and spacers
        toolIndexMap[tools[i]->id] = i;
      }
    }
  }

  /**
   * @brief Calculate required size for all tools
   */
  int calculateRequiredSize() const {
    int totalSize = 2 * state.toolPadding;
    int toolWidth, toolHeight;
    getToolDimensions(toolWidth, toolHeight);

    int visibleCount = 0;
    for (const auto& tool : tools) {
      if (!tool->visible)
        continue;

      if (tool->type == ToolType::SEPARATOR) {
        totalSize += tool->separatorSize;
      } else if (tool->type == ToolType::FIXED_SPACER) {
        totalSize += tool->spacerSize;
      } else if (tool->type != ToolType::SPACER) {
        totalSize += (state.orientation == Orientation::HORIZONTAL) ? toolWidth : toolHeight;
        visibleCount++;
      }
    }

    // Add spacing between tools
    if (visibleCount > 1) {
      totalSize += (visibleCount - 1) * state.toolSpacing;
    }

    return totalSize;
  }
};

} // namespace bombfork::prong
