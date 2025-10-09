#include <bombfork/prong/core/coordinate_system.h>

#include <cassert>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <utility>

using namespace bombfork::prong;

// Helper function for floating point comparison
bool floatEquals(float a, float b, float epsilon = 0.1f) {
  return std::fabs(a - b) < epsilon;
}

void test_viewport_construction() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  const auto& vp = coordSys.getViewport();
  assert(floatEquals(vp.camera.x, 1536.0f));
  assert(floatEquals(vp.camera.y, 1536.0f));
  assert(floatEquals(vp.zoomLevel, 1.0f));
  assert(vp.screenWidth == 800);
  assert(vp.screenHeight == 600);

  std::cout << "✓ Viewport construction tests passed\n";
}

void test_viewport_updates() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  // Update camera position
  coordSys.setCameraPosition(2000.0f, 2000.0f);
  assert(floatEquals(coordSys.getViewport().camera.x, 2000.0f));
  assert(floatEquals(coordSys.getViewport().camera.y, 2000.0f));

  // Update zoom level
  coordSys.setZoomLevel(2.0f);
  assert(floatEquals(coordSys.getViewport().zoomLevel, 2.0f));

  // Update entire viewport
  CoordinateSystem::ViewportInfo newViewport(500.0f, 500.0f, 0.5f, 1024, 768);
  coordSys.updateViewport(newViewport);
  assert(floatEquals(coordSys.getViewport().camera.x, 500.0f));
  assert(floatEquals(coordSys.getViewport().camera.y, 500.0f));
  assert(floatEquals(coordSys.getViewport().zoomLevel, 0.5f));
  assert(coordSys.getViewport().screenWidth == 1024);
  assert(coordSys.getViewport().screenHeight == 768);

  std::cout << "✓ Viewport update tests passed\n";
}

void test_cell_size_calculations() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  // Base cell size at zoom 1.0
  auto [width, height] = coordSys.getScaledCellSize();
  assert(width == CoordinateSystem::BASE_CELL_WIDTH);
  assert(height == CoordinateSystem::BASE_CELL_HEIGHT);

  // Double zoom should double cell size
  coordSys.setZoomLevel(2.0f);
  auto [width2, height2] = coordSys.getScaledCellSize();
  assert(width2 == CoordinateSystem::BASE_CELL_WIDTH * 2);
  assert(height2 == CoordinateSystem::BASE_CELL_HEIGHT * 2);

  // Half zoom should halve cell size
  coordSys.setZoomLevel(0.5f);
  auto [width3, height3] = coordSys.getScaledCellSize();
  assert(width3 == CoordinateSystem::BASE_CELL_WIDTH / 2);
  assert(height3 == CoordinateSystem::BASE_CELL_HEIGHT / 2);

  std::cout << "✓ Cell size calculation tests passed\n";
}

void test_world_screen_conversion() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  // Center of viewport should map to center of screen
  CoordinateSystem::WorldPosition worldCenter(1536.0f, 1536.0f);
  CoordinateSystem::ScreenPosition screenCenter = coordSys.worldToScreen(worldCenter);
  assert(std::abs(screenCenter.x - 400) < 5); // Allow small error
  assert(std::abs(screenCenter.y - 300) < 5);

  // Convert back to world coordinates
  CoordinateSystem::WorldPosition worldBack = coordSys.screenToWorld(screenCenter);
  assert(floatEquals(worldBack.x, worldCenter.x));
  assert(floatEquals(worldBack.y, worldCenter.y));

  std::cout << "✓ World-screen conversion tests passed\n";
}

void test_cell_screen_conversion() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  // Test cell to screen conversion
  CoordinateSystem::ScreenPosition screen = coordSys.cellToScreen(1536, 1536);

  // Convert back to cell
  auto [cellX, cellY] = coordSys.screenToCell(screen);

  // Should be approximately the same (allowing for rounding)
  assert(std::abs(cellX - 1536) <= 1);
  assert(std::abs(cellY - 1536) <= 1);

  std::cout << "✓ Cell-screen conversion tests passed\n";
}

void test_viewport_cell_count() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  auto [cellsX, cellsY] = coordSys.getViewportCellCount();

  // At zoom 1.0 with 800x600 screen and 32x16 cells:
  // cellsX = 800 / 32 = 25
  // cellsY = 600 / 16 = 37.5 ≈ 38
  assert(cellsX > 0 && cellsX < 100);
  assert(cellsY > 0 && cellsY < 100);

  // Double zoom should halve cell count
  coordSys.setZoomLevel(2.0f);
  auto [cellsX2, cellsY2] = coordSys.getViewportCellCount();
  assert(cellsX2 <= cellsX);
  assert(cellsY2 <= cellsY);

  std::cout << "✓ Viewport cell count tests passed\n";
}

void test_visible_region() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  auto region = coordSys.getVisibleRegion();

  // Region should be centered around camera
  assert(region.width > 0);
  assert(region.height > 0);
  assert(region.startX >= 0);
  assert(region.startY >= 0);

  // Center of visible region should be near camera position
  float centerX = region.startX + region.width / 2.0f;
  float centerY = region.startY + region.height / 2.0f;
  assert(floatEquals(centerX, 1536.0f, 20.0f));
  assert(floatEquals(centerY, 1536.0f, 20.0f));

  std::cout << "✓ Visible region tests passed\n";
}

void test_cell_visibility() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  // Cell at camera position should be visible
  assert(coordSys.isCellVisible(1536, 1536));

  // Cell very far away should not be visible
  assert(!coordSys.isCellVisible(0, 0));
  assert(!coordSys.isCellVisible(3000, 3000));

  std::cout << "✓ Cell visibility tests passed\n";
}

void test_screen_position_validation() {
  CoordinateSystem::ViewportInfo viewport(1536.0f, 1536.0f, 1.0f, 800, 600);
  CoordinateSystem coordSys(viewport);

  // Valid positions
  assert(coordSys.isScreenPositionValid({0, 0}));
  assert(coordSys.isScreenPositionValid({400, 300}));
  assert(coordSys.isScreenPositionValid({799, 599}));

  // Invalid positions
  assert(!coordSys.isScreenPositionValid({-1, 0}));
  assert(!coordSys.isScreenPositionValid({0, -1}));
  assert(!coordSys.isScreenPositionValid({800, 600}));
  assert(!coordSys.isScreenPositionValid({1000, 1000}));

  std::cout << "✓ Screen position validation tests passed\n";
}

void test_coord_key_functions() {
  // Make coordinate key
  std::string key = CoordinateSystem::makeCoordKey(123, 456, 789);
  assert(!key.empty());

  // Parse it back
  uint16_t tileId;
  int worldX, worldY;
  bool success = CoordinateSystem::parseCoordKey(key, tileId, worldX, worldY);

  assert(success);
  assert(tileId == 123);
  assert(worldX == 456);
  assert(worldY == 789);

  // Test invalid key
  bool invalidParse = CoordinateSystem::parseCoordKey("invalid_key", tileId, worldX, worldY);
  assert(!invalidParse);

  std::cout << "✓ Coordinate key function tests passed\n";
}

void test_map_bounds() {
  // Valid coordinates
  assert(CoordinateSystem::isValidMapCoordinate(0, 0));
  assert(CoordinateSystem::isValidMapCoordinate(1536, 1536));
  assert(CoordinateSystem::isValidMapCoordinate(3071, 3071));

  // Invalid coordinates
  assert(!CoordinateSystem::isValidMapCoordinate(-1, 0));
  assert(!CoordinateSystem::isValidMapCoordinate(0, -1));
  assert(!CoordinateSystem::isValidMapCoordinate(3072, 0));
  assert(!CoordinateSystem::isValidMapCoordinate(0, 3072));

  // Clamp to bounds
  CoordinateSystem::WorldPosition outOfBounds(-10.0f, 5000.0f);
  CoordinateSystem::WorldPosition clamped = CoordinateSystem::clampToMapBounds(outOfBounds);
  assert(clamped.x >= 0.0f && clamped.x < CoordinateSystem::MAP_SIZE);
  assert(clamped.y >= 0.0f && clamped.y < CoordinateSystem::MAP_SIZE);

  std::cout << "✓ Map bounds tests passed\n";
}

void test_distance_calculations() {
  // World distance
  CoordinateSystem::WorldPosition p1(0.0f, 0.0f);
  CoordinateSystem::WorldPosition p2(3.0f, 4.0f);
  float worldDist = CoordinateSystem::worldDistance(p1, p2);
  assert(floatEquals(worldDist, 5.0f)); // 3-4-5 triangle

  // Screen distance
  CoordinateSystem::ScreenPosition s1(0, 0);
  CoordinateSystem::ScreenPosition s2(30, 40);
  float screenDist = CoordinateSystem::screenDistance(s1, s2);
  assert(floatEquals(screenDist, 50.0f)); // 30-40-50 triangle

  std::cout << "✓ Distance calculation tests passed\n";
}

void test_world_position_to_cell() {
  CoordinateSystem::WorldPosition pos(10.5f, 20.7f);
  auto [x, y] = pos.toCellCoords();
  assert(x == 10);
  assert(y == 20);

  std::cout << "✓ WorldPosition to cell tests passed\n";
}

int main() {
  std::cout << "Running CoordinateSystem class tests...\n\n";

  try {
    test_viewport_construction();
    test_viewport_updates();
    test_cell_size_calculations();
    test_world_screen_conversion();
    test_cell_screen_conversion();
    test_viewport_cell_count();
    test_visible_region();
    test_cell_visibility();
    test_screen_position_validation();
    test_coord_key_functions();
    test_map_bounds();
    test_distance_calculations();
    test_world_position_to_cell();

    std::cout << "\n✓ All CoordinateSystem tests passed!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\n✗ Test failed with unknown exception\n";
    return 1;
  }
}
