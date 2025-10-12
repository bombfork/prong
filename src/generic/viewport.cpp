#include "bombfork/prong/core/component.h"
#include "bombfork/prong/rendering/irenderer.h"
#include "bombfork/prong/theming/color.h"

#include <bombfork/prong/generic/viewport.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>

namespace bombfork::prong {

Viewport::Viewport() : Component(nullptr, "Viewport") {
  // Initialize default theme
  theme = ViewportTheme();

  // Initialize transform
  transform.zoomLevel = 1.0f;
  transform.panX = 0.0f;
  transform.panY = 0.0f;
  transform.targetZoom = 1.0f;
  transform.targetPanX = 0.0f;
  transform.targetPanY = 0.0f;
  transform.animating = false;
}

void Viewport::setMode(ViewportMode mode) {
  state.mode = mode;
}

void Viewport::setZoomMode(ZoomMode mode) {
  state.zoomMode = mode;
}

void Viewport::setPanMode(PanMode mode) {
  state.panMode = mode;
}

void Viewport::setContentSize(int contentWidth, int contentHeight) {
  state.contentWidth = contentWidth;
  state.contentHeight = contentHeight;
}

void Viewport::setZoom(float zoomLevel, bool animate) {
  if (animate) {
    transform.targetZoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, zoomLevel));
    transform.animating = true;
    transform.animationStartTime = std::chrono::steady_clock::now();
  } else {
    transform.zoomLevel = std::max(MIN_ZOOM, std::min(MAX_ZOOM, zoomLevel));
    transform.targetZoom = transform.zoomLevel;
  }

  if (zoomCallback) {
    zoomCallback(transform.zoomLevel);
  }
}

void Viewport::setPan(float panX, float panY, bool animate) {
  if (animate) {
    transform.targetPanX = panX;
    transform.targetPanY = panY;
    transform.animating = true;
    transform.animationStartTime = std::chrono::steady_clock::now();
  } else {
    transform.panX = panX;
    transform.panY = panY;
    transform.targetPanX = panX;
    transform.targetPanY = panY;
  }

  if (panCallback) {
    panCallback(transform.panX, transform.panY);
  }
}

void Viewport::setShowGrid(bool show) {
  state.showGrid = show;
}

void Viewport::setShowRulers(bool show) {
  state.showRulers = show;
}

void Viewport::setShowScrollbars(bool show) {
  state.showScrollbars = show;
}

void Viewport::setShowFPS(bool show) {
  state.showFPS = show;
}

void Viewport::setViewportTheme(const ViewportTheme& newTheme) {
  theme = newTheme;
}

void Viewport::setRenderCallback(RenderCallback callback) {
  renderCallback = std::move(callback);
}

void Viewport::setZoomCallback(ZoomChangedCallback callback) {
  zoomCallback = std::move(callback);
}

void Viewport::setPanCallback(PanChangedCallback callback) {
  panCallback = std::move(callback);
}

void Viewport::render() {
  if (!isVisible() || !renderer)
    return;

  // Update animation if active
  if (transform.animating) {
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - transform.animationStartTime).count();
    float t = std::min(1.0f, elapsed / transform.animationDuration);

    // Smooth interpolation
    t = t * t * (3.0f - 2.0f * t);

    transform.zoomLevel = transform.zoomLevel + (transform.targetZoom - transform.zoomLevel) * t;
    transform.panX = transform.panX + (transform.targetPanX - transform.panX) * t;
    transform.panY = transform.panY + (transform.targetPanY - transform.panY) * t;

    if (t >= 1.0f) {
      transform.animating = false;
    }
  }

  // Render background (using drawRect - full implementation would use filled rect)
  renderer->drawRect(x, y, width, height, theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b,
                     theme.backgroundColor.a);

  // Render border
  if (theme.borderWidth > 0.0f) {
    renderer->drawRect(x, y, width, height, theme.borderColor.r, theme.borderColor.g, theme.borderColor.b,
                       theme.borderColor.a);
  }

  // Call custom render callback if provided
  if (renderCallback) {
    renderCallback(renderer, transform, width, height);
  }

  // Render grid if enabled
  if (state.showGrid) {
    renderGrid();
  }

  // Render rulers if enabled
  if (state.showRulers) {
    renderRulers();
  }

  // Render scrollbars if enabled
  if (state.showScrollbars) {
    renderScrollbars();
  }

  // Render FPS if enabled
  if (state.showFPS) {
    renderFPS();
  }
}

void Viewport::setBounds(int newX, int newY, int newWidth, int newHeight) {
  x = newX;
  y = newY;
  width = newWidth;
  height = newHeight;
}

bool Viewport::handleClick(int localX, int localY) {
  // Default implementation - can be overridden by derived classes
  return Component::handleClick(localX, localY);
}

bool Viewport::handleMousePress(int localX, int localY, int button) {
  // Handle viewport panning/dragging
  if (button == 0) { // Left mouse button
    state.dragging = true;
    state.dragStartX = localX;
    state.dragStartY = localY;
    state.dragStartPanX = transform.panX;
    state.dragStartPanY = transform.panY;
    return true;
  }

  return Component::handleMousePress(localX, localY, button);
}

bool Viewport::handleMouseRelease(int localX, int localY, int button) {
  if (button == 0 && state.dragging) {
    state.dragging = false;
    return true;
  }

  return Component::handleMouseRelease(localX, localY, button);
}

bool Viewport::handleMouseMove(int localX, int localY) {
  if (state.dragging) {
    float deltaX = static_cast<float>(localX - state.dragStartX);
    float deltaY = static_cast<float>(localY - state.dragStartY);

    setPan(state.dragStartPanX + deltaX, state.dragStartPanY + deltaY, false);

    return true;
  }

  return Component::handleMouseMove(localX, localY);
}

bool Viewport::handleScroll(int localX, int localY, double xoffset, double yoffset) {
  (void)localX;  // Unused parameter
  (void)localY;  // Unused parameter
  (void)xoffset; // Unused parameter
  // Handle zoom with mouse wheel
  float zoomFactor = 1.0f + static_cast<float>(yoffset) * 0.1f;
  setZoom(transform.zoomLevel * zoomFactor, false);
  return true;
}

bool Viewport::handleKey(int key, int action, int mods) {
  // Default implementation - can be overridden by derived classes
  return Component::handleKey(key, action, mods);
}

void Viewport::renderGrid() {
  // Stub implementation - would render grid overlay
  // Full implementation would draw grid lines based on zoom and pan
}

void Viewport::renderRulers() {
  // Stub implementation - would render ruler overlays
  // Full implementation would draw rulers with measurements
}

void Viewport::renderScrollbars() {
  // Stub implementation - would render scrollbars
  // Full implementation would draw proportional scrollbars
}

void Viewport::renderFPS() {
  // Stub implementation - would render FPS counter
  // Full implementation would track and display frame rate
}

} // namespace bombfork::prong
