#pragma once

#include <bombfork/prong/rendering/irenderer.h>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bombfork::prong::tests {

/**
 * @brief Mock renderer implementation for testing
 *
 * This mock provides a simple implementation of IRenderer for unit tests,
 * allowing tests to verify layout and sizing behavior without requiring
 * a real graphics backend.
 */
class MockRenderer : public rendering::IRenderer {
private:
  int frameWidth = 800;
  int frameHeight = 600;
  int charWidth = 8;   // Default fixed-width font character width
  int charHeight = 16; // Default font height

public:
  /**
   * @brief Construct mock renderer with default dimensions
   */
  MockRenderer() = default;

  /**
   * @brief Construct mock renderer with custom dimensions
   * @param width Frame width in pixels
   * @param height Frame height in pixels
   */
  MockRenderer(int width, int height) : frameWidth(width), frameHeight(height) {}

  /**
   * @brief Set font metrics for text measurement
   * @param width Character width in pixels
   * @param height Character height in pixels
   */
  void setFontMetrics(int width, int height) {
    charWidth = width;
    charHeight = height;
  }

  // === Frame Lifecycle ===

  bool beginFrame() override { return true; }

  void endFrame() override {}

  void present() override {}

  void onWindowResize(int width, int height) override {
    frameWidth = width;
    frameHeight = height;
  }

  // === Texture Management ===

  std::unique_ptr<rendering::TextureHandle> createTexture(uint32_t width, uint32_t height,
                                                          const uint8_t* data) override {
    (void)data;
    auto handle = std::make_unique<rendering::TextureHandle>();
    handle->width = width;
    handle->height = height;
    return handle;
  }

  void updateTexture(rendering::TextureHandle* texture, const uint8_t* data) override {
    (void)texture;
    (void)data;
  }

  void deleteTexture(std::unique_ptr<rendering::TextureHandle> texture) override { (void)texture; }

  // === Drawing Primitives ===

  void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) override {
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

  void drawSprite(rendering::TextureHandle* texture, int x, int y, int width = 0, int height = 0,
                  float alpha = 1.0f) override {
    (void)texture;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)alpha;
  }

  void drawSprites(const std::vector<rendering::IRenderer::SpriteDrawCmd>& sprites) override { (void)sprites; }

  int drawText(const std::string& text, int x, int y, float r = 1.0f, float g = 1.0f, float b = 1.0f,
               float a = 1.0f) override {
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
    return static_cast<int>(text.length()) * charWidth;
  }

  std::pair<int, int> measureText(const std::string& text) override {
    return {static_cast<int>(text.length()) * charWidth, charHeight};
  }

  // === Clipping Support ===

  void enableScissorTest(int x, int y, int width, int height) override {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
  }

  void disableScissorTest() override {}

  void flushPendingBatches() override {}

  // === Information ===

  std::string getName() const override { return "MockRenderer"; }

  bool isInitialized() const override { return true; }

  uint64_t getGPUMemoryUsageMB() const override { return 0; }

  // === Performance Metrics ===

  float getFrameTimeMs() const override { return 16.67f; } // ~60 FPS

  float getFPS() const override { return 60.0f; }
};

} // namespace bombfork::prong::tests
