#pragma once

#include <bombfork/prong/core/component.h>
#include <bombfork/prong/events/event_dispatcher.h>
#include <bombfork/prong/events/iwindow.h>
#include <bombfork/prong/rendering/irenderer.h>

#include <memory>

namespace bombfork::prong {

/**
 * @brief Root scene component that manages the entire UI hierarchy
 *
 * Scene is the top-level container for all UI components. It owns the
 * window and renderer references, manages the event dispatcher, and
 * automatically handles window resizing.
 *
 * Key responsibilities:
 * - Owns and manages EventDispatcher for input routing
 * - Automatically fills window dimensions
 * - Handles window resize events and propagates to children
 * - Provides simplified update/render/present interface
 * - Registers all children with the event dispatcher
 *
 * Usage:
 * ```cpp
 * IWindow* window = createWindow();
 * IRenderer* renderer = createRenderer();
 * Scene scene(window, renderer);
 * scene.attach();
 *
 * // Add UI components
 * auto panel = std::make_unique<Panel>(renderer);
 * scene.addChild(std::move(panel));
 *
 * // Main loop
 * while (!window->shouldClose()) {
 *   scene.update(deltaTime);
 *   scene.render();
 *   scene.present();
 * }
 *
 * scene.detach();
 * ```
 */
class Scene : public Component {
private:
  events::IWindow* window = nullptr;
  std::unique_ptr<events::EventDispatcher> eventDispatcher;
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

    // Create event dispatcher
    eventDispatcher = std::make_unique<events::EventDispatcher>(window);

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
   * This registers the scene and all children with the event dispatcher
   * and sets up window resize callbacks.
   */
  void attach() {
    if (attached) {
      return;
    }

    // Ensure window size is current
    int windowWidth, windowHeight;
    window->getSize(windowWidth, windowHeight);
    setBounds(0, 0, windowWidth, windowHeight);

    // Register resize callback
    eventDispatcher->setWindowResizeCallback([this](int width, int height) { onWindowResize(width, height); });

    // Register scene with event dispatcher
    eventDispatcher->registerComponent(this);

    // Register all children with event dispatcher
    registerChildrenWithDispatcher();

    attached = true;
  }

  /**
   * @brief Detach scene from window and stop event handling
   *
   * This unregisters the scene and all children from the event dispatcher.
   */
  void detach() {
    if (!attached) {
      return;
    }

    // Unregister scene and children from event dispatcher
    eventDispatcher->clearComponents();

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

    // Update event dispatcher window size
    eventDispatcher->setWindowSize(width, height);

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

  /**
   * @brief Add child component to scene
   *
   * Overrides Component::addChild to automatically register the child
   * with the event dispatcher if the scene is attached.
   *
   * @param child Child component to add (ownership transferred)
   */
  void addChild(std::unique_ptr<Component> child) {
    if (child) {
      // Register with event dispatcher if attached
      if (attached) {
        eventDispatcher->registerComponent(child.get());
      }

      // Add to component hierarchy
      Component::addChild(std::move(child));
    }
  }

  /**
   * @brief Remove child component from scene
   *
   * Overrides Component::removeChild to automatically unregister the child
   * from the event dispatcher.
   *
   * @param child Child component to remove
   * @return true if child was found and removed
   */
  bool removeChild(Component* child) {
    if (child) {
      // Unregister from event dispatcher
      if (attached) {
        eventDispatcher->unregisterComponent(child);
      }

      // Remove from component hierarchy
      return Component::removeChild(child);
    }
    return false;
  }

  // === Accessors ===

  /**
   * @brief Get the window interface
   * @return Window interface pointer
   */
  events::IWindow* getWindow() const { return window; }

  /**
   * @brief Get the event dispatcher
   * @return Event dispatcher pointer
   */
  events::EventDispatcher* getEventDispatcher() const { return eventDispatcher.get(); }

  /**
   * @brief Check if scene is attached to window
   * @return true if scene is attached
   */
  bool isAttached() const { return attached; }

private:
  /**
   * @brief Register all children with event dispatcher
   *
   * Recursively registers all child components for event handling.
   */
  void registerChildrenWithDispatcher() {
    for (auto& child : children) {
      if (child) {
        eventDispatcher->registerComponent(child.get());
        // Note: We only register direct children, not all descendants.
        // The EventDispatcher will handle event routing to nested children
        // through the Component::handleXXX methods.
      }
    }
  }

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
