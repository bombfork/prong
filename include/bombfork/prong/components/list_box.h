#pragma once

#include <bombfork/prong/core/component.h>
#include <bombfork/prong/rendering/irenderer.h>
#include <bombfork/prong/theming/color.h>

#include <functional>
#include <string>
#include <vector>

namespace bombfork::prong {

/**
 * @brief Simple ListBox component for the new UI system
 *
 * Provides a scrollable list of text items with selection support.
 * This is the canonical ListBox class for t4c::ui namespace components.
 */
class ListBox : public Component {
public:
  using SelectionCallback = std::function<void(int index, const std::string& item)>;

  struct Style {
    theming::Color backgroundColor = theming::Color(0.15f, 0.15f, 0.15f, 1.0f);
    theming::Color itemColor = theming::Color(0.2f, 0.2f, 0.2f, 1.0f);
    theming::Color selectedColor = theming::Color(0.3f, 0.5f, 0.8f, 1.0f);
    theming::Color hoverColor = theming::Color(0.25f, 0.25f, 0.25f, 1.0f);
    theming::Color textColor = theming::Color::WHITE();
    theming::Color selectedTextColor = theming::Color::WHITE();
    theming::Color borderColor = theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
    theming::Color scrollbarColor = theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
    theming::Color scrollbarThumbColor = theming::Color(0.6f, 0.6f, 0.6f, 1.0f);

    int itemHeight = 32; // Increased from 20 to accommodate 24px font + spacing
    int padding = 4;
    float borderWidth = 1.0f;
    int scrollbarWidth = 10;
  };

private:
  Style style;
  std::vector<std::string> items;
  int selectedIndex = -1;
  int hoveredIndex = -1;
  int scrollOffset = 0;
  SelectionCallback selectionCallback;

public:
  explicit ListBox(const std::string& debugName = "ListBox") : Component(nullptr, debugName) {}

  virtual ~ListBox() = default;

  // === Style Management ===

  void setStyle(const Style& newStyle) { style = newStyle; }

  const Style& getStyle() const { return style; }

  // === Item Management ===

  void setItems(const std::vector<std::string>& newItems) {
    items = newItems;
    if (selectedIndex >= static_cast<int>(items.size())) {
      selectedIndex = -1;
    }
  }

  const std::vector<std::string>& getItems() const { return items; }

  void addItem(const std::string& item) { items.push_back(item); }

  void clearItems() {
    items.clear();
    selectedIndex = -1;
    hoveredIndex = -1;
    scrollOffset = 0;
  }

  // === Selection Management ===

  void setSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(items.size())) {
      selectedIndex = index;
      if (selectionCallback && selectedIndex >= 0) {
        selectionCallback(selectedIndex, items[selectedIndex]);
      }
    }
  }

  int getSelectedIndex() const { return selectedIndex; }

  std::string getSelectedItem() const {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size())) {
      return items[selectedIndex];
    }
    return "";
  }

  void setSelectionCallback(SelectionCallback callback) { selectionCallback = std::move(callback); }

  void clearSelection() { setSelectedIndex(-1); }

  // === Scrolling ===

  void setScrollOffset(int offset) { scrollOffset = std::max(0, std::min(offset, getMaxScrollOffset())); }

  int getScrollOffset() const { return scrollOffset; }

  int getMaxScrollOffset() const {
    int totalHeight = static_cast<int>(items.size()) * style.itemHeight;
    int visibleHeight = height - (style.padding * 2);
    return std::max(0, totalHeight - visibleHeight);
  }

  // === Event Handling ===

  bool handleClick(int /* localX */, int localY) override {
    if (!enabled) {
      return false;
    }

    // localX, localY are already in component-local coordinates (relative to this component's origin)
    // No need to subtract component position - that's already done by EventDispatcher

    int contentY = style.padding;
    int itemIndex = (localY - contentY + scrollOffset) / style.itemHeight;

    if (itemIndex >= 0 && itemIndex < static_cast<int>(items.size())) {
      setSelectedIndex(itemIndex);
      return true;
    }

    return false;
  }

  bool handleMousePress(int localX, int localY, int /* button */) override {
    // Delegate to handleClick for selection
    return handleClick(localX, localY);
  }

  bool handleMouseMove(int localX, int localY) override {
    if (!enabled) {
      return false;
    }

    // localX, localY are already in component-local coordinates
    // Check if within component bounds (0,0 to width,height in local space)
    if (localX >= 0 && localX < width && localY >= 0 && localY < height) {
      int contentY = style.padding;
      int itemIndex = (localY - contentY + scrollOffset) / style.itemHeight;

      if (itemIndex >= 0 && itemIndex < static_cast<int>(items.size())) {
        hoveredIndex = itemIndex;
      } else {
        hoveredIndex = -1;
      }
      return true;
    } else {
      hoveredIndex = -1;
    }

    return false;
  }

  void handleMouseLeave() override { hoveredIndex = -1; }

  bool handleScroll(int localX, int localY, double /* xoffset */, double yoffset) override {
    if (!enabled) {
      return false;
    }

    // localX, localY are already in component-local coordinates
    // Check if within component bounds
    if (localX >= 0 && localX < width && localY >= 0 && localY < height) {
      int delta = static_cast<int>(-yoffset * style.itemHeight);
      setScrollOffset(scrollOffset + delta);
      return true;
    }

    return false;
  }

  bool handleKey(int key, int action, int /* mods */) override {
    if (!enabled || action != 1) { // 1 = GLFW_PRESS
      return false;
    }

    // Arrow key navigation
    if (key == 265) { // Up arrow
      if (selectedIndex > 0) {
        setSelectedIndex(selectedIndex - 1);
      }
      return true;
    } else if (key == 264) { // Down arrow
      if (selectedIndex < static_cast<int>(items.size()) - 1) {
        setSelectedIndex(selectedIndex + 1);
      }
      return true;
    }

    return false;
  }

  // === Update ===

  void update(double deltaTime) override {
    (void)deltaTime; // Unused - state changes are event-driven
  }

  // === Rendering ===

  void render() override {
    if (!visible || !renderer) {
      return;
    }

    int gx = getGlobalX();
    int gy = getGlobalY();

    // Render background
    renderer->drawRect(gx, gy, width, height, style.backgroundColor.r, style.backgroundColor.g, style.backgroundColor.b,
                       style.backgroundColor.a);

    // Render border
    if (style.borderWidth > 0) {
      renderBorder();
    }

    // Render items
    renderItems();

    // Render scrollbar if needed
    if (getMaxScrollOffset() > 0) {
      renderScrollbar();
    }
  }

protected:
  void renderBorder() {
    if (!renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();
    int bw = static_cast<int>(style.borderWidth);

    // Top, bottom, left, right borders
    renderer->drawRect(gx, gy, width, bw, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);
    renderer->drawRect(gx, gy + height - bw, width, bw, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);
    renderer->drawRect(gx, gy, bw, height, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);
    renderer->drawRect(gx + width - bw, gy, bw, height, style.borderColor.r, style.borderColor.g, style.borderColor.b,
                       style.borderColor.a);
  }

  void renderItems() {
    if (!renderer || items.empty())
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();
    int contentX = gx + style.padding;
    int contentY = gy + style.padding;
    int contentWidth = width - (style.padding * 2);
    int contentHeight = height - (style.padding * 2);

    // Calculate visible item range
    int firstVisible = scrollOffset / style.itemHeight;
    int lastVisible = (scrollOffset + contentHeight) / style.itemHeight;

    for (int i = firstVisible; i <= lastVisible && i < static_cast<int>(items.size()); ++i) {
      int itemY = contentY + (i * style.itemHeight) - scrollOffset;

      // Skip if item is outside visible area
      if (itemY + style.itemHeight < contentY || itemY > contentY + contentHeight) {
        continue;
      }

      // Determine item color
      theming::Color bgColor = style.itemColor;
      theming::Color txtColor = style.textColor;

      if (i == selectedIndex) {
        bgColor = style.selectedColor;
        txtColor = style.selectedTextColor;
      } else if (i == hoveredIndex) {
        bgColor = style.hoverColor;
      }

      // Render item background
      renderer->drawRect(contentX, itemY, contentWidth, style.itemHeight, bgColor.r, bgColor.g, bgColor.b, bgColor.a);

      // Render item text - vertically centered
      auto [textWidth, textHeight] = renderer->measureText(items[i]);
      renderer->drawText(items[i], contentX + style.padding, itemY + (style.itemHeight - textHeight) / 2, txtColor.r,
                         txtColor.g, txtColor.b, txtColor.a);
    }
  }

  void renderScrollbar() {
    if (!renderer)
      return;

    int maxScroll = getMaxScrollOffset();
    if (maxScroll <= 0)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();

    // Calculate scrollbar dimensions
    int scrollbarX = gx + width - style.scrollbarWidth;
    int scrollbarY = gy;
    int scrollbarHeight = height;

    // Render scrollbar track
    renderer->drawRect(scrollbarX, scrollbarY, style.scrollbarWidth, scrollbarHeight, style.scrollbarColor.r,
                       style.scrollbarColor.g, style.scrollbarColor.b, style.scrollbarColor.a);

    // Calculate thumb dimensions
    int contentHeight = height - (style.padding * 2);
    int totalHeight = static_cast<int>(items.size()) * style.itemHeight;
    float visibleRatio = static_cast<float>(contentHeight) / static_cast<float>(totalHeight);
    int thumbHeight = std::max(20, static_cast<int>(scrollbarHeight * visibleRatio));

    // Calculate thumb position
    float scrollRatio = static_cast<float>(scrollOffset) / static_cast<float>(maxScroll);
    int thumbY = scrollbarY + static_cast<int>((scrollbarHeight - thumbHeight) * scrollRatio);

    // Render scrollbar thumb
    renderer->drawRect(scrollbarX, thumbY, style.scrollbarWidth, thumbHeight, style.scrollbarThumbColor.r,
                       style.scrollbarThumbColor.g, style.scrollbarThumbColor.b, style.scrollbarThumbColor.a);
  }
};

} // namespace bombfork::prong