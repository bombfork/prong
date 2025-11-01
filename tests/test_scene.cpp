#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/event.h>
#include <bombfork/prong/core/scene.h>
#include <bombfork/prong/events/iwindow.h>
#include <bombfork/prong/rendering/irenderer.h>

#include <cassert>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

using namespace bombfork::prong;
using namespace bombfork::prong::events;
using namespace bombfork::prong::rendering;

// === Mock Window ===

class MockWindow : public IWindow {
private:
  int width = 800;
  int height = 600;
  double cursorX = 0.0;
  double cursorY = 0.0;
  bool closeRequested = false;
  WindowCallbacks callbacks;

public:
  void getSize(int& w, int& h) const override {
    w = width;
    h = height;
  }

  void* getNativeHandle() override { return nullptr; }

  void getCursorPos(double& x, double& y) const override {
    x = cursorX;
    y = cursorY;
  }

  bool shouldClose() const override { return closeRequested; }

  bool isMouseButtonPressed(int button) const override {
    (void)button;
    return false;
  }

  bool isKeyPressed(int key) const override {
    (void)key;
    return false;
  }

  int getModifiers() const override { return 0; }

  void setCallbacks(const WindowCallbacks& cb) override { callbacks = cb; }

  // Test helpers
  void setSize(int w, int h) {
    width = w;
    height = h;
    if (callbacks.framebufferSize) {
      callbacks.framebufferSize(w, h);
    }
  }

  void setCursorPos(double x, double y) {
    cursorX = x;
    cursorY = y;
  }

  void requestClose() { closeRequested = true; }

  const WindowCallbacks& getCallbacks() const { return callbacks; }
};

// === Mock Renderer ===

class MockRenderer : public IRenderer {
private:
  bool frameBegun = false;
  int resizeCount = 0;
  int lastResizeWidth = 0;
  int lastResizeHeight = 0;
  int presentCount = 0;

public:
  bool beginFrame() override {
    frameBegun = true;
    return true;
  }

  void endFrame() override { frameBegun = false; }

  void present() override { presentCount++; }

  void onWindowResize(int width, int height) override {
    resizeCount++;
    lastResizeWidth = width;
    lastResizeHeight = height;
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
    (void)text;
    return {0, 0};
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

  // Test accessors
  int getResizeCount() const { return resizeCount; }
  int getLastResizeWidth() const { return lastResizeWidth; }
  int getLastResizeHeight() const { return lastResizeHeight; }
  int getPresentCount() const { return presentCount; }
};

// === Mock Component ===

class MockComponent : public Component {
private:
  int updateCount = 0;
  int renderCount = 0;
  double lastDeltaTime = 0.0;
  int eventCount = 0;
  core::Event::Type lastEventType = core::Event::Type::MOUSE_MOVE;

public:
  explicit MockComponent(IRenderer* renderer) : Component(renderer, "MockComponent") {}

  void update(double deltaTime) override {
    updateCount++;
    lastDeltaTime = deltaTime;
  }

  void render() override { renderCount++; }

  bool handleEventSelf(const core::Event& event) override {
    eventCount++;
    lastEventType = event.type;
    return false; // Don't consume events by default
  }

  int getUpdateCount() const { return updateCount; }
  int getRenderCount() const { return renderCount; }
  double getLastDeltaTime() const { return lastDeltaTime; }
  int getEventCount() const { return eventCount; }
  core::Event::Type getLastEventType() const { return lastEventType; }
};

// === Tests ===

void test_scene_construction() {
  MockWindow window;
  MockRenderer renderer;

  // Valid construction
  Scene scene(&window, &renderer);
  assert(scene.getWindow() == &window);
  assert(!scene.isAttached());

  // Scene should fill window dimensions
  int x, y, width, height;
  scene.getBounds(x, y, width, height);
  assert(x == 0);
  assert(y == 0);
  assert(width == 800);
  assert(height == 600);

  std::cout << "✓ Scene construction tests passed\n";
}

void test_scene_null_checks() {
  MockWindow window;
  MockRenderer renderer;

  // Null window should throw
  try {
    Scene scene(nullptr, &renderer);
    assert(false && "Should have thrown for null window");
  } catch (const std::invalid_argument& e) {
    // Expected
  }

  // Null renderer should throw
  try {
    Scene scene(&window, nullptr);
    assert(false && "Should have thrown for null renderer");
  } catch (const std::invalid_argument& e) {
    // Expected
  }

  std::cout << "✓ Scene null check tests passed\n";
}

void test_scene_attach_detach() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  // Initially not attached
  assert(!scene.isAttached());

  // Attach
  scene.attach();
  assert(scene.isAttached());

  // Detach
  scene.detach();
  assert(!scene.isAttached());

  // Multiple attach/detach should be safe
  scene.attach();
  scene.attach(); // Should be idempotent
  assert(scene.isAttached());

  scene.detach();
  scene.detach(); // Should be idempotent
  assert(!scene.isAttached());

  std::cout << "✓ Scene attach/detach tests passed\n";
}

void test_scene_window_resize() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  scene.attach();

  // Trigger resize
  scene.onWindowResize(1024, 768);

  // Scene bounds should update
  int x, y, width, height;
  scene.getBounds(x, y, width, height);
  assert(width == 1024);
  assert(height == 768);

  // Renderer should be notified
  assert(renderer.getResizeCount() == 1);
  assert(renderer.getLastResizeWidth() == 1024);
  assert(renderer.getLastResizeHeight() == 768);

  std::cout << "✓ Scene window resize tests passed\n";
}

void test_scene_present() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  int initialPresentCount = renderer.getPresentCount();

  // Present should delegate to renderer
  scene.present();
  assert(renderer.getPresentCount() == initialPresentCount + 1);

  scene.present();
  assert(renderer.getPresentCount() == initialPresentCount + 2);

  std::cout << "✓ Scene present tests passed\n";
}

void test_scene_child_registration() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  auto child = std::make_unique<MockComponent>(&renderer);
  child->setBounds(0, 0, 100, 100);
  child->setVisible(true);
  child->setEnabled(true);
  auto* childPtr = child.get();

  // Add child before attach
  scene.addChild(std::move(child));

  // Attach scene
  scene.attach();

  // Add another child after attach
  auto child2 = std::make_unique<MockComponent>(&renderer);
  child2->setBounds(0, 0, 100, 100);
  child2->setVisible(true);
  child2->setEnabled(true);
  auto* child2Ptr = child2.get();
  scene.addChild(std::move(child2));

  // Test that events propagate to children using hierarchical model
  core::Event mouseEvent{.type = core::Event::Type::MOUSE_PRESS, .localX = 10, .localY = 10, .button = 0};

  scene.handleEvent(mouseEvent);

  // Both children should have received the event (last added child receives event first in z-order)
  assert(child2Ptr->getEventCount() > 0 && "Child added after attach should receive events");

  // Remove child
  bool removed = scene.removeChild(childPtr);
  assert(removed);

  scene.detach();

  std::cout << "✓ Scene child registration tests passed\n";
}

void test_scene_update_render() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  auto child = std::make_unique<MockComponent>(&renderer);
  auto* childPtr = child.get();
  scene.addChild(std::move(child));

  // Update scene
  scene.update(0.016);
  // Note: update() itself doesn't call updateAll(), that's done externally
  // But we can verify the scene's update() doesn't crash

  // Render scene
  scene.render();
  // Similarly, render() itself doesn't call renderAll()

  // Verify child was not updated/rendered (that requires updateAll/renderAll)
  assert(childPtr->getUpdateCount() == 0);
  assert(childPtr->getRenderCount() == 0);

  // Now call updateAll and renderAll
  scene.updateAll(0.016);
  assert(childPtr->getUpdateCount() == 1);
  assert(childPtr->getLastDeltaTime() == 0.016);

  scene.renderAll();
  assert(childPtr->getRenderCount() == 1);

  std::cout << "✓ Scene update/render tests passed\n";
}

void test_scene_destructor_detaches() {
  MockWindow window;
  MockRenderer renderer;

  {
    Scene scene(&window, &renderer);
    scene.attach();
    assert(scene.isAttached());
    // Scene destructor should call detach automatically
  }

  // If we get here without crash, destructor worked correctly

  std::cout << "✓ Scene destructor tests passed\n";
}

void test_scene_resize_callback() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  scene.attach();

  // Simulate window resize through callback
  window.setSize(1920, 1080);

  // Note: The window resize callback is set up but we need to manually
  // trigger it through the scene for this test
  scene.onWindowResize(1920, 1080);

  // Scene should have new dimensions
  int x, y, width, height;
  scene.getBounds(x, y, width, height);
  assert(width == 1920);
  assert(height == 1080);

  std::cout << "✓ Scene resize callback tests passed\n";
}

void test_scene_recursive_registration() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  // Create a nested component hierarchy:
  // parent (0, 0, 400, 400)
  //   └─ child (50, 50, 300, 300)
  //        └─ grandchild (75, 75, 150, 150)

  auto parent = std::make_unique<MockComponent>(&renderer);
  parent->setBounds(0, 0, 400, 400);
  parent->setVisible(true);
  parent->setEnabled(true);

  auto child = std::make_unique<MockComponent>(&renderer);
  child->setBounds(50, 50, 300, 300);
  child->setVisible(true);
  child->setEnabled(true);

  auto grandchild = std::make_unique<MockComponent>(&renderer);
  grandchild->setBounds(75, 75, 150, 150);
  grandchild->setVisible(true);
  grandchild->setEnabled(true);
  auto* grandchildPtr = grandchild.get();

  // Build hierarchy: grandchild -> child -> parent
  child->addChild(std::move(grandchild));
  parent->addChild(std::move(child));

  // Add parent to scene BEFORE attaching
  scene.addChild(std::move(parent));

  // Now attach the scene
  scene.attach();

  // Test that events propagate through the hierarchy
  // The child is at local position (50, 50) within the parent
  // The grandchild is at local position (75, 75) within the child
  // So global position of grandchild is (50+75, 50+75) = (125, 125)

  // Send event to a point within the grandchild's bounds
  core::Event event{.type = core::Event::Type::MOUSE_PRESS, .localX = 130, .localY = 130, .button = 0};

  scene.handleEvent(event);

  // The grandchild should have received the event (converted to its local coordinates)
  assert(grandchildPtr->getEventCount() > 0 && "Grandchild should receive events through hierarchy");
  assert(grandchildPtr->getLastEventType() == core::Event::Type::MOUSE_PRESS);

  std::cout << "✓ Scene recursive registration (before attach) tests passed\n";
}

void test_scene_recursive_registration_after_attach() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  // Attach scene first
  scene.attach();

  // Create nested hierarchy
  auto parent = std::make_unique<MockComponent>(&renderer);
  parent->setBounds(0, 0, 400, 400);
  parent->setVisible(true);
  parent->setEnabled(true);

  auto child = std::make_unique<MockComponent>(&renderer);
  child->setBounds(50, 50, 300, 300);
  child->setVisible(true);
  child->setEnabled(true);

  auto grandchild = std::make_unique<MockComponent>(&renderer);
  grandchild->setBounds(75, 75, 150, 150);
  grandchild->setVisible(true);
  grandchild->setEnabled(true);
  auto* grandchildPtr = grandchild.get();

  // Build hierarchy
  child->addChild(std::move(grandchild));
  parent->addChild(std::move(child));

  // Add parent to ALREADY ATTACHED scene
  scene.addChild(std::move(parent));

  // Test that events propagate through the hierarchy even when added after attach
  core::Event event{.type = core::Event::Type::MOUSE_PRESS, .localX = 130, .localY = 130, .button = 0};

  scene.handleEvent(event);

  // The grandchild should have received the event
  assert(grandchildPtr->getEventCount() > 0 && "Grandchild should receive events when added after attach");
  assert(grandchildPtr->getLastEventType() == core::Event::Type::MOUSE_PRESS);

  std::cout << "✓ Scene recursive registration (after attach) tests passed\n";
}

void test_scene_deeply_nested_registration() {
  MockWindow window;
  MockRenderer renderer;
  Scene scene(&window, &renderer);

  // Create a deeply nested hierarchy (5 levels)
  auto level1 = std::make_unique<MockComponent>(&renderer);
  level1->setBounds(0, 0, 500, 500);
  level1->setVisible(true);
  level1->setEnabled(true);

  auto level2 = std::make_unique<MockComponent>(&renderer);
  level2->setBounds(10, 10, 480, 480);
  level2->setVisible(true);
  level2->setEnabled(true);

  auto level3 = std::make_unique<MockComponent>(&renderer);
  level3->setBounds(20, 20, 460, 460);
  level3->setVisible(true);
  level3->setEnabled(true);

  auto level4 = std::make_unique<MockComponent>(&renderer);
  level4->setBounds(30, 30, 440, 440);
  level4->setVisible(true);
  level4->setEnabled(true);

  auto level5 = std::make_unique<MockComponent>(&renderer);
  level5->setBounds(40, 40, 420, 420);
  level5->setVisible(true);
  level5->setEnabled(true);
  auto* level5Ptr = level5.get();

  // Build deep hierarchy
  level4->addChild(std::move(level5));
  level3->addChild(std::move(level4));
  level2->addChild(std::move(level3));
  level1->addChild(std::move(level2));

  scene.addChild(std::move(level1));
  scene.attach();

  // Test that events propagate through deep hierarchies
  // Global position of level5: 10+20+30+40 = 100
  // Send event to a point within level5's bounds
  core::Event event{.type = core::Event::Type::MOUSE_PRESS, .localX = 105, .localY = 105, .button = 0};

  scene.handleEvent(event);

  // All levels that contain the point should have received the event
  assert(level5Ptr->getEventCount() > 0 && "Level 5 should receive events through deep hierarchy");
  assert(level5Ptr->getLastEventType() == core::Event::Type::MOUSE_PRESS);

  std::cout << "✓ Scene deeply nested registration tests passed\n";
}

int main() {
  std::cout << "Running Scene class tests...\n\n";

  try {
    test_scene_construction();
    test_scene_null_checks();
    test_scene_attach_detach();
    test_scene_window_resize();
    test_scene_present();
    test_scene_child_registration();
    test_scene_update_render();
    test_scene_destructor_detaches();
    test_scene_resize_callback();
    test_scene_recursive_registration();
    test_scene_recursive_registration_after_attach();
    test_scene_deeply_nested_registration();

    std::cout << "\n✓ All Scene tests passed!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\n✗ Test failed with unknown exception\n";
    return 1;
  }
}
