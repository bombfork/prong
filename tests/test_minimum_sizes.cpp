#include "mocks/mock_renderer.h"
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/layout/flex_layout.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using namespace bombfork::prong;
using namespace bombfork::prong::tests;

// ============================================================================
// Button Minimum Size Tests
// ============================================================================

void test_button_minimum_size_with_text() {
  std::cout << "Testing button minimum size with text..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16); // 8px per char, 16px height

  Button button("Hello");
  button.setRenderer(&renderer);

  // Text "Hello" = 5 chars * 8px = 40px width
  // With default padding (16px * 2) = 32px
  // With border (1px * 2) = 2px
  // Total: 40 + 32 + 2 = 74px
  int minWidth = button.getMinimumWidth();
  assert(minWidth == 74);

  // Text height = 16px
  // With default padding (8px * 2) = 16px
  // With border (1px * 2) = 2px
  // Total: 16 + 16 + 2 = 34px
  int minHeight = button.getMinimumHeight();
  assert(minHeight == 34);

  std::cout << "✓ Button minimum size with text tests passed (width=" << minWidth << ", height=" << minHeight << ")"
            << std::endl;
}

void test_button_fallback_size_without_text() {
  std::cout << "Testing button fallback size without text..." << std::endl;

  MockRenderer renderer;
  Button button(""); // Empty text
  button.setRenderer(&renderer);

  // Should use fallback sizes for icon-only buttons
  int minWidth = button.getMinimumWidth();
  int minHeight = button.getMinimumHeight();

  assert(minWidth == 50);  // Fallback width
  assert(minHeight == 30); // Fallback height

  std::cout << "✓ Button fallback size without text tests passed (width=" << minWidth << ", height=" << minHeight << ")"
            << std::endl;
}

void test_button_no_renderer() {
  std::cout << "Testing button minimum size without renderer..." << std::endl;

  Button button("Hello");
  // No renderer set

  // Should use fallback sizes
  int minWidth = button.getMinimumWidth();
  int minHeight = button.getMinimumHeight();

  assert(minWidth == 50);  // Fallback width
  assert(minHeight == 30); // Fallback height

  std::cout << "✓ Button minimum size without renderer tests passed" << std::endl;
}

void test_button_different_paddings() {
  std::cout << "Testing button minimum size with custom padding..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  Button button("Test");
  button.setRenderer(&renderer);

  // Customize padding
  auto style = button.getStyle();
  style.paddingX = 20;
  style.paddingY = 10;
  button.setStyle(style);

  // Text "Test" = 4 chars * 8px = 32px
  // With padding (20px * 2) = 40px
  // With border (1px * 2) = 2px
  // Total: 32 + 40 + 2 = 74px
  int minWidth = button.getMinimumWidth();
  assert(minWidth == 74);

  // Text height = 16px
  // With padding (10px * 2) = 20px
  // With border (1px * 2) = 2px
  // Total: 16 + 20 + 2 = 38px
  int minHeight = button.getMinimumHeight();
  assert(minHeight == 38);

  std::cout << "✓ Button minimum size with custom padding tests passed" << std::endl;
}

void test_button_no_border() {
  std::cout << "Testing button minimum size without border..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  Button button("OK");
  button.setRenderer(&renderer);

  // Disable border
  auto style = button.getStyle();
  style.showBorder = false;
  button.setStyle(style);

  // Text "OK" = 2 chars * 8px = 16px
  // With padding (16px * 2) = 32px
  // No border
  // Total: 16 + 32 = 48px
  int minWidth = button.getMinimumWidth();
  assert(minWidth == 48);

  // Text height = 16px
  // With padding (8px * 2) = 16px
  // No border
  // Total: 16 + 16 = 32px
  int minHeight = button.getMinimumHeight();
  assert(minHeight == 32);

  std::cout << "✓ Button minimum size without border tests passed" << std::endl;
}

// ============================================================================
// TextInput Minimum Size Tests
// ============================================================================

void test_textinput_default_sizes() {
  std::cout << "Testing TextInput default minimum sizes..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  TextInput textInput;
  textInput.setRenderer(&renderer);

  // Without placeholder, should use fallback width
  int minWidth = textInput.getMinimumWidth();
  assert(minWidth == 100); // Fallback width

  // Height based on font height
  // Text height = 16px (from measureText("A"))
  // With default padding (4px top + 4px bottom) = 8px
  // With border (1px * 2) = 2px
  // Total: 16 + 8 + 2 = 26px
  int minHeight = textInput.getMinimumHeight();
  assert(minHeight == 26);

  std::cout << "✓ TextInput default minimum sizes tests passed (width=" << minWidth << ", height=" << minHeight << ")"
            << std::endl;
}

void test_textinput_with_placeholder() {
  std::cout << "Testing TextInput minimum size with placeholder..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  TextInput textInput;
  textInput.setRenderer(&renderer);
  textInput.setPlaceholder("Enter your name here");

  // Placeholder "Enter your name here" = 20 chars * 8px = 160px
  // With default padding (8px left + 8px right) = 16px
  // With border (1px * 2) = 2px
  // Total: 160 + 16 + 2 = 178px
  int minWidth = textInput.getMinimumWidth();
  assert(minWidth == 178);

  std::cout << "✓ TextInput minimum size with placeholder tests passed (width=" << minWidth << ")" << std::endl;
}

void test_textinput_no_renderer() {
  std::cout << "Testing TextInput minimum size without renderer..." << std::endl;

  TextInput textInput;
  textInput.setPlaceholder("Placeholder");

  // Without renderer, should use fallback sizes
  int minWidth = textInput.getMinimumWidth();
  int minHeight = textInput.getMinimumHeight();

  assert(minWidth == 100); // Fallback width (no renderer to measure placeholder)
  assert(minHeight == 30); // Fallback height

  std::cout << "✓ TextInput minimum size without renderer tests passed" << std::endl;
}

void test_textinput_custom_padding() {
  std::cout << "Testing TextInput minimum size with custom padding..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  TextInput textInput;
  textInput.setRenderer(&renderer);
  textInput.setPlaceholder("Test");

  // Customize padding
  auto style = textInput.getStyle();
  style.paddingLeft = 10;
  style.paddingRight = 10;
  style.paddingTop = 5;
  style.paddingBottom = 5;
  textInput.setStyle(style);

  // Placeholder "Test" = 4 chars * 8px = 32px
  // With padding (10px + 10px) = 20px
  // With border (1px * 2) = 2px
  // Total: 32 + 20 + 2 = 54px
  int minWidth = textInput.getMinimumWidth();
  assert(minWidth == 54);

  // Text height = 16px
  // With padding (5px + 5px) = 10px
  // With border (1px * 2) = 2px
  // Total: 16 + 10 + 2 = 28px
  int minHeight = textInput.getMinimumHeight();
  assert(minHeight == 28);

  std::cout << "✓ TextInput minimum size with custom padding tests passed" << std::endl;
}

// ============================================================================
// Panel Minimum Size Tests
// ============================================================================

void test_panel_with_no_children() {
  std::cout << "Testing Panel minimum size with no children..." << std::endl;

  MockRenderer renderer;
  Panel panel;
  panel.setRenderer(&renderer);

  // No layout manager, should return 0
  int minWidth = panel.getMinimumWidth();
  int minHeight = panel.getMinimumHeight();

  assert(minWidth == 0);
  assert(minHeight == 0);

  std::cout << "✓ Panel minimum size with no children tests passed" << std::endl;
}

void test_panel_with_children() {
  std::cout << "Testing Panel minimum size based on children..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  Panel panel;
  panel.setRenderer(&renderer);

  // Add a flex layout
  auto layout = std::make_shared<layout::FlexLayout>();
  panel.setLayout(layout);

  // Add two buttons as children
  auto button1 = std::make_unique<Button>("Button 1");
  button1->setRenderer(&renderer);
  button1->setSize(100, 40);

  auto button2 = std::make_unique<Button>("Button 2");
  button2->setRenderer(&renderer);
  button2->setSize(120, 40);

  panel.addChild(std::move(button1));
  panel.addChild(std::move(button2));

  // Panel should report minimum size based on layout
  // The layout measures preferred sizes, not minimum sizes
  // Button 1: setSize(100, 40) -> 100px preferred width
  // Button 2: setSize(120, 40) -> 120px preferred width
  // Total: 100 + 120 = 220px (no gap by default)
  // With border (1px * 2) = 2px
  // With padding (0px * 2) = 0px (default)
  // Total: 220 + 2 = 222px
  int minWidth = panel.getMinimumWidth();

  // Button height: 40px (preferred size)
  // Panel height: 40px + border (2px) = 42px
  int minHeight = panel.getMinimumHeight();

  assert(minWidth == 222);
  assert(minHeight == 42);

  std::cout << "✓ Panel minimum size based on children tests passed (width=" << minWidth << ", height=" << minHeight
            << ")" << std::endl;
}

void test_panel_with_title() {
  std::cout << "Testing Panel minimum size with title bar..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  Panel panel;
  panel.setRenderer(&renderer);
  panel.setTitle("My Panel");

  auto layout = std::make_shared<layout::FlexLayout>();
  panel.setLayout(layout);

  auto button = std::make_unique<Button>("Test");
  button->setRenderer(&renderer);
  button->setSize(100, 40);
  panel.addChild(std::move(button));

  int minHeight = panel.getMinimumHeight();

  // Button: setSize(100, 40) -> 40px preferred height
  // Panel height = button height (40px) + border (2px) + title bar height (25px)
  // Total: 40 + 2 + 25 = 67px
  assert(minHeight == 67);

  std::cout << "✓ Panel minimum size with title bar tests passed (height=" << minHeight << ")" << std::endl;
}

void test_panel_with_padding() {
  std::cout << "Testing Panel minimum size with custom padding..." << std::endl;

  MockRenderer renderer;
  renderer.setFontMetrics(8, 16);

  Panel panel;
  panel.setRenderer(&renderer);

  // Set custom padding
  auto style = panel.getStyle();
  style.padding = 10;
  panel.setStyle(style);

  auto layout = std::make_shared<layout::FlexLayout>();
  panel.setLayout(layout);

  auto button = std::make_unique<Button>("Test");
  button->setRenderer(&renderer);
  button->setSize(100, 40);
  panel.addChild(std::move(button));

  // Button: setSize(100, 40) -> 100px preferred width, 40px preferred height
  // Panel width = button width (100px) + border (2px) + padding (20px)
  // Total: 100 + 2 + 20 = 122px
  int minWidth = panel.getMinimumWidth();
  assert(minWidth == 122);

  // Button height: 40px (preferred)
  // Panel height = button height (40px) + border (2px) + padding (20px)
  // Total: 40 + 2 + 20 = 62px
  int minHeight = panel.getMinimumHeight();
  assert(minHeight == 62);

  std::cout << "✓ Panel minimum size with custom padding tests passed (width=" << minWidth << ", height=" << minHeight
            << ")" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
  try {
    std::cout << "Running Minimum Size tests...\n" << std::endl;

    // Button tests
    test_button_minimum_size_with_text();
    test_button_fallback_size_without_text();
    test_button_no_renderer();
    test_button_different_paddings();
    test_button_no_border();

    // TextInput tests
    test_textinput_default_sizes();
    test_textinput_with_placeholder();
    test_textinput_no_renderer();
    test_textinput_custom_padding();

    // Panel tests
    test_panel_with_no_children();
    test_panel_with_children();
    test_panel_with_title();
    test_panel_with_padding();

    std::cout << "\n✓ All Minimum Size tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception" << std::endl;
    return 1;
  }
}
