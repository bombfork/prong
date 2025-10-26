#include "bombfork/prong/core/component.h"
#include "bombfork/prong/events/iwindow.h"

#include <bombfork/prong/events/event_dispatcher.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace bombfork::prong::events {

EventDispatcher::EventDispatcher(IWindow* window) : window(window) {
  if (window) {
    registerCallbacks();
  }
}

EventDispatcher::~EventDispatcher() {
  if (window) {
    unregisterCallbacks();
  }
}

void EventDispatcher::setWindow(IWindow* window) {
  if (this->window) {
    unregisterCallbacks();
  }

  this->window = window;

  if (window) {
    registerCallbacks();
    // Update window dimensions
    int width, height;
    window->getSize(width, height);
    setWindowSize(width, height);
  }
}

void EventDispatcher::setWindowSize(int width, int height) {
  windowWidth = width;
  windowHeight = height;
}

void EventDispatcher::setWindowResizeCallback(std::function<void(int, int)> callback) {
  windowResizeCallback = callback;
}

void EventDispatcher::setWorkingAreaMode(WorkingAreaMode mode) {
  currentMode = mode;
}

void EventDispatcher::registerComponent(bombfork::prong::Component* component) {
  if (component && std::find(components.begin(), components.end(), component) == components.end()) {
    components.push_back(component);
  }
}

void EventDispatcher::unregisterComponent(bombfork::prong::Component* component) {
  auto it = std::find(components.begin(), components.end(), component);
  if (it != components.end()) {
    if (hoveredComponent == component) {
      hoveredComponent = nullptr;
    }
    if (focusedComponent == component) {
      focusedComponent = nullptr;
    }
    if (draggedComponent == component) {
      draggedComponent = nullptr;
    }
    components.erase(it);
  }
}

void EventDispatcher::clearComponents() {
  components.clear();
  hoveredComponent = nullptr;
  focusedComponent = nullptr;
  draggedComponent = nullptr;
}

bool EventDispatcher::setFocus(bombfork::prong::Component* component) {
  if (component && component->canReceiveFocus()) {
    if (focusedComponent) {
      focusedComponent->setFocusState(bombfork::prong::Component::FocusState::NONE);
    }

    focusedComponent = component;
    component->setFocusState(bombfork::prong::Component::FocusState::FOCUSED);
    return true;
  }
  return false;
}

void EventDispatcher::clearFocus() {
  if (focusedComponent) {
    focusedComponent->setFocusState(bombfork::prong::Component::FocusState::NONE);
    focusedComponent = nullptr;
  }
}

void EventDispatcher::getMousePosition(int& x, int& y) const {
  x = mouseState.currentX;
  y = mouseState.currentY;
}

void EventDispatcher::processMouseButton(int button, int action, int mods) {
  (void)mods; // Unused parameter

  if (button == MOUSE_BUTTON_LEFT) {
    if (action == INPUT_PRESS) {
      mouseState.isDragging = false;
      mouseState.dragStartX = mouseState.currentX;
      mouseState.dragStartY = mouseState.currentY;
      mouseState.pressedButton = button;

      // Find component at current position and try to handle press
      bombfork::prong::Component* component = findComponentAt(mouseState.currentX, mouseState.currentY);

      if (component) {
        // Convert to component-local coordinates
        int localX, localY;
        component->globalToLocal(mouseState.currentX, mouseState.currentY, localX, localY);

        if (component->handleMousePress(localX, localY, button)) {
          // Set focus to the component that handled the press
          setFocus(component);
          // Track this component for dragging
          draggedComponent = component;
          return;
        }
      }
    } else if (action == INPUT_RELEASE) {
      mouseState.isDragging = false;
      mouseState.pressedButton = -1;

      // Always send release to dragged component first, regardless of current position
      // This ensures proper cleanup even if mouse was released outside the component
      if (draggedComponent) {
        int localX, localY;
        draggedComponent->globalToLocal(mouseState.currentX, mouseState.currentY, localX, localY);

        // Always send release event to the dragged component for proper cleanup
        draggedComponent->handleMouseRelease(localX, localY, button);

        // If release is over the same component, also trigger click (before clearing
        // draggedComponent)
        bombfork::prong::Component* currentComponent = findComponentAt(mouseState.currentX, mouseState.currentY);
        if (currentComponent && currentComponent == draggedComponent) {
          currentComponent->handleClick(localX, localY);
        }

        draggedComponent = nullptr; // Clear drag tracking after release
        return;
      }

      // Fallback to component at current position (for components that weren't being dragged)
      bombfork::prong::Component* component = findComponentAt(mouseState.currentX, mouseState.currentY);
      if (component) {
        int localX, localY;
        component->globalToLocal(mouseState.currentX, mouseState.currentY, localX, localY);

        if (component->handleMouseRelease(localX, localY, button)) {
          // Also trigger click if release is on same component
          component->handleClick(localX, localY);
          return;
        }
      }
    }
  }
}

void EventDispatcher::processMouseMove(double xpos, double ypos) {
  int mouseX = static_cast<int>(xpos);
  int mouseY = static_cast<int>(ypos);

  mouseState.currentX = mouseX;
  mouseState.currentY = mouseY;

  // Update hover state
  updateMouseHover(mouseX, mouseY);

  // Update drag state
  updateDragState(mouseX, mouseY);

  // Send move events to components
  // If we have a dragged component, send move events to it regardless of mouse position
  if (draggedComponent) {
    int localX, localY;
    draggedComponent->globalToLocal(mouseX, mouseY, localX, localY);
    draggedComponent->handleMouseMove(localX, localY);
  } else {
    // Normal move event handling
    bombfork::prong::Component* component = findComponentAt(mouseX, mouseY);
    if (component) {
      int localX, localY;
      component->globalToLocal(mouseX, mouseY, localX, localY);
      component->handleMouseMove(localX, localY);
    }
  }
}

void EventDispatcher::processScroll(double xoffset, double yoffset) {
  // Use complex area-specific scroll logic from original
  handleAreaSpecificScroll(xoffset, yoffset);
}

void EventDispatcher::processKey(int key, int scancode, int action, int mods) {
  (void)scancode; // Unused parameter
  // First try focused component
  if (focusedComponent && focusedComponent->isEnabled() && focusedComponent->isVisible()) {
    if (focusedComponent->handleKey(key, action, mods)) {
      return;
    }
  }

  // Then try all components (reverse order for proper layering)
  for (auto it = components.rbegin(); it != components.rend(); ++it) {
    bombfork::prong::Component* component = *it;
    if (component->isEnabled() && component->isVisible()) {
      if (component->handleKey(key, action, mods)) {
        return;
      }
    }
  }
}

void EventDispatcher::processChar(unsigned int codepoint) {
  // First try focused component
  if (focusedComponent && focusedComponent->isEnabled() && focusedComponent->isVisible()) {
    if (focusedComponent->handleChar(codepoint)) {
      return;
    }
  }

  // Then try all components (reverse order for proper layering)
  for (auto it = components.rbegin(); it != components.rend(); ++it) {
    bombfork::prong::Component* component = *it;
    if (component->isEnabled() && component->isVisible()) {
      if (component->handleChar(codepoint)) {
        return;
      }
    }
  }
}

void EventDispatcher::processFramebufferResize(int width, int height) {
  setWindowSize(width, height);

  // Notify MainWindow of resize so it can update layout
  if (windowResizeCallback) {
    windowResizeCallback(width, height);
  }
}

// === Internal Event Processing ===

bombfork::prong::Component* EventDispatcher::findComponentAt(int x, int y) {
  // Check registered components in reverse order (last rendered = topmost = first to handle events)
  // IMPORTANT: Only check ROOT components (those without parents) to avoid duplicate checking.
  // Child components will be checked recursively through their parents.
  for (auto it = components.rbegin(); it != components.rend(); ++it) {
    bombfork::prong::Component* component = *it;

    // Skip if this component has a parent - it will be checked through the parent hierarchy
    if (component->getParent() != nullptr) {
      continue;
    }

    // Recursively check component and its children
    bombfork::prong::Component* found = findComponentAtRecursive(component, x, y);
    if (found) {
      return found;
    }
  }
  return nullptr;
}

bombfork::prong::Component* EventDispatcher::findComponentAtRecursive(bombfork::prong::Component* component, int x,
                                                                      int y) {
  if (!component || !component->isVisible() || !component->isEnabled()) {
    return nullptr;
  }

  // First, check if the point is even within this component's bounds
  if (!component->containsGlobal(x, y)) {
    return nullptr;
  }

  // Check children in reverse order (last rendered = topmost)
  // Children are checked before the parent to ensure proper event capture
  const auto& children = component->getChildren();
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    bombfork::prong::Component* child = it->get();
    bombfork::prong::Component* found = findComponentAtRecursive(child, x, y);
    if (found) {
      return found;
    }
  }

  // If no children matched, return this component (since we already checked it contains the point)
  return component;
}

void EventDispatcher::updateMouseHover(int x, int y) {
  bombfork::prong::Component* newHovered = findComponentAt(x, y);

  if (newHovered != hoveredComponent) {
    // Mouse left previous component
    if (hoveredComponent) {
      hoveredComponent->handleMouseLeave();
      hoveredComponent->setFocusState(bombfork::prong::Component::FocusState::NONE);
    }

    // Mouse entered new component
    hoveredComponent = newHovered;
    if (hoveredComponent) {
      hoveredComponent->handleMouseEnter();
      if (hoveredComponent->getFocusState() == bombfork::prong::Component::FocusState::NONE) {
        hoveredComponent->setFocusState(bombfork::prong::Component::FocusState::HOVERING);
      }
    }
  }
}

void EventDispatcher::handleAreaSpecificScroll(double xoffset, double yoffset) {
  if (currentMode == WorkingAreaMode::SPRITE_EDITOR) {
    handleSpriteEditorScroll(xoffset, yoffset);
  } else if (currentMode == WorkingAreaMode::MAP_EDITOR) {
    handleMapEditorScroll(xoffset, yoffset);
  }
}

void EventDispatcher::handleSpriteEditorScroll(double xoffset, double yoffset) {
  // Replicate exact original sprite editor scroll logic
  int mouseX = mouseState.currentX;
  int mouseY = mouseState.currentY;

  // Calculate sprite display area boundaries (same logic as renderSpriteEditor)
  const int areaY = 60;                             // Menu height + padding
  const int areaHeight = windowHeight - areaY - 40; // Reserve space for status
  const int spriteListWidth = 300;                  // Updated from original 250
  const int sep1X = spriteListWidth + 10;
  const int separatorWidth = 2;
  const int spriteDisplayX = sep1X + separatorWidth + 10;
  const int spriteInfoWidth = 250;
  const int spriteDisplayWidth = windowWidth - spriteDisplayX - spriteInfoWidth - separatorWidth - 30;

  // Check if mouse is over sprite display area
  if (mouseX >= spriteDisplayX && mouseX <= spriteDisplayX + spriteDisplayWidth && mouseY >= areaY &&
      mouseY <= areaY + areaHeight) {

    // Handle sprite zoom - delegate to sprite viewer component
    bombfork::prong::Component* spriteViewer = findComponentAt(mouseX, mouseY);
    if (spriteViewer) {
      int localX, localY;
      spriteViewer->globalToLocal(mouseX, mouseY, localX, localY);
      spriteViewer->handleScroll(localX, localY, xoffset, yoffset);
    }

  } else if (mouseX >= 0 && mouseX <= spriteListWidth + 10 && mouseY >= areaY && mouseY <= areaY + areaHeight) {

    // Handle sprite list scrolling only when mouse is over sprite list area
    bombfork::prong::Component* spriteList = findComponentAt(mouseX, mouseY);
    if (spriteList) {
      int localX, localY;
      spriteList->globalToLocal(mouseX, mouseY, localX, localY);
      spriteList->handleScroll(localX, localY, xoffset, yoffset);
    }
  }
  // If mouse is over sprite info area or other areas, do nothing (no scrolling)
}

void EventDispatcher::handleMapEditorScroll(double xoffset, double yoffset) {
  // Replicate exact original map editor scroll logic
  int mouseX = mouseState.currentX;
  int mouseY = mouseState.currentY;

  const int areaY = 60;
  const int controlPanelWidth = 200;
  const int mapViewX = controlPanelWidth + 10;
  const int mapViewWidth = windowWidth - mapViewX - 210;
  const int areaHeight = windowHeight - areaY - 40;

  // Check if mouse is over map view area
  if (mouseX >= mapViewX && mouseX <= mapViewX + mapViewWidth && mouseY >= areaY && mouseY <= areaY + areaHeight) {

    // Handle camera zoom - delegate to map viewport component
    bombfork::prong::Component* mapViewport = findComponentAt(mouseX, mouseY);
    if (mapViewport) {
      int localX, localY;
      mapViewport->globalToLocal(mouseX, mouseY, localX, localY);
      mapViewport->handleScroll(localX, localY, xoffset, yoffset);
    }
  }
}

bool EventDispatcher::isMouseOverSpriteDisplay(int mouseX, int mouseY) {
  const int areaY = 60;
  const int areaHeight = windowHeight - areaY - 40;
  const int spriteListWidth = 300;
  const int sep1X = spriteListWidth + 10;
  const int separatorWidth = 2;
  const int spriteDisplayX = sep1X + separatorWidth + 10;
  const int spriteInfoWidth = 250;
  const int spriteDisplayWidth = windowWidth - spriteDisplayX - spriteInfoWidth - separatorWidth - 30;

  return mouseX >= spriteDisplayX && mouseX <= spriteDisplayX + spriteDisplayWidth && mouseY >= areaY &&
         mouseY <= areaY + areaHeight;
}

bool EventDispatcher::isMouseOverSpriteList(int mouseX, int mouseY) {
  const int areaY = 60;
  const int areaHeight = windowHeight - areaY - 40;
  const int spriteListWidth = 300;

  return mouseX >= 0 && mouseX <= spriteListWidth + 10 && mouseY >= areaY && mouseY <= areaY + areaHeight;
}

bool EventDispatcher::isMouseOverMapView(int mouseX, int mouseY) {
  const int areaY = 60;
  const int controlPanelWidth = 200;
  const int mapViewX = controlPanelWidth + 10;
  const int mapViewWidth = windowWidth - mapViewX - 210;
  const int areaHeight = windowHeight - areaY - 40;

  return mouseX >= mapViewX && mouseX <= mapViewX + mapViewWidth && mouseY >= areaY && mouseY <= areaY + areaHeight;
}

void EventDispatcher::updateDragState(int mouseX, int mouseY) {
  if (mouseState.pressedButton >= 0) {
    if (!mouseState.isDragging) {
      // Start dragging if mouse moved enough
      int deltaX = mouseX - mouseState.dragStartX;
      int deltaY = mouseY - mouseState.dragStartY;
      if (abs(deltaX) > MouseState::DRAG_THRESHOLD || abs(deltaY) > MouseState::DRAG_THRESHOLD) {
        mouseState.isDragging = true;
      }
    }

    if (mouseState.isDragging) {
      // Update sprite offset for panning (used by sprite viewer)
      mouseState.spriteOffsetX = static_cast<float>(mouseX - mouseState.dragStartX);
      mouseState.spriteOffsetY = static_cast<float>(mouseY - mouseState.dragStartY);
    }
  }
}

void EventDispatcher::registerCallbacks() {
  if (!window) {
    std::cout << "EventDispatcher::registerCallbacks() - NO WINDOW!" << std::endl;
    return;
  }

  WindowCallbacks callbacks;

  callbacks.mouseButton = [this](int button, int action, int mods) { processMouseButton(button, action, mods); };

  callbacks.cursorPos = [this](double x, double y) { processMouseMove(x, y); };

  callbacks.scroll = [this](double xoffset, double yoffset) { processScroll(xoffset, yoffset); };

  callbacks.key = [this](int key, int scancode, int action, int mods) { processKey(key, scancode, action, mods); };

  callbacks.character = [this](unsigned int codepoint) { processChar(codepoint); };

  callbacks.framebufferSize = [this](int width, int height) { processFramebufferResize(width, height); };

  window->setCallbacks(callbacks);
}

void EventDispatcher::unregisterCallbacks() {
  if (!window)
    return;

  // Clear all callbacks by setting empty WindowCallbacks
  WindowCallbacks emptyCallbacks;
  window->setCallbacks(emptyCallbacks);
}

} // namespace bombfork::prong::events
