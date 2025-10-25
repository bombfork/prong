#include "bombfork/prong/core/component.h"
#include "bombfork/prong/rendering/irenderer.h"
#include "bombfork/prong/theming/color.h"

#include <bombfork/prong/generic/dialog.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bombfork::prong {

Dialog::Dialog() : Component(nullptr, "Dialog") {
  // Initialize default theme
  theme = DialogTheme();

  // Set initial state
  state.visible = false;
  state.type = DialogType::MODAL;
}

void Dialog::show() {
  state.visible = true;
  setVisible(true);
}

void Dialog::hide() {
  state.visible = false;
  setVisible(false);
}

void Dialog::setPosition(int newX, int newY) {
  Component::setPosition(newX, newY);
}

void Dialog::setDialogType(DialogType type) {
  state.type = type;
}

void Dialog::setTitle(const std::string& title) {
  state.title = title;
}

void Dialog::setStandardButtons(StandardButtons buttons) {
  standardButtons = buttons;
  // Button creation would happen here in full implementation
}

void Dialog::setButtonCallback(ButtonCallback callback) {
  buttonCallback = std::move(callback);
}

void Dialog::addContentComponent(std::unique_ptr<Component> component) {
  if (component) {
    addChild(std::move(component));
  }
}

void Dialog::render() {
  if (!isVisible() || !renderer)
    return;

  int gx = getGlobalX();
  int gy = getGlobalY();

  // Render dialog background (using drawRect - full implementation would use filled rect)
  renderer->drawRect(gx, gy, width, height, theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b,
                     theme.backgroundColor.a);

  // Render border
  renderer->drawRect(gx, gy, width, height, theme.borderColor.r, theme.borderColor.g, theme.borderColor.b,
                     theme.borderColor.a);

  // Render title bar if enabled
  if (state.showTitleBar && !state.title.empty()) {
    renderer->drawRect(gx, gy, width, TITLE_BAR_HEIGHT, theme.titleBarColor.r, theme.titleBarColor.g,
                       theme.titleBarColor.b, theme.titleBarColor.a);

    // Measure text for proper vertical centering
    auto [textWidth, textHeight] = renderer->measureText(state.title);
    renderer->drawText(state.title, gx + CONTENT_PADDING, gy + (TITLE_BAR_HEIGHT - textHeight) / 2,
                       theme.titleTextColor.r, theme.titleTextColor.g, theme.titleTextColor.b, theme.titleTextColor.a);
  }

  // Render children
  for (auto& child : getChildren()) {
    if (child && child->isVisible()) {
      child->render();
    }
  }
}

void Dialog::setBounds(int newX, int newY, int newWidth, int newHeight) {
  Component::setBounds(newX, newY, newWidth, newHeight);
}

bool Dialog::handleClick(int localX, int localY) {
  // Delegate to children
  for (auto& child : getChildren()) {
    if (!child || !child->isVisible())
      continue;

    // Get child's local position and size
    int childX, childY, childWidth, childHeight;
    child->getBounds(childX, childY, childWidth, childHeight);

    // Check if point is within child's local bounds
    if (localX >= childX && localX < childX + childWidth && localY >= childY && localY < childY + childHeight) {
      // Convert to child-local coordinates
      int childLocalX = localX - childX;
      int childLocalY = localY - childY;
      if (child->handleClick(childLocalX, childLocalY)) {
        return true;
      }
    }
  }
  return true; // Dialog consumes clicks to prevent pass-through
}

bool Dialog::handleMousePress(int localX, int localY, int button) {
  // Check if clicking in title bar for dragging
  if (state.showTitleBar && localY < TITLE_BAR_HEIGHT) {
    state.dragging = true;
    state.dragStartX = localX;
    state.dragStartY = localY;
    state.dragOffsetX = getGlobalX();
    state.dragOffsetY = getGlobalY();
    return true;
  }

  // Delegate to children
  for (auto& child : getChildren()) {
    if (!child || !child->isVisible())
      continue;

    // Get child's local position and size
    int childX, childY, childWidth, childHeight;
    child->getBounds(childX, childY, childWidth, childHeight);

    // Check if point is within child's local bounds
    if (localX >= childX && localX < childX + childWidth && localY >= childY && localY < childY + childHeight) {
      // Convert to child-local coordinates
      int childLocalX = localX - childX;
      int childLocalY = localY - childY;
      if (child->handleMousePress(childLocalX, childLocalY, button)) {
        return true;
      }
    }
  }
  return true;
}

bool Dialog::handleMouseRelease(int localX, int localY, int button) {
  if (state.dragging) {
    state.dragging = false;
    return true;
  }

  // Delegate to children
  for (auto& child : getChildren()) {
    if (!child || !child->isVisible())
      continue;

    // Get child's local position and size
    int childX, childY, childWidth, childHeight;
    child->getBounds(childX, childY, childWidth, childHeight);

    // Check if point is within child's local bounds
    if (localX >= childX && localX < childX + childWidth && localY >= childY && localY < childY + childHeight) {
      // Convert to child-local coordinates
      int childLocalX = localX - childX;
      int childLocalY = localY - childY;
      if (child->handleMouseRelease(childLocalX, childLocalY, button)) {
        return true;
      }
    }
  }
  return true;
}

bool Dialog::handleMouseMove(int localX, int localY) {
  if (state.dragging) {
    int deltaX = localX - state.dragStartX;
    int deltaY = localY - state.dragStartY;
    setPosition(state.dragOffsetX + deltaX, state.dragOffsetY + deltaY);
    return true;
  }

  // Delegate to children
  for (auto& child : getChildren()) {
    if (!child || !child->isVisible())
      continue;

    // Get child's local position and size
    int childX, childY, childWidth, childHeight;
    child->getBounds(childX, childY, childWidth, childHeight);

    // Check if point is within child's local bounds
    if (localX >= childX && localX < childX + childWidth && localY >= childY && localY < childY + childHeight) {
      // Convert to child-local coordinates
      int childLocalX = localX - childX;
      int childLocalY = localY - childY;
      if (child->handleMouseMove(childLocalX, childLocalY)) {
        return true;
      }
    }
  }
  return false;
}

bool Dialog::handleKey(int key, int action, int mods) {
  // Escape key closes dialog
  if (key == 256 && action == 1) { // ESC key press (GLFW_KEY_ESCAPE)
    hide();
    if (dialogCallback) {
      dialogCallback(DialogResult::CANCEL);
    }
    return true;
  }

  // Delegate to children
  for (auto& child : getChildren()) {
    if (child && child->isVisible()) {
      if (child->handleKey(key, action, mods)) {
        return true;
      }
    }
  }
  return false;
}

} // namespace bombfork::prong
