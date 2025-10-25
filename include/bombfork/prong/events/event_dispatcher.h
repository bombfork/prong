#pragma once

#include <bombfork/prong/core/component.h>
#include <bombfork/prong/events/iwindow.h>

#include <functional>
#include <memory>
#include <vector>

namespace bombfork::prong::events {

/**
 * @brief Complete event handling system with full GLFW integration
 *
 * Manages all input events and routes them to appropriate UI components
 * with complex area-specific logic and mode-aware handling.
 */
class EventDispatcher {
public:
  enum class WorkingAreaMode { SPRITE_EDITOR, MAP_EDITOR };

  struct MouseState {
    bool isDragging = false;
    int dragStartX = 0, dragStartY = 0;
    int currentX = 0, currentY = 0;
    int pressedButton = -1; // Which button was pressed (-1 = none)

    // Sprite editor specific
    float spriteOffsetX = 0.0f, spriteOffsetY = 0.0f; // Camera offset for sprite view

    // Thresholds
    static constexpr int DRAG_THRESHOLD = 3; // Pixels to move before dragging starts
  };

private:
  // GLFW window
  IWindow* window = nullptr;

  // Window dimensions
  int windowWidth = 1280;
  int windowHeight = 720;

  // Current mode
  WorkingAreaMode currentMode = WorkingAreaMode::SPRITE_EDITOR;

  // Mouse state
  MouseState mouseState;

  // Registered components (in rendering order - last rendered = first to receive events)
  std::vector<bombfork::prong::Component*> components;

  // Component that currently has mouse hover
  bombfork::prong::Component* hoveredComponent = nullptr;

  // Component that currently has keyboard focus
  bombfork::prong::Component* focusedComponent = nullptr;

  // Component that is currently being dragged
  bombfork::prong::Component* draggedComponent = nullptr;

  // Resize callback
  std::function<void(int, int)> windowResizeCallback = nullptr;

public:
  EventDispatcher(IWindow* window);
  ~EventDispatcher();

  // === Window Management ===

  /**
   * @brief Set window and register GLFW callbacks
   */
  void setWindow(IWindow* window);

  /**
   * @brief Update window dimensions
   */
  void setWindowSize(int width, int height);

  /**
   * @brief Get window dimensions
   */
  void getWindowSize(int& width, int& height) const {
    width = windowWidth;
    height = windowHeight;
  }

  // === Mode Management ===

  /**
   * @brief Set current working area mode
   */
  void setWorkingAreaMode(WorkingAreaMode mode);

  /**
   * @brief Get current working area mode
   */
  WorkingAreaMode getWorkingAreaMode() const { return currentMode; }

  // === Component Registration ===

  /**
   * @brief Register a UI component for event handling
   */
  void registerComponent(bombfork::prong::Component* component);

  /**
   * @brief Unregister a UI component
   */
  void unregisterComponent(bombfork::prong::Component* component);

  /**
   * @brief Clear all registered components
   */
  void clearComponents();

  // === Focus Management ===

  /**
   * @brief Set keyboard focus to a component
   */
  bool setFocus(bombfork::prong::Component* component);

  /**
   * @brief Clear keyboard focus
   */
  void clearFocus();

  /**
   * @brief Get currently focused component
   */
  bombfork::prong::Component* getFocusedComponent() const { return focusedComponent; }

  // === Mouse State Access ===

  /**
   * @brief Get current mouse state
   */
  const MouseState& getMouseState() const { return mouseState; }

  /**
   * @brief Get current mouse position
   */
  void getMousePosition(int& x, int& y) const;

  // === Resize Callback Support ===

  /**
   * @brief Set callback for window resize events
   * @param callback Function to call when window is resized
   */
  void setWindowResizeCallback(std::function<void(int, int)> callback);

  // === Event Processing (called by GLFW callbacks) ===

  /**
   * @brief Process mouse button events with full original logic
   */
  void processMouseButton(int button, int action, int mods);

  /**
   * @brief Process mouse movement events
   */
  void processMouseMove(double xpos, double ypos);

  /**
   * @brief Process scroll events with area-specific logic
   */
  void processScroll(double xoffset, double yoffset);

  /**
   * @brief Process keyboard events
   */
  void processKey(int key, int scancode, int action, int mods);

  /**
   * @brief Process character input events
   */
  void processChar(unsigned int codepoint);

  /**
   * @brief Process framebuffer resize events
   */
  void processFramebufferResize(int width, int height);

private:
  // === Internal Event Processing ===

  /**
   * @brief Find component at screen coordinates (topmost first)
   */
  bombfork::prong::Component* findComponentAt(int x, int y);

  /**
   * @brief Recursively search a component and its children for the topmost component at coordinates
   * @param component The component to search
   * @param x Global X coordinate
   * @param y Global Y coordinate
   * @return The topmost component at the given coordinates, or nullptr if none found
   */
  bombfork::prong::Component* findComponentAtRecursive(bombfork::prong::Component* component, int x, int y);

  /**
   * @brief Update mouse hover state
   */
  void updateMouseHover(int x, int y);

  /**
   * @brief Handle complex scroll logic based on area and mode
   */
  void handleAreaSpecificScroll(double xoffset, double yoffset);

  /**
   * @brief Handle sprite editor specific scroll logic
   */
  void handleSpriteEditorScroll(double xoffset, double yoffset);

  /**
   * @brief Handle map editor specific scroll logic
   */
  void handleMapEditorScroll(double xoffset, double yoffset);

  /**
   * @brief Check if mouse is over sprite display area (for zoom)
   */
  bool isMouseOverSpriteDisplay(int mouseX, int mouseY);

  /**
   * @brief Check if mouse is over sprite list area (for scrolling)
   */
  bool isMouseOverSpriteList(int mouseX, int mouseY);

  /**
   * @brief Check if mouse is over map view area
   */
  bool isMouseOverMapView(int mouseX, int mouseY);

  /**
   * @brief Update drag state based on mouse movement
   */
  void updateDragState(int mouseX, int mouseY);

  /**
   * @brief Register all GLFW callbacks
   */
  void registerCallbacks();

  /**
   * @brief Unregister GLFW callbacks
   */
  void unregisterCallbacks();
};

} // namespace bombfork::prong::events