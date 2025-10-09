#include "bombfork/prong/components/button.h"
#include "bombfork/prong/core/component.h"
#include "bombfork/prong/layout/flow_layout.h"
#include "bombfork/prong/layout/layout_measurement.h"

#include <bombfork/prong/generic/toolbar.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bombfork {
namespace prong {
namespace rendering {
class IRenderer;
}
namespace theming {
class AdvancedTheme;
}
} // namespace prong
} // namespace bombfork

namespace bombfork::prong {

ToolBar::ToolBar(bombfork::prong::rendering::IRenderer* renderer) : bombfork::prong::Component(renderer) {
  initializeLayout();
}

void ToolBar::setOrientation(Orientation orientation) {
  if (state.orientation != orientation) {
    state.orientation = orientation;
    updateLayout();
  }
}

void ToolBar::setToolSize(ToolSize size) {
  if (state.toolSize != size) {
    state.toolSize = size;
    updateLayout();
  }
}

void ToolBar::setToolSpacing(int spacing) {
  if (state.toolSpacing != spacing) {
    state.toolSpacing = spacing;
    updateLayout();
  }
}

void ToolBar::setShowText(bool show) {
  if (state.showText != show) {
    state.showText = show;
    updateLayout();
  }
}

void ToolBar::setShowTooltips(bool show) {
  state.showTooltips = show;
}

int ToolBar::addTool(const std::string& text, const std::string& iconPath, const std::string& tooltip,
                     const std::string& shortcut) {
  auto tool = std::make_unique<ToolItem>();
  tool->id = getNextToolId();
  tool->type = ToolType::PUSH_BUTTON;
  tool->text = text;
  tool->iconPath = iconPath;
  tool->tooltip = tooltip.empty() ? text : tooltip;
  tool->shortcut = shortcut;
  tool->button = createToolButton(tool.get());

  int toolId = tool->id;
  tools.push_back(std::move(tool));
  rebuildIndexMap();
  updateLayout();

  return toolId;
}

int ToolBar::addToggleTool(const std::string& text, const std::string& iconPath, const std::string& tooltip,
                           bool initiallyChecked, const std::string& shortcut) {
  auto tool = std::make_unique<ToolItem>();
  tool->id = getNextToolId();
  tool->type = ToolType::TOGGLE_BUTTON;
  tool->text = text;
  tool->iconPath = iconPath;
  tool->tooltip = tooltip.empty() ? text : tooltip;
  tool->checked = initiallyChecked;
  tool->shortcut = shortcut;
  tool->button = createToolButton(tool.get());

  int toolId = tool->id;
  tools.push_back(std::move(tool));
  rebuildIndexMap();
  updateLayout();

  return toolId;
}

int ToolBar::addDropdownTool(const std::string& text, const std::string& iconPath, const std::string& tooltip,
                             const std::string& shortcut) {
  auto tool = std::make_unique<ToolItem>();
  tool->id = getNextToolId();
  tool->type = ToolType::DROPDOWN_BUTTON;
  tool->text = text;
  tool->iconPath = iconPath;
  tool->tooltip = tooltip.empty() ? text : tooltip;
  tool->shortcut = shortcut;
  tool->button = createToolButton(tool.get());

  int toolId = tool->id;
  tools.push_back(std::move(tool));
  rebuildIndexMap();
  updateLayout();

  return toolId;
}

void ToolBar::addSeparator(int size) {
  auto tool = std::make_unique<ToolItem>();
  tool->id = -1; // Separators don't have IDs
  tool->type = ToolType::SEPARATOR;
  tool->separatorSize = size;

  tools.push_back(std::move(tool));
  rebuildIndexMap();
  updateLayout();
}

void ToolBar::addSpacer(int size) {
  auto tool = std::make_unique<ToolItem>();
  tool->id = -1; // Spacers don't have IDs
  tool->type = (size == 0) ? ToolType::SPACER : ToolType::FIXED_SPACER;
  tool->spacerSize = size;

  tools.push_back(std::move(tool));
  rebuildIndexMap();
  updateLayout();
}

int ToolBar::addLabel(const std::string& text) {
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

void ToolBar::removeTool(int toolId) {
  auto it = toolIndexMap.find(toolId);
  if (it != toolIndexMap.end()) {
    size_t index = it->second;
    if (index < tools.size()) {
      tools.erase(tools.begin() + index);
      rebuildIndexMap();
      updateLayout();
    }
  }
}

void ToolBar::clearTools() {
  tools.clear();
  toolIndexMap.clear();
  overflowTools.clear();
  updateLayout();
}

ToolBar::ToolItem* ToolBar::getTool(int toolId) {
  auto it = toolIndexMap.find(toolId);
  if (it != toolIndexMap.end() && it->second < tools.size()) {
    return tools[it->second].get();
  }
  return nullptr;
}

const ToolBar::ToolItem* ToolBar::getTool(int toolId) const {
  auto it = toolIndexMap.find(toolId);
  if (it != toolIndexMap.end() && it->second < tools.size()) {
    return tools[it->second].get();
  }
  return nullptr;
}

void ToolBar::setToolEnabled(int toolId, bool enabled) {
  if (auto tool = getTool(toolId)) {
    tool->enabled = enabled;
    updateToolButton(tool);
  }
}

bool ToolBar::isToolEnabled(int toolId) const {
  if (const auto tool = getTool(toolId)) {
    return tool->enabled;
  }
  return false;
}

void ToolBar::setToolChecked(int toolId, bool checked) {
  if (auto tool = getTool(toolId)) {
    tool->checked = checked;
    updateToolButton(tool);
  }
}

bool ToolBar::isToolChecked(int toolId) const {
  if (const auto tool = getTool(toolId)) {
    return tool->checked;
  }
  return false;
}

void ToolBar::setToolVisible(int toolId, bool visible) {
  if (auto tool = getTool(toolId)) {
    tool->visible = visible;
    updateLayout();
  }
}

bool ToolBar::isToolVisible(int toolId) const {
  if (const auto tool = getTool(toolId)) {
    return tool->visible;
  }
  return false;
}

void ToolBar::setToolText(int toolId, const std::string& text) {
  if (auto tool = getTool(toolId)) {
    tool->text = text;
    updateToolButton(tool);
  }
}

std::string ToolBar::getToolText(int toolId) const {
  if (const auto tool = getTool(toolId)) {
    return tool->text;
  }
  return "";
}

void ToolBar::setToolTooltip(int toolId, const std::string& tooltip) {
  if (auto tool = getTool(toolId)) {
    tool->tooltip = tooltip;
    updateToolButton(tool);
  }
}

std::string ToolBar::getToolTooltip(int toolId) const {
  if (const auto tool = getTool(toolId)) {
    return tool->tooltip;
  }
  return "";
}

void ToolBar::setToolCallback(ToolCallback callback) {
  toolCallback = callback;
}

void ToolBar::setToolStateCallback(ToolStateCallback callback) {
  toolStateCallback = callback;
}

void ToolBar::applyTheme(const bombfork::prong::theming::AdvancedTheme& theme) {
  (void)theme; // Unused parameter
  // Apply theme colors from AdvancedTheme system
  // This would map AdvancedTheme properties to ToolBarTheme properties
  // For now, use default theme (already initialized in constructor)
}

void ToolBar::setToolBarTheme(const ToolBarTheme& customTheme) {
  theme = customTheme;
  updateLayout(); // Refresh with new theme
}

void ToolBar::render() {
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

bool ToolBar::handleClick(int localX, int localY) {
  // Check overflow button first
  if (overflowButton) {
    // Convert toolbar-local coords to button-local coords
    int buttonLocalX = localX - overflowButton->x;
    int buttonLocalY = localY - overflowButton->y;
    if (overflowButton->handleClick(buttonLocalX, buttonLocalY)) {
      // Show overflow menu
      return true;
    }
  }

  // Check each visible tool button
  for (const auto& tool : tools) {
    if (!tool->visible || !tool->button)
      continue;

    // Convert toolbar-local coords to button-local coords
    int buttonLocalX = localX - tool->button->x;
    int buttonLocalY = localY - tool->button->y;

    if (tool->button->handleClick(buttonLocalX, buttonLocalY)) {
      handleToolClick(tool->id);
      return true;
    }
  }

  return false;
}

bool ToolBar::handleMousePress(int localX, int localY, int button) {
  // Check overflow button first
  if (overflowButton) {
    int buttonLocalX = localX - overflowButton->x;
    int buttonLocalY = localY - overflowButton->y;
    if (overflowButton->handleMousePress(buttonLocalX, buttonLocalY, button)) {
      return true;
    }
  }

  // Check each visible tool button
  for (const auto& tool : tools) {
    if (!tool->visible || !tool->button)
      continue;

    int buttonLocalX = localX - tool->button->x;
    int buttonLocalY = localY - tool->button->y;

    if (tool->button->handleMousePress(buttonLocalX, buttonLocalY, button)) {
      return true;
    }
  }

  return false;
}

bool ToolBar::handleMouseRelease(int localX, int localY, int button) {
  // Check overflow button first
  if (overflowButton) {
    int buttonLocalX = localX - overflowButton->x;
    int buttonLocalY = localY - overflowButton->y;
    if (overflowButton->handleMouseRelease(buttonLocalX, buttonLocalY, button)) {
      return true;
    }
  }

  // Check each visible tool button
  for (const auto& tool : tools) {
    if (!tool->visible || !tool->button)
      continue;

    int buttonLocalX = localX - tool->button->x;
    int buttonLocalY = localY - tool->button->y;

    if (tool->button->handleMouseRelease(buttonLocalX, buttonLocalY, button)) {
      return true;
    }
  }

  return false;
}

bool ToolBar::handleMouseMove(int localX, int localY) {
  bool handled = false;

  // Check overflow button first
  if (overflowButton) {
    int buttonLocalX = localX - overflowButton->x;
    int buttonLocalY = localY - overflowButton->y;
    if (overflowButton->handleMouseMove(buttonLocalX, buttonLocalY)) {
      handled = true;
    }
  }

  // Send move events to all visible tool buttons
  // This ensures proper hover state updates
  for (const auto& tool : tools) {
    if (!tool->visible || !tool->button)
      continue;

    int buttonLocalX = localX - tool->button->x;
    int buttonLocalY = localY - tool->button->y;

    if (tool->button->handleMouseMove(buttonLocalX, buttonLocalY)) {
      handled = true;
    }
  }

  return handled;
}

bool ToolBar::handleKey(int key, int action, int mods) {
  (void)key;    // Unused parameter
  (void)action; // Unused parameter
  (void)mods;   // Unused parameter
  // Handle keyboard shortcuts
  // This would need platform-specific key mapping
  // For now, return false (not handled)
  return false;
}

void ToolBar::setBounds(int x, int y, int width, int height) {
  bombfork::prong::Component::setBounds(x, y, width, height);
  updateLayout();
}

bombfork::prong::layout::LayoutMeasurement ToolBar::measurePreferredSize() const {
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

void ToolBar::initializeLayout() {
  layout = std::make_unique<bombfork::prong::layout::FlowLayout>();

  // Configure flow layout based on orientation
  bombfork::prong::layout::FlowLayoutManager<bombfork::prong::layout::FlowLayout>::Configuration config;
  config.horizontal = (state.orientation == Orientation::HORIZONTAL);
  config.spacing = static_cast<float>(state.toolSpacing);

  layout->configure(config);
}

void ToolBar::updateLayout() {
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

      if (state.orientation == Orientation::HORIZONTAL) {
        currentX += toolWidth + state.toolSpacing;
      } else {
        currentY += toolHeight + state.toolSpacing;
      }
    }
  }

  updateOverflow();
}

std::unique_ptr<bombfork::prong::Button> ToolBar::createToolButton(ToolItem* tool) {
  if (!renderer)
    return nullptr;

  auto button = std::make_unique<bombfork::prong::Button>(renderer);
  button->setText(tool->text);

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

  return button;
}

void ToolBar::updateToolButton(ToolItem* tool) {
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

void ToolBar::handleToolClick(int toolId) {
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

void ToolBar::getToolDimensions(int& toolWidth, int& toolHeight) const {
  int iconSize = static_cast<int>(state.toolSize);

  if (state.showText) {
    toolWidth = iconSize + 40;           // Icon + text space + padding
    toolHeight = std::max(iconSize, 20); // At least text height
  } else {
    toolWidth = iconSize + 8;  // Icon + padding
    toolHeight = iconSize + 8; // Icon + padding
  }
}

bool ToolBar::needsOverflow() const {
  int availableSize = (state.orientation == Orientation::HORIZONTAL) ? width : height;
  int requiredSize = calculateRequiredSize();
  return state.allowOverflow && requiredSize > availableSize;
}

void ToolBar::updateOverflow() {
  if (needsOverflow()) {
    if (!overflowButton) {
      overflowButton = std::make_unique<bombfork::prong::Button>(renderer);
      overflowButton->setText("▼");
    }

    // Calculate which tools need to go in overflow
    overflowTools.clear();
    // Implementation would calculate which tools don't fit

  } else {
    overflowButton.reset();
    overflowTools.clear();
  }
}

void ToolBar::renderBackground() {
  if (!renderer || !theme.showBackground)
    return;

  // Render background
  // This would use renderer to draw background rectangle
  // Implementation depends on renderer API
}

void ToolBar::renderSeparators() {
  if (!renderer)
    return;

  // Render separators between tools
  // Implementation depends on renderer API
}

bool ToolBar::processShortcut(const std::string& shortcut) {
  int toolId = findToolByShortcut(shortcut);
  if (toolId != -1) {
    handleToolClick(toolId);
    return true;
  }
  return false;
}

int ToolBar::findToolByShortcut(const std::string& shortcut) const {
  for (const auto& tool : tools) {
    if (tool->shortcut == shortcut) {
      return tool->id;
    }
  }
  return -1;
}

void ToolBar::rebuildIndexMap() {
  toolIndexMap.clear();
  for (size_t i = 0; i < tools.size(); ++i) {
    if (tools[i]->id != -1) { // Skip separators and spacers
      toolIndexMap[tools[i]->id] = i;
    }
  }
}

int ToolBar::calculateRequiredSize() const {
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

} // namespace bombfork::prong