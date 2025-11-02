#pragma once

#include "../core/event.h"
#include "../layout/layout_manager.h"
#include "../layout/layout_measurement.h"
#include "../theming/advanced_theme.h"
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/rendering/irenderer.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <utility>

namespace bombfork::prong {

/**
 * @brief Professional viewport component for hardware-accelerated rendering
 *
 * Provides comprehensive viewport functionality for OpenGL/Vulkan rendering with:
 * - Hardware-accelerated sprite and map rendering
 * - Zoom and pan controls with smooth animations
 * - Grid overlay and ruler display
 * - Selection rectangle and multi-selection support
 * - Context menu integration for viewport actions
 * - Professional scroll bars with proportional sizing
 * - Frame rate control and performance monitoring
 * - Coordinate transformation and hit testing
 * - Background pattern and texture support
 * - Professional theming integration
 */
class Viewport : public Component {
public:
  enum class ViewportMode {
    SPRITE_VIEWER,  // Sprite display and editing
    MAP_VIEWER,     // Map display and editing
    TEXTURE_VIEWER, // Generic texture viewing
    CUSTOM          // Custom rendering content
  };

  enum class ZoomMode {
    FIT_TO_WINDOW, // Scale content to fit viewport
    ACTUAL_SIZE,   // Show content at 1:1 pixel ratio
    CUSTOM_SCALE   // Custom zoom level
  };

  enum class PanMode {
    FREE,            // Pan in any direction
    HORIZONTAL_ONLY, // Pan horizontally only
    VERTICAL_ONLY    // Pan vertically only
  };

  struct ViewportTransform {
    float zoomLevel = 1.0f;  // Current zoom level (1.0 = 100%)
    float panX = 0.0f;       // Pan offset X
    float panY = 0.0f;       // Pan offset Y
    float targetZoom = 1.0f; // Target zoom for animation
    float targetPanX = 0.0f; // Target pan X for animation
    float targetPanY = 0.0f; // Target pan Y for animation
    bool animating = false;  // Is transform animating
    std::chrono::steady_clock::time_point animationStartTime;
    float animationDuration = 0.3f; // seconds
  };

  using RenderCallback = std::function<void(bombfork::prong::rendering::IRenderer* renderer,
                                            const ViewportTransform& transform, int viewportWidth, int viewportHeight)>;
  using ZoomChangedCallback = std::function<void(float zoomLevel)>;
  using PanChangedCallback = std::function<void(float panX, float panY)>;
  using SelectionCallback = std::function<void(int x, int y, int width, int height)>;

  struct ViewportTheme {
    // Background colors
    bombfork::prong::theming::Color backgroundColor;
    bombfork::prong::theming::Color checkerboardColor1;
    bombfork::prong::theming::Color checkerboardColor2;

    // Grid and rulers
    bombfork::prong::theming::Color gridColor;
    bombfork::prong::theming::Color rulerColor;
    bombfork::prong::theming::Color rulerTextColor;

    // Selection
    bombfork::prong::theming::Color selectionColor;
    bombfork::prong::theming::Color selectionBorderColor;

    // Scrollbars
    bombfork::prong::theming::Color scrollbarTrackColor;
    bombfork::prong::theming::Color scrollbarThumbColor;
    bombfork::prong::theming::Color scrollbarThumbHoverColor;

    // Border
    bombfork::prong::theming::Color borderColor;
    float borderWidth = 1.0f;

    ViewportTheme() {
      // Professional desktop theme defaults
      backgroundColor = bombfork::prong::theming::Color(0.12f, 0.12f, 0.12f, 1.0f);
      checkerboardColor1 = bombfork::prong::theming::Color(0.15f, 0.15f, 0.15f, 1.0f);
      checkerboardColor2 = bombfork::prong::theming::Color(0.18f, 0.18f, 0.18f, 1.0f);

      gridColor = bombfork::prong::theming::Color(0.3f, 0.3f, 0.3f, 0.5f);
      rulerColor = bombfork::prong::theming::Color(0.25f, 0.25f, 0.25f, 1.0f);
      rulerTextColor = bombfork::prong::theming::Color(0.8f, 0.8f, 0.8f, 1.0f);

      selectionColor = bombfork::prong::theming::Color(0.2f, 0.4f, 0.8f, 0.3f);
      selectionBorderColor = bombfork::prong::theming::Color(0.3f, 0.5f, 0.9f, 0.8f);

      scrollbarTrackColor = bombfork::prong::theming::Color(0.2f, 0.2f, 0.2f, 1.0f);
      scrollbarThumbColor = bombfork::prong::theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
      scrollbarThumbHoverColor = bombfork::prong::theming::Color(0.5f, 0.5f, 0.5f, 1.0f);

      borderColor = bombfork::prong::theming::Color(0.3f, 0.3f, 0.3f, 1.0f);
    }
  };

private:
  static constexpr float MIN_ZOOM = 0.1f;
  static constexpr float MAX_ZOOM = 20.0f;
  static constexpr float ZOOM_STEP = 1.2f;
  static constexpr int SCROLLBAR_SIZE = 16;
  static constexpr int GRID_SPACING = 32;

  struct ViewportState {
    ViewportMode mode = ViewportMode::CUSTOM;
    ZoomMode zoomMode = ZoomMode::CUSTOM_SCALE;
    PanMode panMode = PanMode::FREE;

    // Content dimensions
    int contentWidth = 0;
    int contentHeight = 0;

    // Viewport dimensions (calculated)
    int viewportWidth = 0;
    int viewportHeight = 0;

    // Interaction state
    bool dragging = false;
    bool selecting = false;
    int dragStartX = 0, dragStartY = 0;
    int dragCurrentX = 0, dragCurrentY = 0;
    float dragStartPanX = 0.0f, dragStartPanY = 0.0f;

    // Selection rectangle
    int selectionX = 0, selectionY = 0;
    int selectionWidth = 0, selectionHeight = 0;
    bool hasSelection = false;

    // UI state
    bool showGrid = false;
    bool showRulers = false;
    bool showScrollbars = true;
    bool showFPS = false;

    // Performance monitoring
    int frameCount = 0;
    std::chrono::steady_clock::time_point lastFPSUpdate;
    float currentFPS = 0.0f;
  };

  ViewportState state;
  ViewportTheme theme;
  ViewportTransform transform;

  // Callbacks
  RenderCallback renderCallback;
  ZoomChangedCallback zoomCallback;
  PanChangedCallback panCallback;
  SelectionCallback selectionCallback;

  // Scrollbar state
  bool horizontalScrollbarHover = false;
  bool verticalScrollbarHover = false;
  bool horizontalScrollbarDrag = false;
  bool verticalScrollbarDrag = false;

public:
  explicit Viewport() : Component(nullptr, "Viewport") {
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

  ~Viewport() override = default;

  // === Configuration ===

  /**
   * @brief Set viewport mode
   */
  void setMode(ViewportMode mode) { state.mode = mode; }

  /**
   * @brief Get viewport mode
   */
  ViewportMode getMode() const { return state.mode; }

  /**
   * @brief Set zoom mode
   */
  void setZoomMode(ZoomMode mode) { state.zoomMode = mode; }

  /**
   * @brief Get zoom mode
   */
  ZoomMode getZoomMode() const { return state.zoomMode; }

  /**
   * @brief Set pan mode
   */
  void setPanMode(PanMode mode) { state.panMode = mode; }

  /**
   * @brief Get pan mode
   */
  PanMode getPanMode() const { return state.panMode; }

  /**
   * @brief Set content size
   */
  void setContentSize(int contentWidth, int contentHeight) {
    state.contentWidth = contentWidth;
    state.contentHeight = contentHeight;
  }

  /**
   * @brief Get content size
   */
  void getContentSize(int& contentWidth, int& contentHeight) const {
    contentWidth = state.contentWidth;
    contentHeight = state.contentHeight;
  }

  // === Transform Management ===

  /**
   * @brief Set zoom level
   */
  void setZoom(float zoomLevel, bool animate = true) {
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

  /**
   * @brief Get current zoom level
   */
  float getZoom() const { return transform.zoomLevel; }

  /**
   * @brief Zoom in by step
   */
  void zoomIn() { setZoom(transform.zoomLevel * ZOOM_STEP, true); }

  /**
   * @brief Zoom out by step
   */
  void zoomOut() { setZoom(transform.zoomLevel / ZOOM_STEP, true); }

  /**
   * @brief Zoom to fit content in viewport
   */
  void zoomToFit() {
    float fitZoom = calculateFitZoom();
    setZoom(fitZoom, true);
  }

  /**
   * @brief Zoom to actual size (1:1)
   */
  void zoomToActualSize() { setZoom(1.0f, true); }

  /**
   * @brief Set pan position
   */
  void setPan(float panX, float panY, bool animate = true) {
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

  /**
   * @brief Get pan position
   */
  void getPan(float& panX, float& panY) const {
    panX = transform.panX;
    panY = transform.panY;
  }

  /**
   * @brief Center content in viewport
   */
  void centerContent() {
    float panX, panY;
    calculateCenterPan(panX, panY);
    setPan(panX, panY, true);
  }

  /**
   * @brief Reset transform to defaults
   */
  void resetTransform() {
    setZoom(1.0f, true);
    setPan(0.0f, 0.0f, true);
  }

  // === Coordinate Transformation ===

  /**
   * @brief Convert viewport coordinates to content coordinates
   */
  void viewportToContent(int viewportX, int viewportY, float& contentX, float& contentY) const {
    contentX = (static_cast<float>(viewportX) - transform.panX) / transform.zoomLevel;
    contentY = (static_cast<float>(viewportY) - transform.panY) / transform.zoomLevel;
  }

  /**
   * @brief Convert content coordinates to viewport coordinates
   */
  void contentToViewport(float contentX, float contentY, int& viewportX, int& viewportY) const {
    viewportX = static_cast<int>(contentX * transform.zoomLevel + transform.panX);
    viewportY = static_cast<int>(contentY * transform.zoomLevel + transform.panY);
  }

  /**
   * @brief Get visible content rectangle
   */
  void getVisibleContentRect(float& x, float& y, float& w, float& h) const {
    viewportToContent(0, 0, x, y);
    w = static_cast<float>(width) / transform.zoomLevel;
    h = static_cast<float>(height) / transform.zoomLevel;
  }

  // === Selection Management ===

  /**
   * @brief Set selection rectangle
   */
  void setSelection(int x, int y, int w, int h) {
    state.selectionX = x;
    state.selectionY = y;
    state.selectionWidth = w;
    state.selectionHeight = h;
    state.hasSelection = true;

    if (selectionCallback) {
      selectionCallback(x, y, w, h);
    }
  }

  /**
   * @brief Get selection rectangle
   */
  bool getSelection(int& x, int& y, int& w, int& h) const {
    if (!state.hasSelection) {
      return false;
    }

    x = state.selectionX;
    y = state.selectionY;
    w = state.selectionWidth;
    h = state.selectionHeight;
    return true;
  }

  /**
   * @brief Clear selection
   */
  void clearSelection() {
    state.hasSelection = false;
    state.selectionX = 0;
    state.selectionY = 0;
    state.selectionWidth = 0;
    state.selectionHeight = 0;
  }

  /**
   * @brief Check if there is a selection
   */
  bool hasSelection() const { return state.hasSelection; }

  // === Visual Options ===

  /**
   * @brief Show/hide grid overlay
   */
  void setShowGrid(bool show) { state.showGrid = show; }

  /**
   * @brief Check if grid is shown
   */
  bool getShowGrid() const { return state.showGrid; }

  /**
   * @brief Show/hide rulers
   */
  void setShowRulers(bool show) { state.showRulers = show; }

  /**
   * @brief Check if rulers are shown
   */
  bool getShowRulers() const { return state.showRulers; }

  /**
   * @brief Show/hide scrollbars
   */
  void setShowScrollbars(bool show) { state.showScrollbars = show; }

  /**
   * @brief Check if scrollbars are shown
   */
  bool getShowScrollbars() const { return state.showScrollbars; }

  /**
   * @brief Show/hide FPS counter
   */
  void setShowFPS(bool show) { state.showFPS = show; }

  /**
   * @brief Check if FPS counter is shown
   */
  bool getShowFPS() const { return state.showFPS; }

  /**
   * @brief Get current FPS
   */
  float getCurrentFPS() const { return state.currentFPS; }

  // === Callbacks ===

  /**
   * @brief Set render callback for custom content
   */
  void setRenderCallback(RenderCallback callback) { renderCallback = std::move(callback); }

  /**
   * @brief Set zoom changed callback
   */
  void setZoomCallback(ZoomChangedCallback callback) { zoomCallback = std::move(callback); }

  /**
   * @brief Set pan changed callback
   */
  void setPanCallback(PanChangedCallback callback) { panCallback = std::move(callback); }

  /**
   * @brief Set selection callback
   */
  void setSelectionCallback(SelectionCallback callback) { selectionCallback = std::move(callback); }

  // === Theming ===

  /**
   * @brief Apply theme from AdvancedTheme system
   */
  void applyTheme(const bombfork::prong::theming::AdvancedTheme& advancedTheme) {
    // Map advanced theme to viewport theme
    theme.backgroundColor = advancedTheme.primary;
    theme.borderColor = advancedTheme.border;
    theme.gridColor = advancedTheme.text;
    theme.rulerColor = advancedTheme.secondary;
    theme.rulerTextColor = advancedTheme.text;
    theme.selectionColor = advancedTheme.accent;
    theme.selectionBorderColor = advancedTheme.accent;
    theme.scrollbarTrackColor = advancedTheme.secondary;
    theme.scrollbarThumbColor = advancedTheme.text;
    theme.scrollbarThumbHoverColor = advancedTheme.accent;
  }

  /**
   * @brief Set custom theme
   */
  void setViewportTheme(const ViewportTheme& customTheme) { theme = customTheme; }

  /**
   * @brief Get current theme
   */
  const ViewportTheme& getViewportTheme() const { return theme; }

  // === Component Overrides ===

  void update(double deltaTime) override {
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

    (void)deltaTime; // Suppress unused parameter warning
  }

  void render() override {
    if (!visible || !renderer)
      return;

    int gx = getGlobalX();
    int gy = getGlobalY();

    // Render background (using drawRect - full implementation would use filled rect)
    renderer->drawRect(gx, gy, width, height, theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b,
                       theme.backgroundColor.a);

    // Render border
    if (theme.borderWidth > 0.0f) {
      renderer->drawRect(gx, gy, width, height, theme.borderColor.r, theme.borderColor.g, theme.borderColor.b,
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

  bool handleEventSelf(const core::Event& event) override {
    switch (event.type) {
    case core::Event::Type::MOUSE_PRESS:
      if (event.button == 0) { // Left mouse button
        state.dragging = true;
        state.dragStartX = event.localX;
        state.dragStartY = event.localY;
        state.dragStartPanX = transform.panX;
        state.dragStartPanY = transform.panY;
        return true;
      }
      break;

    case core::Event::Type::MOUSE_RELEASE:
      if (event.button == 0 && state.dragging) {
        state.dragging = false;
        return true;
      }
      break;

    case core::Event::Type::MOUSE_MOVE:
      if (state.dragging) {
        float deltaX = static_cast<float>(event.localX - state.dragStartX);
        float deltaY = static_cast<float>(event.localY - state.dragStartY);

        setPan(state.dragStartPanX + deltaX, state.dragStartPanY + deltaY, false);

        return true;
      }
      break;

    case core::Event::Type::MOUSE_SCROLL:
      // Handle zoom with mouse wheel
      {
        float zoomFactor = 1.0f + static_cast<float>(event.scrollY) * 0.1f;
        setZoom(transform.zoomLevel * zoomFactor, false);
        return true;
      }

    default:
      break;
    }

    return false;
  }

  void setBounds(int x, int y, int newWidth, int newHeight) override {
    Component::setBounds(x, y, newWidth, newHeight);
  }

  // === Layout Integration ===

  bombfork::prong::layout::LayoutMeasurement measurePreferredSize() const {
    // Return preferred size based on content dimensions
    return bombfork::prong::layout::LayoutMeasurement{state.contentWidth > 0 ? state.contentWidth : 400,
                                                      state.contentHeight > 0 ? state.contentHeight : 300};
  }

private:
  /**
   * @brief Render grid overlay
   */
  void renderGrid() {
    // Stub implementation - would render grid overlay
    // Full implementation would draw grid lines based on zoom and pan
  }

  /**
   * @brief Render rulers
   */
  void renderRulers() {
    // Stub implementation - would render ruler overlays
    // Full implementation would draw rulers with measurements
  }

  /**
   * @brief Render scrollbars
   */
  void renderScrollbars() {
    // Stub implementation - would render scrollbars
    // Full implementation would draw proportional scrollbars
  }

  /**
   * @brief Render FPS counter
   */
  void renderFPS() {
    // Stub implementation - would render FPS counter
    // Full implementation would track and display frame rate
  }

  /**
   * @brief Calculate zoom level for fit-to-window
   */
  float calculateFitZoom() const {
    if (state.contentWidth <= 0 || state.contentHeight <= 0 || width <= 0 || height <= 0) {
      return 1.0f;
    }

    float zoomX = static_cast<float>(width) / static_cast<float>(state.contentWidth);
    float zoomY = static_cast<float>(height) / static_cast<float>(state.contentHeight);

    return std::min(zoomX, zoomY);
  }

  /**
   * @brief Calculate center pan position
   */
  void calculateCenterPan(float& panX, float& panY) const {
    if (state.contentWidth <= 0 || state.contentHeight <= 0) {
      panX = 0.0f;
      panY = 0.0f;
      return;
    }

    // Center the content in the viewport
    panX = (static_cast<float>(width) - static_cast<float>(state.contentWidth) * transform.zoomLevel) / 2.0f;
    panY = (static_cast<float>(height) - static_cast<float>(state.contentHeight) * transform.zoomLevel) / 2.0f;
  }
};

} // namespace bombfork::prong