#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bombfork::prong::rendering {

/**
 * @brief Abstract texture handle
 *
 * Opaque handle for renderer-managed textures.
 * Implementations should derive from this to store their specific texture data.
 */
struct TextureHandle {
  uint32_t width = 0;
  uint32_t height = 0;
  virtual ~TextureHandle() = default;
};

/**
 * @brief Abstract graphics renderer interface
 *
 * Provides hardware-accelerated rendering for UI components.
 * Implementations can use any backend (OpenGL, Vulkan, DirectX, software, etc.)
 *
 * Design principles:
 * - Minimal surface area: only essential operations
 * - Backend agnostic: no API-specific types exposed
 * - Performance: supports batching and efficient operations
 * - Type-safe: uses strong typing and RAII patterns
 */
class IRenderer {
public:
  virtual ~IRenderer() = default;

  // === Frame Lifecycle ===

  /**
   * @brief Begin frame rendering
   * @return true if ready to render, false on error
   */
  virtual bool beginFrame() = 0;

  /**
   * @brief End frame rendering
   */
  virtual void endFrame() = 0;

  /**
   * @brief Present rendered frame to screen
   */
  virtual void present() = 0;

  /**
   * @brief Handle window resize
   * @param width New window width in pixels
   * @param height New window height in pixels
   */
  virtual void onWindowResize(int width, int height) = 0;

  // === Texture Management ===

  /**
   * @brief Create texture from RGBA data
   * @param width Texture width in pixels
   * @param height Texture height in pixels
   * @param data RGBA pixel data (width * height * 4 bytes)
   * @return Texture handle, nullptr if creation failed
   */
  virtual std::unique_ptr<TextureHandle> createTexture(uint32_t width, uint32_t height, const uint8_t* data) = 0;

  /**
   * @brief Update texture with new data
   * @param texture Texture handle to update
   * @param data New RGBA pixel data (must match texture dimensions)
   */
  virtual void updateTexture(TextureHandle* texture, const uint8_t* data) = 0;

  /**
   * @brief Delete texture and free GPU memory
   * @param texture Texture handle to delete
   */
  virtual void deleteTexture(std::unique_ptr<TextureHandle> texture) = 0;

  // === Drawing Primitives ===

  /**
   * @brief Clear screen with color
   * @param r Red component (0.0 - 1.0)
   * @param g Green component (0.0 - 1.0)
   * @param b Blue component (0.0 - 1.0)
   * @param a Alpha component (0.0 - 1.0)
   */
  virtual void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) = 0;

  /**
   * @brief Draw filled rectangle
   * @param x Rectangle X coordinate (top-left)
   * @param y Rectangle Y coordinate (top-left)
   * @param width Rectangle width in pixels
   * @param height Rectangle height in pixels
   * @param r Red component (0.0 - 1.0)
   * @param g Green component (0.0 - 1.0)
   * @param b Blue component (0.0 - 1.0)
   * @param a Alpha component (0.0 - 1.0)
   */
  virtual void drawRect(int x, int y, int width, int height, float r, float g, float b, float a = 1.0f) = 0;

  /**
   * @brief Draw sprite/texture at position
   * @param texture Sprite texture handle
   * @param x Screen X coordinate (top-left)
   * @param y Screen Y coordinate (top-left)
   * @param width Render width (0 = use texture width)
   * @param height Render height (0 = use texture height)
   * @param alpha Alpha transparency (0.0 - 1.0)
   */
  virtual void drawSprite(TextureHandle* texture, int x, int y, int width = 0, int height = 0, float alpha = 1.0f) = 0;

  /**
   * @brief Draw sprite batch for better performance
   * @param sprites Vector of sprite draw commands
   *
   * Batching multiple sprites reduces API overhead and improves performance.
   * Implementations may reorder sprites for optimal rendering.
   */
  struct SpriteDrawCmd {
    TextureHandle* texture;
    int x, y, width, height;
    float alpha;
  };
  virtual void drawSprites(const std::vector<SpriteDrawCmd>& sprites) = 0;

  // === Text Rendering ===

  /**
   * @brief Draw text string at position
   * @param text Text to render
   * @param x Text X coordinate (top-left)
   * @param y Text Y coordinate (top-left)
   * @param r Red component (0.0 - 1.0)
   * @param g Green component (0.0 - 1.0)
   * @param b Blue component (0.0 - 1.0)
   * @param a Alpha component (0.0 - 1.0)
   * @return Width of rendered text in pixels
   */
  virtual int drawText(const std::string& text, int x, int y, float r = 1.0f, float g = 1.0f, float b = 1.0f,
                       float a = 1.0f) = 0;

  /**
   * @brief Get text dimensions without rendering
   * @param text Text to measure
   * @return Width and height in pixels (as pair)
   */
  virtual std::pair<int, int> measureText(const std::string& text) = 0;

  // === Clipping Support ===

  /**
   * @brief Enable scissor test for content clipping
   * @param x Left edge of scissor rectangle
   * @param y Top edge of scissor rectangle (screen coordinates)
   * @param width Width of scissor rectangle
   * @param height Height of scissor rectangle
   *
   * All subsequent drawing will be clipped to this rectangle.
   */
  virtual void enableScissorTest(int x, int y, int width, int height) = 0;

  /**
   * @brief Disable scissor test
   */
  virtual void disableScissorTest() = 0;

  /**
   * @brief Flush all pending batched rendering commands
   *
   * Forces immediate rendering of all accumulated batch data.
   * Useful when you need to ensure all rendering happens with current state
   * (e.g., before disabling scissor test, changing blend modes, etc.).
   */
  virtual void flushPendingBatches() = 0;

  // === Information ===

  /**
   * @brief Get renderer name
   * @return Human-readable renderer name (e.g., "OpenGL 4.6", "Vulkan 1.3")
   */
  virtual std::string getName() const = 0;

  /**
   * @brief Check if renderer is initialized and ready
   * @return true if ready for rendering
   */
  virtual bool isInitialized() const = 0;

  /**
   * @brief Get GPU memory usage (optional, may return 0 if unsupported)
   * @return Used GPU memory in MB
   */
  virtual uint64_t getGPUMemoryUsageMB() const = 0;

  /**
   * @brief Get frame render time (optional, may return 0 if unsupported)
   * @return Last frame time in milliseconds
   */
  virtual float getFrameTimeMs() const = 0;

  /**
   * @brief Get frames per second (optional, may return 0 if unsupported)
   * @return Current FPS
   */
  virtual float getFPS() const = 0;
};

} // namespace bombfork::prong::rendering
