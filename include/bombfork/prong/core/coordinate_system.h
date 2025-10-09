#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace bombfork::prong {

/**
 * @brief T4C-specific coordinate system and transformations
 *
 * Handles conversions between world coordinates, screen coordinates,
 * and T4C-specific cell-based positioning with zoom support.
 */
class CoordinateSystem {
public:
  // T4C standard cell dimensions
  static constexpr int BASE_CELL_WIDTH = 32;
  static constexpr int BASE_CELL_HEIGHT = 16;

  // T4C map dimensions
  static constexpr int MAP_SIZE = 3072;
  static constexpr int MAP_CELLS_TOTAL = MAP_SIZE * MAP_SIZE;

  struct WorldPosition {
    float x, y;

    WorldPosition() : x(0.0f), y(0.0f) {}
    WorldPosition(float x, float y) : x(x), y(y) {}

    // Convert to cell coordinates
    std::pair<int, int> toCellCoords() const { return {static_cast<int>(x), static_cast<int>(y)}; }
  };

  struct ScreenPosition {
    int x, y;

    ScreenPosition() : x(0), y(0) {}
    ScreenPosition(int x, int y) : x(x), y(y) {}
  };

  struct ViewportInfo {
    WorldPosition camera;          // Camera center in world coords
    float zoomLevel = 1.0f;        // Zoom factor
    int screenWidth, screenHeight; // Viewport dimensions in pixels

    ViewportInfo(float cameraX, float cameraY, float zoom, int width, int height)
      : camera(cameraX, cameraY), zoomLevel(zoom), screenWidth(width), screenHeight(height) {}
  };

private:
  ViewportInfo viewport;

public:
  explicit CoordinateSystem(const ViewportInfo& viewport);

  // === Viewport Management ===

  /**
   * @brief Update viewport information
   */
  void updateViewport(const ViewportInfo& newViewport);

  /**
   * @brief Get current viewport
   */
  const ViewportInfo& getViewport() const { return viewport; }

  /**
   * @brief Set camera position
   */
  void setCameraPosition(float worldX, float worldY);

  /**
   * @brief Set zoom level
   */
  void setZoomLevel(float zoom);

  // === Coordinate Transformations ===

  /**
   * @brief Convert world coordinates to screen coordinates
   */
  ScreenPosition worldToScreen(const WorldPosition& worldPos) const;

  /**
   * @brief Convert screen coordinates to world coordinates
   */
  WorldPosition screenToWorld(const ScreenPosition& screenPos) const;

  /**
   * @brief Convert world cell coordinates to screen position
   */
  ScreenPosition cellToScreen(int cellX, int cellY) const;

  /**
   * @brief Convert screen position to world cell coordinates
   */
  std::pair<int, int> screenToCell(const ScreenPosition& screenPos) const;

  // === Cell Calculations ===

  /**
   * @brief Get scaled cell dimensions for current zoom
   */
  std::pair<int, int> getScaledCellSize() const;

  /**
   * @brief Get scaled cell dimensions as floats for precise calculations
   */
  std::pair<float, float> getScaledCellSizeFloat() const;

  /**
   * @brief Calculate how many cells fit in the current viewport
   */
  std::pair<int, int> getViewportCellCount() const;

  /**
   * @brief Get the world bounds that are visible in current viewport
   */
  struct VisibleRegion {
    int startX, startY; // Top-left cell coordinates
    int width, height;  // Size in cells
  };
  VisibleRegion getVisibleRegion() const;

  /**
   * @brief Check if world cell is visible in current viewport
   */
  bool isCellVisible(int cellX, int cellY) const;

  /**
   * @brief Check if screen position is within viewport bounds
   */
  bool isScreenPositionValid(const ScreenPosition& pos) const;

  // === Utility Functions ===

  /**
   * @brief Generate coordinate key for tile ID mapping
   * Format: "tileId_worldX_worldY"
   */
  static std::string makeCoordKey(uint16_t tileId, int worldX, int worldY);

  /**
   * @brief Parse coordinate key back to components
   */
  static bool parseCoordKey(const std::string& key, uint16_t& tileId, int& worldX, int& worldY);

  /**
   * @brief Clamp world coordinates to valid map bounds
   */
  static WorldPosition clampToMapBounds(const WorldPosition& pos);

  /**
   * @brief Check if world coordinates are within map bounds
   */
  static bool isValidMapCoordinate(int cellX, int cellY);

  // === Distance and Area Calculations ===

  /**
   * @brief Calculate distance between two world positions
   */
  static float worldDistance(const WorldPosition& pos1, const WorldPosition& pos2);

  /**
   * @brief Calculate screen distance between two screen positions
   */
  static float screenDistance(const ScreenPosition& pos1, const ScreenPosition& pos2);

  /**
   * @brief Calculate world area covered by screen rectangle
   */
  struct WorldRect {
    WorldPosition topLeft;
    WorldPosition bottomRight;
    float width() const { return bottomRight.x - topLeft.x; }
    float height() const { return bottomRight.y - topLeft.y; }
  };
  WorldRect screenRectToWorldRect(const ScreenPosition& topLeft, int width, int height) const;
};

} // namespace bombfork::prong