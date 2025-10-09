#include <bombfork/prong/theming/color.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <exception>
#include <iostream>

using namespace bombfork::prong::theming;

// Helper function for floating point comparison
bool floatEquals(float a, float b, float epsilon = 0.0001f) {
  return std::fabs(a - b) < epsilon;
}

bool colorEquals(const Color& c1, const Color& c2) {
  return floatEquals(c1.r, c2.r) && floatEquals(c1.g, c2.g) && floatEquals(c1.b, c2.b) && floatEquals(c1.a, c2.a);
}

void test_color_construction() {
  // Default constructor
  Color defaultColor;
  assert(colorEquals(defaultColor, Color::WHITE()));

  // Parameterized constructor
  Color customColor(0.5f, 0.6f, 0.7f, 0.8f);
  assert(floatEquals(customColor.r, 0.5f));
  assert(floatEquals(customColor.g, 0.6f));
  assert(floatEquals(customColor.b, 0.7f));
  assert(floatEquals(customColor.a, 0.8f));

  std::cout << "✓ Color construction tests passed\n";
}

void test_color_equality() {
  Color c1(1.0f, 0.5f, 0.0f, 1.0f);
  Color c2(1.0f, 0.5f, 0.0f, 1.0f);
  Color c3(0.5f, 0.5f, 0.0f, 1.0f);

  assert(c1 == c2);
  assert(c1 != c3);
  assert(!(c1 == c3));
  assert(!(c1 != c2));

  std::cout << "✓ Color equality tests passed\n";
}

void test_predefined_colors() {
  Color white = Color::WHITE();
  assert(colorEquals(white, {1.0f, 1.0f, 1.0f, 1.0f}));

  Color black = Color::BLACK();
  assert(colorEquals(black, {0.0f, 0.0f, 0.0f, 1.0f}));

  Color transparent = Color::TRANSPARENT();
  assert(colorEquals(transparent, {0.0f, 0.0f, 0.0f, 0.0f}));

  Color red = Color::RED();
  assert(red.r > 0.5f && red.g < 0.5f && red.b < 0.5f);

  Color green = Color::GREEN();
  assert(green.r < 0.5f && green.g > 0.5f && green.b < 0.5f);

  Color blue = Color::BLUE();
  assert(blue.r < 0.5f && blue.g < 0.8f && blue.b > 0.5f);

  std::cout << "✓ Predefined color tests passed\n";
}

void test_color_blend() {
  Color red = Color::RED();
  Color blue = Color::BLUE();

  // Blend 0% should give first color
  Color blend0 = red.blend(blue, 0.0f);
  assert(colorEquals(blend0, red));

  // Blend 100% should give second color
  Color blend100 = red.blend(blue, 1.0f);
  assert(colorEquals(blend100, blue));

  // Blend 50% should be halfway
  Color blend50 = red.blend(blue, 0.5f);
  assert(floatEquals(blend50.r, (red.r + blue.r) / 2.0f));
  assert(floatEquals(blend50.g, (red.g + blue.g) / 2.0f));
  assert(floatEquals(blend50.b, (red.b + blue.b) / 2.0f));

  std::cout << "✓ Color blend tests passed\n";
}

void test_brightness_adjustment() {
  Color gray = Color::GRAY();

  // Brighten
  Color brighter = gray.adjustBrightness(0.2f);
  assert(brighter.r > gray.r);
  assert(brighter.g > gray.g);
  assert(brighter.b > gray.b);
  assert(floatEquals(brighter.a, gray.a)); // Alpha unchanged

  // Darken
  Color darker = gray.adjustBrightness(-0.2f);
  assert(darker.r < gray.r);
  assert(darker.g < gray.g);
  assert(darker.b < gray.b);

  // Clamping test - shouldn't exceed 1.0
  Color white = Color::WHITE();
  Color brighterWhite = white.adjustBrightness(0.5f);
  assert(floatEquals(brighterWhite.r, 1.0f));
  assert(floatEquals(brighterWhite.g, 1.0f));
  assert(floatEquals(brighterWhite.b, 1.0f));

  // Clamping test - shouldn't go below 0.0
  Color black = Color::BLACK();
  Color darkerBlack = black.adjustBrightness(-0.5f);
  assert(floatEquals(darkerBlack.r, 0.0f));
  assert(floatEquals(darkerBlack.g, 0.0f));
  assert(floatEquals(darkerBlack.b, 0.0f));

  std::cout << "✓ Brightness adjustment tests passed\n";
}

void test_brightness_factor() {
  Color gray = Color::GRAY();

  // Double brightness
  Color doubled = gray.adjustBrightnessFactor(2.0f);
  assert(floatEquals(doubled.r, std::min(1.0f, gray.r * 2.0f)));

  // Half brightness
  Color halved = gray.adjustBrightnessFactor(0.5f);
  assert(floatEquals(halved.r, gray.r * 0.5f));
  assert(floatEquals(halved.g, gray.g * 0.5f));
  assert(floatEquals(halved.b, gray.b * 0.5f));

  // Clamping test
  Color white = Color::WHITE();
  Color scaled = white.adjustBrightnessFactor(2.0f);
  assert(floatEquals(scaled.r, 1.0f));
  assert(floatEquals(scaled.g, 1.0f));
  assert(floatEquals(scaled.b, 1.0f));

  std::cout << "✓ Brightness factor tests passed\n";
}

void test_alpha_adjustment() {
  Color opaque = Color::RED();

  // Set to semi-transparent
  Color semiTransparent = opaque.withAlpha(0.5f);
  assert(floatEquals(semiTransparent.r, opaque.r));
  assert(floatEquals(semiTransparent.g, opaque.g));
  assert(floatEquals(semiTransparent.b, opaque.b));
  assert(floatEquals(semiTransparent.a, 0.5f));

  // Set to fully transparent
  Color transparent = opaque.withAlpha(0.0f);
  assert(floatEquals(transparent.a, 0.0f));

  // Set to fully opaque
  Color fullyOpaque = semiTransparent.withAlpha(1.0f);
  assert(floatEquals(fullyOpaque.a, 1.0f));

  std::cout << "✓ Alpha adjustment tests passed\n";
}

void test_constexpr_operations() {
  // Test that color operations work in constexpr contexts
  constexpr Color white = Color::WHITE();
  constexpr Color black = Color::BLACK();
  constexpr bool areEqual = (white == white);
  constexpr bool areDifferent = (white != black);

  static_assert(areEqual, "White should equal white");
  static_assert(areDifferent, "White should not equal black");

  constexpr Color blended = white.blend(black, 0.5f);
  constexpr Color dimmed = white.adjustBrightnessFactor(0.5f);
  constexpr Color adjusted = white.adjustBrightness(-0.2f);
  constexpr Color transparent = white.withAlpha(0.5f);

  // Use the variables to avoid unused warnings
  (void)blended;
  (void)dimmed;
  (void)adjusted;
  (void)transparent;

  // If we got here, all constexpr operations compiled successfully
  std::cout << "✓ Constexpr operations tests passed\n";
}

int main() {
  std::cout << "Running Color class tests...\n\n";

  try {
    test_color_construction();
    test_color_equality();
    test_predefined_colors();
    test_color_blend();
    test_brightness_adjustment();
    test_brightness_factor();
    test_alpha_adjustment();
    test_constexpr_operations();

    std::cout << "\n✓ All Color tests passed!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\n✗ Test failed with unknown exception\n";
    return 1;
  }
}
