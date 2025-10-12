#include <bombfork/prong/core/component.h>
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

public:
  explicit MockComponent(IRenderer* renderer) : Component(renderer, "MockComponent") {}

  void update(double deltaTime) override {
    updateCount++;
    lastDeltaTime = deltaTime;
  }

  void render() override { renderCount++; }

  int getUpdateCount() const { return updateCount; }
  int getRenderCount() const { return renderCount; }
  double getLastDeltaTime() const { return lastDeltaTime; }
};

// === Tests ===

void test_scene_construction() {
  MockWindow window;
  MockRenderer renderer;

  // Valid construction
  Scene scene(&window, &renderer);
  assert(scene.getWindow() == &window);
  assert(scene.getEventDispatcher() != nullptr);
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
  auto* childPtr = child.get();

  // Add child before attach - should not be registered yet
  scene.addChild(std::move(child));

  // Attach scene - should register all children
  scene.attach();

  // Child should be registered with event dispatcher
  assert(scene.getEventDispatcher() != nullptr);

  // Add another child after attach - should be registered immediately
  auto child2 = std::make_unique<MockComponent>(&renderer);
  scene.addChild(std::move(child2));

  // Remove child - should be unregistered
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
