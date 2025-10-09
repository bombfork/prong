#pragma once

#include "../layout/layout_manager.h"
#include "../layout/layout_measurement.h"
#include "../theming/advanced_theme.h"
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/rendering/irenderer.h>

#include <chrono>
#include <functional>
#include <memory>

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
  explicit Viewport(bombfork::prong::rendering::IRenderer* renderer = nullptr);
  ~Viewport() override = default;

  // === Configuration ===

  /**
   * @brief Set viewport mode
   */
  void setMode(ViewportMode mode);

  /**
   * @brief Get viewport mode
   */
  ViewportMode getMode() const { return state.mode; }

  /**
   * @brief Set zoom mode
   */
  void setZoomMode(ZoomMode mode);

  /**
   * @brief Get zoom mode
   */
  ZoomMode getZoomMode() const { return state.zoomMode; }

  /**
   * @brief Set pan mode
   */
  void setPanMode(PanMode mode);

  /**
   * @brief Get pan mode
   */
  PanMode getPanMode() const { return state.panMode; }

  /**
   * @brief Set content size
   */
  void setContentSize(int width, int height);

  /**
   * @brief Get content size
   */
  void getContentSize(int& width, int& height) const;

  // === Transform Management ===

  /**
   * @brief Set zoom level
   */
  void setZoom(float zoomLevel, bool animate = true);

  /**
   * @brief Get current zoom level
   */
  float getZoom() const { return transform.zoomLevel; }

  /**
   * @brief Zoom in by step
   */
  void zoomIn();

  /**
   * @brief Zoom out by step
   */
  void zoomOut();

  /**
   * @brief Zoom to fit content in viewport
   */
  void zoomToFit();

  /**
   * @brief Zoom to actual size (1:1)
   */
  void zoomToActualSize();

  /**
   * @brief Set pan position
   */
  void setPan(float panX, float panY, bool animate = true);

  /**
   * @brief Get pan position
   */
  void getPan(float& panX, float& panY) const;

  /**
   * @brief Center content in viewport
   */
  void centerContent();

  /**
   * @brief Reset transform to defaults
   */
  void resetTransform();

  // === Coordinate Transformation ===

  /**
   * @brief Convert viewport coordinates to content coordinates
   */
  void viewportToContent(int viewportX, int viewportY, float& contentX, float& contentY) const;

  /**
   * @brief Convert content coordinates to viewport coordinates
   */
  void contentToViewport(float contentX, float contentY, int& viewportX, int& viewportY) const;

  /**
   * @brief Get visible content rectangle
   */
  void getVisibleContentRect(float& x, float& y, float& width, float& height) const;

  // === Selection Management ===

  /**
   * @brief Set selection rectangle
   */
  void setSelection(int x, int y, int width, int height);

  /**
   * @brief Get selection rectangle
   */
  bool getSelection(int& x, int& y, int& width, int& height) const;

  /**
   * @brief Clear selection
   */
  void clearSelection();

  /**
   * @brief Check if there is a selection
   */
  bool hasSelection() const { return state.hasSelection; }

  // === Visual Options ===

  /**
   * @brief Show/hide grid overlay
   */
  void setShowGrid(bool show);

  /**
   * @brief Check if grid is shown
   */
  bool getShowGrid() const { return state.showGrid; }

  /**
   * @brief Show/hide rulers
   */
  void setShowRulers(bool show);

  /**
   * @brief Check if rulers are shown
   */
  bool getShowRulers() const { return state.showRulers; }

  /**
   * @brief Show/hide scrollbars
   */
  void setShowScrollbars(bool show);

  /**
   * @brief Check if scrollbars are shown
   */
  bool getShowScrollbars() const { return state.showScrollbars; }

  /**
   * @brief Show/hide FPS counter
   */
  void setShowFPS(bool show);

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
  void setRenderCallback(RenderCallback callback);

  /**
   * @brief Set zoom changed callback
   */
  void setZoomCallback(ZoomChangedCallback callback);

  /**
   * @brief Set pan changed callback
   */
  void setPanCallback(PanChangedCallback callback);

  /**
   * @brief Set selection callback
   */
  void setSelectionCallback(SelectionCallback callback);

  // === Theming ===

  /**
   * @brief Apply theme from AdvancedTheme system
   */
  void applyTheme(const bombfork::prong::theming::AdvancedTheme& theme);

  /**
   * @brief Set custom theme
   */
  void setViewportTheme(const ViewportTheme& customTheme);

  /**
   * @brief Get current theme
   */
  const ViewportTheme& getViewportTheme() const { return theme; }

  // === Component Overrides ===

  void render() override;
  bool handleClick(int localX, int localY) override;
  bool handleMousePress(int localX, int localY, int button) override;
  bool handleMouseRelease(int localX, int localY, int button) override;
  bool handleMouseMove(int localX, int localY) override;
  bool handleKey(int key, int action, int mods) override;
  bool handleScroll(int localX, int localY, double xoffset, double yoffset) override;
  void setBounds(int x, int y, int width, int height) override;

  // === Layout Integration ===

  bombfork::prong::layout::LayoutMeasurement measurePreferredSize() const;

private:
  /**
   * @brief Update transform animation
   */
  void updateTransform();

  /**
   * @brief Update viewport dimensions
   */
  void updateViewportDimensions();

  /**
   * @brief Constrain pan to content bounds
   */
  void constrainPan();

  /**
   * @brief Render background pattern
   */
  void renderBackground();

  /**
   * @brief Render grid overlay
   */
  void renderGrid();

  /**
   * @brief Render rulers
   */
  void renderRulers();

  /**
   * @brief Render viewport content
   */
  void renderContent();

  /**
   * @brief Render selection rectangle
   */
  void renderSelection();

  /**
   * @brief Render scrollbars
   */
  void renderScrollbars();

  /**
   * @brief Render FPS counter
   */
  void renderFPS();

  /**
   * @brief Render viewport border
   */
  void renderBorder();

  /**
   * @brief Get horizontal scrollbar rectangle
   */
  bombfork::prong::layout::Rect getHorizontalScrollbarRect() const;

  /**
   * @brief Get vertical scrollbar rectangle
   */
  bombfork::prong::layout::Rect getVerticalScrollbarRect() const;

  /**
   * @brief Get horizontal scrollbar thumb rectangle
   */
  bombfork::prong::layout::Rect getHorizontalScrollbarThumbRect() const;

  /**
   * @brief Get vertical scrollbar thumb rectangle
   */
  bombfork::prong::layout::Rect getVerticalScrollbarThumbRect() const;

  /**
   * @brief Check if point is in scrollbar
   */
  bool isPointInScrollbar(int localX, int localY, bool& horizontal, bool& thumb) const;

  /**
   * @brief Handle scrollbar drag
   */
  void handleScrollbarDrag(int localX, int localY, bool horizontal);

  /**
   * @brief Update FPS counter
   */
  void updateFPS();

  /**
   * @brief Get current time for animations
   */
  std::chrono::steady_clock::time_point getCurrentTime() const;

  /**
   * @brief Ease animation curve
   */
  float easeInOutCubic(float t) const;

  /**
   * @brief Calculate zoom level for fit-to-window
   */
  float calculateFitZoom() const;

  /**
   * @brief Calculate center pan position
   */
  void calculateCenterPan(float& panX, float& panY) const;

  /**
   * @brief Notify transform changed
   */
  void notifyTransformChanged();

  /**
   * @brief Start selection
   */
  void startSelection(int x, int y);

  /**
   * @brief Update selection during drag
   */
  void updateSelection(int x, int y);

  /**
   * @brief End selection
   */
  void endSelection();
};

} // namespace bombfork::prong