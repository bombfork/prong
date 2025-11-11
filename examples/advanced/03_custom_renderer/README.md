# Custom Renderer Example

Demonstrates the IRenderer interface and renderer concepts. For a full custom renderer, implement all IRenderer methods.

## IRenderer Interface

```cpp
class IRenderer {
  virtual bool beginFrame() = 0;
  virtual void endFrame() = 0;
  virtual void present() = 0;
  virtual void clear(float r, float g, float b, float a) = 0;
  virtual void drawRect(int x, int y, int w, int h, float r, float g, float b, float a) = 0;
  virtual void drawText(const char* text, int x, int y, float scale, float r, float g, float b, float a) = 0;
  virtual void enableScissorTest(int x, int y, int w, int h) = 0;
  virtual void disableScissorTest() = 0;
};
```

## Key Concepts

- Frame lifecycle management
- Coordinate system handling
- Clipping/scissor testing
- Batching for performance

## Building

```bash
mise build-examples
./build/examples/advanced/03_custom_renderer/03_custom_renderer
```
