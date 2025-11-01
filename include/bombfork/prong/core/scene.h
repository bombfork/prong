#pragma once

#include <bombfork/prong/core/component.h>
#include <bombfork/prong/core/event.h>
#include <bombfork/prong/events/iwindow.h>
#include <bombfork/prong/rendering/irenderer.h>

#include <memory>

namespace bombfork::prong {

/**
 * @brief Root scene component that manages the entire UI hierarchy
 *
 * Scene is the top-level container for all UI components. It owns the
 * window and renderer references and automatically handles window resizing.
 *
 * Key responsibilities:
 * - Uses hierarchical event handling (Component::handleEvent)
 * - Automatically fills window dimensions
 * - Handles window resize events and propagates to children
 * - Provides simplified update/render/present interface
 * - Entry point for window events into the component hierarchy
 *
 * Usage:
 * ```cpp
 * IWindow* window = createWindow();
 * IRenderer* renderer = createRenderer();
 * Scene scene(window, renderer);
 * scene.attach();
 *
 * // Add UI components - renderer is inherited from scene
 * auto panel = std::make_unique<Panel<>>();
 * scene.addChild(std::move(panel));
 *
 * // Main loop - convert window events to Event structs
 * while (!window->shouldClose()) {
 *   // In window callbacks, call scene.handleEvent() with Event struct
 *   scene.updateAll(deltaTime);
 *   scene.renderAll();
 *   scene.present();
 * }
 *
 * scene.detach();
 * ```
 */
class Scene : public Component {
private:
  events::IWindow* window = nullptr;
  bool attached = false;

public:
  /**
   * @brief Create a scene with window and renderer
   * @param window Window interface (must not be null)
   * @param renderer Renderer interface (must not be null)
   */
  explicit Scene(events::IWindow* window, bombfork::prong::rendering::IRenderer* renderer)
    : Component(renderer, "Scene"), window(window) {
    if (!window) {
      throw std::invalid_argument("Scene: window cannot be null");
    }
    if (!renderer) {
      throw std::invalid_argument("Scene: renderer cannot be null");
    }

    // Initialize scene bounds to window size
    int windowWidth, windowHeight;
    window->getSize(windowWidth, windowHeight);
    setBounds(0, 0, windowWidth, windowHeight);
  }

  virtual ~Scene() {
    if (attached) {
      detach();
    }
  }

  // === Lifecycle Management ===

  /**
   * @brief Attach scene to window and start event handling
   *
   * This ensures the scene bounds match the window size.
   * Note: In the new hierarchical event model, window callbacks should
   * call scene->handleEvent() directly instead of using EventDispatcher.
   */
  void attach() {
    if (attached) {
      return;
    }

    // Ensure window size is current
    int windowWidth, windowHeight;
    window->getSize(windowWidth, windowHeight);
    setBounds(0, 0, windowWidth, windowHeight);

    attached = true;
  }

  /**
   * @brief Detach scene from window and stop event handling
   *
   * This marks the scene as detached. Window callbacks should no longer
   * call scene->handleEvent() after detaching.
   */
  void detach() {
    if (!attached) {
      return;
    }

    attached = false;
  }

  // === Window Management ===

  /**
   * @brief Handle window resize events
   * @param width New window width
   * @param height New window height
   *
   * Automatically updates scene bounds and notifies renderer.
   * Override this to implement custom resize behavior.
   */
  virtual void onWindowResize(int width, int height) {
    // Update scene bounds
    setBounds(0, 0, width, height);

    // Invalidate layout to trigger re-layout on next render
    invalidateLayout();

    // Notify renderer of resize
    if (renderer) {
      renderer->onWindowResize(width, height);
    }

    // Notify children of resize (they can override setBounds if needed)
    notifyChildrenOfResize(width, height);
  }

  // === Component Overrides ===

  /**
   * @brief Update scene and all children
   * @param deltaTime Time since last update in seconds
   */
  void update(double deltaTime) override {
    (void)deltaTime; // Unused - scene has no internal state to update
    // Scene itself has no update logic, just propagate to children
    // (updateAll() will be called externally, which calls this then children)
  }

  /**
   * @brief Render scene and all children
   *
   * Scene itself has no visual representation, just propagate to children.
   * (renderAll() will be called externally, which calls this then children)
   */
  void render() override {
    // Scene itself renders nothing, children will be rendered by renderAll()
  }

  /**
   * @brief Present rendered frame to screen
   *
   * This finalizes rendering and swaps buffers.
   */
  void present() {
    if (renderer) {
      renderer->present();
    }
  }

  // === Child Management ===
  // Note: Scene now uses the default Component::addChild and Component::removeChild
  // No special event dispatcher registration needed with hierarchical event handling

  // === Accessors ===

  /**
   * @brief Get the window interface
   * @return Window interface pointer
   */
  events::IWindow* getWindow() const { return window; }

  /**
   * @brief Check if scene is attached to window
   * @return true if scene is attached
   */
  bool isAttached() const { return attached; }

private:
  /**
   * @brief Notify children of window resize
   *
   * Children can override setBounds() to implement custom resize behavior.
   *
   * @param width New window width
   * @param height New window height
   */
  void notifyChildrenOfResize(int width, int height) {
    (void)width;  // Available for child components to use
    (void)height; // Available for child components to use

    // Children can override setBounds() to implement custom resize behavior.
    // By default, children maintain their current position and size.
    // For automatic layout, use Layout managers or override this method.
  }
};

} // namespace bombfork::prong
