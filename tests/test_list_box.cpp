#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/core/event.h>
#include <bombfork/prong/rendering/irenderer.h>

#include <cassert>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bombfork::prong;
using namespace bombfork::prong::rendering;

// === Mock Renderer ===

class MockRenderer : public IRenderer {
public:
  bool beginFrame() override { return true; }
  void endFrame() override {}
  void present() override {}
  void onWindowResize(int width, int height) override {
    (void)width;
    (void)height;
  }

  std::unique_ptr<TextureHandle> createTexture(uint32_t width, uint32_t height, const uint8_t* data) override {
    (void)width;
    (void)height;
    (void)data;
    return nullptr;
  }

  void updateTexture(TextureHandle* texture, const uint8_t* data) override {
    (void)texture;
    (void)data;
  }

  void deleteTexture(std::unique_ptr<TextureHandle> texture) override { (void)texture; }

  void clear(float r, float g, float b, float a) override {
    (void)r;
    (void)g;
    (void)b;
    (void)a;
  }

  void drawRect(int x, int y, int width, int height, float r, float g, float b, float a) override {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
  }

  void drawSprite(TextureHandle* texture, int x, int y, int width, int height, float alpha) override {
    (void)texture;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)alpha;
  }

  void drawSprites(const std::vector<SpriteDrawCmd>& sprites) override { (void)sprites; }

  int drawText(const std::string& text, int x, int y, float r, float g, float b, float a) override {
    (void)text;
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
    return 0;
  }

  std::pair<int, int> measureText(const std::string& text) override {
    // Return reasonable size for testing
    return {static_cast<int>(text.length() * 8), 16};
  }

  void enableScissorTest(int x, int y, int width, int height) override {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
  }

  void disableScissorTest() override {}
  void flushPendingBatches() override {}
  std::string getName() const override { return "MockRenderer"; }
  bool isInitialized() const override { return true; }
  uint64_t getGPUMemoryUsageMB() const override { return 0; }
  float getFrameTimeMs() const override { return 0.0f; }
  float getFPS() const override { return 0.0f; }
};

// === Tests ===

void test_listbox_construction() {
  ListBox listbox("Test ListBox");
  assert(listbox.getItems().empty());
  assert(listbox.getSelectedIndex() == -1);
  assert(listbox.getScrollOffset() == 0);

  std::cout << "✓ ListBox construction tests passed\n";
}

void test_listbox_item_management() {
  ListBox listbox;

  // Add items
  listbox.addItem("Item 1");
  listbox.addItem("Item 2");
  listbox.addItem("Item 3");

  assert(listbox.getItems().size() == 3);
  assert(listbox.getItems()[0] == "Item 1");
  assert(listbox.getItems()[1] == "Item 2");
  assert(listbox.getItems()[2] == "Item 3");

  // Set items
  std::vector<std::string> newItems = {"Apple", "Banana", "Cherry"};
  listbox.setItems(newItems);
  assert(listbox.getItems().size() == 3);
  assert(listbox.getItems()[0] == "Apple");

  // Clear items
  listbox.clearItems();
  assert(listbox.getItems().empty());
  assert(listbox.getSelectedIndex() == -1);

  std::cout << "✓ ListBox item management tests passed\n";
}

void test_listbox_selection() {
  ListBox listbox;
  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
  listbox.setItems(items);

  // Initial state
  assert(listbox.getSelectedIndex() == -1);
  assert(listbox.getSelectedItem() == "");

  // Select item
  listbox.setSelectedIndex(2);
  assert(listbox.getSelectedIndex() == 2);
  assert(listbox.getSelectedItem() == "Item 3");

  // Select another item
  listbox.setSelectedIndex(0);
  assert(listbox.getSelectedIndex() == 0);
  assert(listbox.getSelectedItem() == "Item 1");

  // Clear selection
  listbox.clearSelection();
  assert(listbox.getSelectedIndex() == -1);
  assert(listbox.getSelectedItem() == "");

  // Invalid selection (out of bounds)
  listbox.setSelectedIndex(10);
  assert(listbox.getSelectedIndex() == -1); // Should remain -1

  std::cout << "✓ ListBox selection tests passed\n";
}

void test_listbox_selection_callback() {
  ListBox listbox;
  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
  listbox.setItems(items);

  int callbackIndex = -1;
  std::string callbackItem;
  listbox.setSelectionCallback([&callbackIndex, &callbackItem](int index, const std::string& item) {
    callbackIndex = index;
    callbackItem = item;
  });

  // Select item - should trigger callback
  listbox.setSelectedIndex(1);
  assert(callbackIndex == 1);
  assert(callbackItem == "Item 2");

  // Select another item
  listbox.setSelectedIndex(2);
  assert(callbackIndex == 2);
  assert(callbackItem == "Item 3");

  std::cout << "✓ ListBox selection callback tests passed\n";
}

void test_listbox_mouse_click_selection() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 160); // Height for 5 items at 32px each
  listbox.setEnabled(true);

  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
  listbox.setItems(items);

  // Get item height from style
  int itemHeight = listbox.getStyle().itemHeight; // Default is 32
  int padding = listbox.getStyle().padding;       // Default is 4

  // Click on first item
  core::Event clickEvent{
    .type = core::Event::Type::MOUSE_PRESS, .localX = 50, .localY = padding + itemHeight / 2, .button = 0};
  bool handled = listbox.handleEvent(clickEvent);
  assert(handled);
  assert(listbox.getSelectedIndex() == 0);

  // Click on third item
  clickEvent.localY = padding + itemHeight * 2 + itemHeight / 2;
  handled = listbox.handleEvent(clickEvent);
  assert(handled);
  assert(listbox.getSelectedIndex() == 2);

  // Right click should be ignored
  core::Event rightClickEvent{
    .type = core::Event::Type::MOUSE_PRESS, .localX = 50, .localY = padding + itemHeight / 2, .button = 1};
  handled = listbox.handleEvent(rightClickEvent);
  assert(!handled);
  assert(listbox.getSelectedIndex() == 2); // Should remain unchanged

  std::cout << "✓ ListBox mouse click selection tests passed\n";
}

void test_listbox_scrolling() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 100); // Small height to require scrolling
  listbox.setEnabled(true);

  // Add many items to require scrolling
  std::vector<std::string> items;
  for (int i = 0; i < 20; ++i) {
    items.push_back("Item " + std::to_string(i + 1));
  }
  listbox.setItems(items);

  // Initial scroll offset
  assert(listbox.getScrollOffset() == 0);

  // Scroll down
  core::Event scrollEvent{.type = core::Event::Type::MOUSE_SCROLL, .localX = 50, .localY = 50, .scrollY = -1.0};
  bool handled = listbox.handleEvent(scrollEvent);
  assert(handled);
  assert(listbox.getScrollOffset() > 0);

  int offsetAfterDown = listbox.getScrollOffset();

  // Scroll up
  scrollEvent.scrollY = 1.0;
  handled = listbox.handleEvent(scrollEvent);
  assert(handled);
  assert(listbox.getScrollOffset() < offsetAfterDown);

  // Test scroll bounds - can't scroll beyond max
  listbox.setScrollOffset(99999);
  assert(listbox.getScrollOffset() == listbox.getMaxScrollOffset());

  // Can't scroll below 0
  listbox.setScrollOffset(-100);
  assert(listbox.getScrollOffset() == 0);

  std::cout << "✓ ListBox scrolling tests passed\n";
}

void test_listbox_keyboard_navigation() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 160);
  listbox.setEnabled(true);

  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
  listbox.setItems(items);

  // Select first item
  listbox.setSelectedIndex(2);
  assert(listbox.getSelectedIndex() == 2);

  // Press up arrow (key code 265)
  core::Event upKeyEvent{.type = core::Event::Type::KEY_PRESS, .key = 265};
  bool handled = listbox.handleEvent(upKeyEvent);
  assert(handled);
  assert(listbox.getSelectedIndex() == 1);

  // Press up arrow again
  handled = listbox.handleEvent(upKeyEvent);
  assert(handled);
  assert(listbox.getSelectedIndex() == 0);

  // Press up arrow at top - should stay at 0
  handled = listbox.handleEvent(upKeyEvent);
  assert(handled);
  assert(listbox.getSelectedIndex() == 0);

  // Press down arrow (key code 264)
  core::Event downKeyEvent{.type = core::Event::Type::KEY_PRESS, .key = 264};
  handled = listbox.handleEvent(downKeyEvent);
  assert(handled);
  assert(listbox.getSelectedIndex() == 1);

  // Navigate to bottom
  listbox.setSelectedIndex(4);
  handled = listbox.handleEvent(downKeyEvent);
  assert(handled);
  assert(listbox.getSelectedIndex() == 4); // Should stay at bottom

  std::cout << "✓ ListBox keyboard navigation tests passed\n";
}

void test_listbox_disabled_no_events() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 160);
  listbox.setEnabled(false);

  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
  listbox.setItems(items);

  // Try to click - should not handle
  core::Event clickEvent{.type = core::Event::Type::MOUSE_PRESS, .localX = 50, .localY = 20, .button = 0};
  bool handled = listbox.handleEvent(clickEvent);
  assert(!handled);
  assert(listbox.getSelectedIndex() == -1);

  // Try to scroll - should not handle
  core::Event scrollEvent{.type = core::Event::Type::MOUSE_SCROLL, .localX = 50, .localY = 50, .scrollY = -1.0};
  handled = listbox.handleEvent(scrollEvent);
  assert(!handled);
  assert(listbox.getScrollOffset() == 0);

  // Try keyboard - should not handle
  core::Event keyEvent{.type = core::Event::Type::KEY_PRESS, .key = 264};
  handled = listbox.handleEvent(keyEvent);
  assert(!handled);

  std::cout << "✓ ListBox disabled (no events) tests passed\n";
}

void test_listbox_mouse_move_hover() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 160);
  listbox.setEnabled(true);

  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
  listbox.setItems(items);

  int itemHeight = listbox.getStyle().itemHeight;
  int padding = listbox.getStyle().padding;

  // Mouse move over second item - note: hoveredIndex is private, but we can test the event is consumed
  core::Event moveEvent{
    .type = core::Event::Type::MOUSE_MOVE, .localX = 50, .localY = padding + itemHeight + itemHeight / 2};
  bool handled = listbox.handleEvent(moveEvent);
  assert(handled); // Should consume the move event

  // Mouse move over fourth item
  moveEvent.localY = padding + itemHeight * 3 + itemHeight / 2;
  handled = listbox.handleEvent(moveEvent);
  assert(handled);

  std::cout << "✓ ListBox mouse move hover tests passed\n";
}

void test_listbox_style_management() {
  ListBox listbox;

  // Test default style
  const auto& style = listbox.getStyle();
  assert(style.itemHeight == 32);
  assert(style.padding == 4);
  assert(style.borderWidth == 1.0f);
  assert(style.scrollbarWidth == 10);

  // Test custom style
  ListBox::Style customStyle;
  customStyle.itemHeight = 40;
  customStyle.padding = 8;
  customStyle.borderWidth = 2.0f;
  customStyle.scrollbarWidth = 12;

  listbox.setStyle(customStyle);
  const auto& newStyle = listbox.getStyle();
  assert(newStyle.itemHeight == 40);
  assert(newStyle.padding == 8);
  assert(newStyle.borderWidth == 2.0f);
  assert(newStyle.scrollbarWidth == 12);

  std::cout << "✓ ListBox style management tests passed\n";
}

void test_listbox_scroll_with_selection() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 100);
  listbox.setEnabled(true);

  // Add many items
  std::vector<std::string> items;
  for (int i = 0; i < 20; ++i) {
    items.push_back("Item " + std::to_string(i + 1));
  }
  listbox.setItems(items);

  // Select an item
  listbox.setSelectedIndex(5);
  assert(listbox.getSelectedIndex() == 5);

  // Scroll - selection should remain
  core::Event scrollEvent{.type = core::Event::Type::MOUSE_SCROLL, .localX = 50, .localY = 50, .scrollY = -1.0};
  listbox.handleEvent(scrollEvent);
  assert(listbox.getSelectedIndex() == 5); // Selection unchanged

  std::cout << "✓ ListBox scroll with selection tests passed\n";
}

void test_listbox_click_outside_items() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 200); // Large enough for 3 items with space below
  listbox.setEnabled(true);

  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
  listbox.setItems(items);

  listbox.setSelectedIndex(1);
  assert(listbox.getSelectedIndex() == 1);

  // Click below all items (in empty space)
  int itemHeight = listbox.getStyle().itemHeight;
  int padding = listbox.getStyle().padding;
  int clickY = padding + itemHeight * 4; // Below all items

  core::Event clickEvent{.type = core::Event::Type::MOUSE_PRESS, .localX = 50, .localY = clickY, .button = 0};
  bool handled = listbox.handleEvent(clickEvent);
  assert(!handled);                        // Click in empty space should not be handled
  assert(listbox.getSelectedIndex() == 1); // Selection unchanged

  std::cout << "✓ ListBox click outside items tests passed\n";
}

void test_listbox_event_consumption() {
  MockRenderer renderer;
  ListBox listbox;
  listbox.setRenderer(&renderer);
  listbox.setBounds(0, 0, 200, 160);
  listbox.setEnabled(true);

  std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
  listbox.setItems(items);

  // Mouse press on item should be consumed
  core::Event pressEvent{.type = core::Event::Type::MOUSE_PRESS, .localX = 50, .localY = 20, .button = 0};
  bool consumed = listbox.handleEvent(pressEvent);
  assert(consumed);

  // Scroll event should be consumed
  core::Event scrollEvent{.type = core::Event::Type::MOUSE_SCROLL, .localX = 50, .localY = 50, .scrollY = -1.0};
  consumed = listbox.handleEvent(scrollEvent);
  assert(consumed);

  // Mouse move should be consumed
  core::Event moveEvent{.type = core::Event::Type::MOUSE_MOVE, .localX = 50, .localY = 50};
  consumed = listbox.handleEvent(moveEvent);
  assert(consumed);

  // Key press (arrow keys) should be consumed
  core::Event keyEvent{.type = core::Event::Type::KEY_PRESS, .key = 264};
  consumed = listbox.handleEvent(keyEvent);
  assert(consumed);

  std::cout << "✓ ListBox event consumption tests passed\n";
}

int main() {
  std::cout << "Running ListBox component tests...\n\n";

  try {
    test_listbox_construction();
    test_listbox_item_management();
    test_listbox_selection();
    test_listbox_selection_callback();
    test_listbox_mouse_click_selection();
    test_listbox_scrolling();
    test_listbox_keyboard_navigation();
    test_listbox_disabled_no_events();
    test_listbox_mouse_move_hover();
    test_listbox_style_management();
    test_listbox_scroll_with_selection();
    test_listbox_click_outside_items();
    test_listbox_event_consumption();

    std::cout << "\n✓ All ListBox tests passed!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\n✗ Test failed with unknown exception\n";
    return 1;
  }
}
