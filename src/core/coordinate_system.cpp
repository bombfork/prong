#include <bombfork/prong/core/coordinate_system.h>

#include <algorithm>
#include <cmath>
#include <exception>
#include <stddef.h>

namespace bombfork::prong {

CoordinateSystem::CoordinateSystem(const ViewportInfo& viewport) : viewport(viewport) {}

void CoordinateSystem::updateViewport(const ViewportInfo& newViewport) {
  viewport = newViewport;
}

void CoordinateSystem::setCameraPosition(float worldX, float worldY) {
  viewport.camera.x = worldX;
  viewport.camera.y = worldY;
}

void CoordinateSystem::setZoomLevel(float zoom) {
  viewport.zoomLevel = std::max(0.1f, std::min(10.0f, zoom));
}

CoordinateSystem::ScreenPosition CoordinateSystem::worldToScreen(const WorldPosition& worldPos) const {
  // Calculate cell dimensions with zoom
  int cellWidth = static_cast<int>(BASE_CELL_WIDTH * viewport.zoomLevel);
  int cellHeight = static_cast<int>(BASE_CELL_HEIGHT * viewport.zoomLevel);

  // Calculate viewport center in screen coordinates
  int viewportCenterX = viewport.screenWidth / 2;
  int viewportCenterY = viewport.screenHeight / 2;

  // Position relative to camera center
  float cellOffsetX = (worldPos.x - viewport.camera.x) * cellWidth;
  float cellOffsetY = (worldPos.y - viewport.camera.y) * cellHeight;

  // Final screen position
  int screenX = viewportCenterX + static_cast<int>(cellOffsetX);
  int screenY = viewportCenterY + static_cast<int>(cellOffsetY);

  return ScreenPosition(screenX, screenY);
}

CoordinateSystem::WorldPosition CoordinateSystem::screenToWorld(const ScreenPosition& screenPos) const {
  // Calculate cell dimensions with zoom
  int cellWidth = static_cast<int>(BASE_CELL_WIDTH * viewport.zoomLevel);
  int cellHeight = static_cast<int>(BASE_CELL_HEIGHT * viewport.zoomLevel);

  // Calculate viewport center in screen coordinates
  int viewportCenterX = viewport.screenWidth / 2;
  int viewportCenterY = viewport.screenHeight / 2;

  // Offset from viewport center
  float cellOffsetX = static_cast<float>(screenPos.x - viewportCenterX) / cellWidth;
  float cellOffsetY = static_cast<float>(screenPos.y - viewportCenterY) / cellHeight;

  // World position
  float worldX = viewport.camera.x + cellOffsetX;
  float worldY = viewport.camera.y + cellOffsetY;

  return WorldPosition(worldX, worldY);
}

CoordinateSystem::ScreenPosition CoordinateSystem::cellToScreen(int cellX, int cellY) const {
  return worldToScreen(WorldPosition(static_cast<float>(cellX), static_cast<float>(cellY)));
}

std::pair<int, int> CoordinateSystem::screenToCell(const ScreenPosition& screenPos) const {
  WorldPosition worldPos = screenToWorld(screenPos);
  return {static_cast<int>(worldPos.x), static_cast<int>(worldPos.y)};
}

std::pair<int, int> CoordinateSystem::getScaledCellSize() const {
  int cellWidth = static_cast<int>(BASE_CELL_WIDTH * viewport.zoomLevel);
  int cellHeight = static_cast<int>(BASE_CELL_HEIGHT * viewport.zoomLevel);
  return {cellWidth, cellHeight};
}

std::pair<float, float> CoordinateSystem::getScaledCellSizeFloat() const {
  float cellWidth = BASE_CELL_WIDTH * viewport.zoomLevel;
  float cellHeight = BASE_CELL_HEIGHT * viewport.zoomLevel;
  return {cellWidth, cellHeight};
}

std::pair<int, int> CoordinateSystem::getViewportCellCount() const {
  auto [cellWidth, cellHeight] = getScaledCellSize();

  // Prevent division by zero for very small cells
  if (cellWidth <= 0)
    cellWidth = 1;
  if (cellHeight <= 0)
    cellHeight = 1;

  // Use ceiling division to ensure we always have enough cells to completely fill the viewport
  // This prevents gaps at the bottom/right when zoomed out
  int requiredCellsWide = (viewport.screenWidth + cellWidth - 1) / cellWidth + 4;    // +4 buffer
  int requiredCellsHigh = (viewport.screenHeight + cellHeight - 1) / cellHeight + 4; // +4 buffer

  // Ensure minimum size for efficient loading
  requiredCellsWide = std::max(20, requiredCellsWide);
  requiredCellsHigh = std::max(20, requiredCellsHigh);

  return {requiredCellsWide, requiredCellsHigh};
}

CoordinateSystem::VisibleRegion CoordinateSystem::getVisibleRegion() const {
  auto [viewportWidth, viewportHeight] = getViewportCellCount();

  // Calculate viewport region based on camera position - no clamping to map bounds
  // This allows the viewport to show empty space when zoomed out or camera is at map edges
  int regionX = static_cast<int>(viewport.camera.x) - viewportWidth / 2;
  int regionY = static_cast<int>(viewport.camera.y) - viewportHeight / 2;

  return VisibleRegion{regionX, regionY, viewportWidth, viewportHeight};
}

bool CoordinateSystem::isCellVisible(int cellX, int cellY) const {
  VisibleRegion region = getVisibleRegion();
  return cellX >= region.startX && cellX < region.startX + region.width && cellY >= region.startY &&
         cellY < region.startY + region.height;
}

bool CoordinateSystem::isScreenPositionValid(const ScreenPosition& pos) const {
  return pos.x >= 0 && pos.x < viewport.screenWidth && pos.y >= 0 && pos.y < viewport.screenHeight;
}

std::string CoordinateSystem::makeCoordKey(uint16_t tileId, int worldX, int worldY) {
  return std::to_string(tileId) + "_" + std::to_string(worldX) + "_" + std::to_string(worldY);
}

bool CoordinateSystem::parseCoordKey(const std::string& key, uint16_t& tileId, int& worldX, int& worldY) {
  size_t firstUnderscore = key.find('_');
  if (firstUnderscore == std::string::npos)
    return false;

  size_t secondUnderscore = key.find('_', firstUnderscore + 1);
  if (secondUnderscore == std::string::npos)
    return false;

  try {
    tileId = static_cast<uint16_t>(std::stoul(key.substr(0, firstUnderscore)));
    worldX = std::stoi(key.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1));
    worldY = std::stoi(key.substr(secondUnderscore + 1));
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

CoordinateSystem::WorldPosition CoordinateSystem::clampToMapBounds(const WorldPosition& pos) {
  float clampedX = std::max(0.0f, std::min(static_cast<float>(MAP_SIZE - 1), pos.x));
  float clampedY = std::max(0.0f, std::min(static_cast<float>(MAP_SIZE - 1), pos.y));
  return WorldPosition(clampedX, clampedY);
}

bool CoordinateSystem::isValidMapCoordinate(int cellX, int cellY) {
  return cellX >= 0 && cellX < MAP_SIZE && cellY >= 0 && cellY < MAP_SIZE;
}

float CoordinateSystem::worldDistance(const WorldPosition& pos1, const WorldPosition& pos2) {
  float dx = pos2.x - pos1.x;
  float dy = pos2.y - pos1.y;
  return std::sqrt(dx * dx + dy * dy);
}

float CoordinateSystem::screenDistance(const ScreenPosition& pos1, const ScreenPosition& pos2) {
  float dx = static_cast<float>(pos2.x - pos1.x);
  float dy = static_cast<float>(pos2.y - pos1.y);
  return std::sqrt(dx * dx + dy * dy);
}

CoordinateSystem::WorldRect CoordinateSystem::screenRectToWorldRect(const ScreenPosition& topLeft, int width,
                                                                    int height) const {
  WorldPosition worldTopLeft = screenToWorld(topLeft);
  WorldPosition worldBottomRight = screenToWorld(ScreenPosition(topLeft.x + width, topLeft.y + height));

  return WorldRect{worldTopLeft, worldBottomRight};
}

} // namespace bombfork::prong