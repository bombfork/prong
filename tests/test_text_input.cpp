#include "mocks/mock_clipboard.h"
#include "mocks/mock_keyboard.h"
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/events/ikeyboard.h>
#include <bombfork/prong/theming/color.h>

#include <cassert>
#include <cctype>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

using namespace bombfork::prong;
using namespace bombfork::prong::tests;

// Helper function to simulate key press
void simulateKey(TextInput& input, events::Key key, bool shift = false, bool ctrl = false) {
  uint8_t mods = 0;
  if (shift)
    mods |= static_cast<uint8_t>(events::KeyModifier::SHIFT);
  if (ctrl)
    mods |= static_cast<uint8_t>(events::KeyModifier::CONTROL);

  int platformKey = static_cast<int>(key);
  int action = static_cast<int>(events::KeyAction::PRESS);
  input.handleKey(platformKey, action, mods);
}

// Helper function to simulate character input
void simulateChar(TextInput& input, char c) {
  input.handleChar(static_cast<unsigned int>(c));
}

// Helper function to simulate text input
void simulateText(TextInput& input, const std::string& text) {
  for (char c : text) {
    simulateChar(input, c);
  }
}

void test_text_input_and_retrieval() {
  std::cout << "Testing text input and retrieval..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  // Test setText and getText
  input.setText("Hello World");
  assert(input.getText() == "Hello World");

  // Test clear
  input.clear();
  assert(input.getText().empty());

  // Test character input
  simulateChar(input, 'A');
  simulateChar(input, 'B');
  simulateChar(input, 'C');
  assert(input.getText() == "ABC");

  std::cout << "✓ Text input and retrieval tests passed" << std::endl;
}

void test_copy_to_clipboard() {
  std::cout << "Testing copy to clipboard..." << std::endl;

  TextInput input;
  MockClipboard clipboard;
  MockKeyboard keyboard;
  input.setClipboard(&clipboard);
  input.setKeyboard(&keyboard);

  // Set text and select all
  input.setText("Copy Me");
  input.selectAll();
  assert(input.hasSelection());
  assert(input.getSelectedText() == "Copy Me");

  // Copy to clipboard (Ctrl+C)
  simulateKey(input, events::Key::C, false, true);
  assert(clipboard.getString() == "Copy Me");

  std::cout << "✓ Copy to clipboard tests passed" << std::endl;
}

void test_paste_from_clipboard() {
  std::cout << "Testing paste from clipboard..." << std::endl;

  TextInput input;
  MockClipboard clipboard;
  MockKeyboard keyboard;
  input.setClipboard(&clipboard);
  input.setKeyboard(&keyboard);

  // Put text in clipboard
  clipboard.setString("Pasted Text");
  assert(clipboard.hasText());

  // Paste from clipboard (Ctrl+V)
  simulateKey(input, events::Key::V, false, true);
  assert(input.getText() == "Pasted Text");

  // Paste should replace selection
  input.selectAll();
  clipboard.setString("New");
  simulateKey(input, events::Key::V, false, true);
  assert(input.getText() == "New");

  std::cout << "✓ Paste from clipboard tests passed" << std::endl;
}

void test_text_editing_operations() {
  std::cout << "Testing text editing operations..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  // Test basic insertion
  simulateText(input, "Hello");
  assert(input.getText() == "Hello");
  assert(input.getCursorPosition() == 5);

  // Test backspace
  simulateKey(input, events::Key::BACKSPACE);
  assert(input.getText() == "Hell");
  assert(input.getCursorPosition() == 4);

  // Test delete at end (should do nothing)
  simulateKey(input, events::Key::DELETE);
  assert(input.getText() == "Hell");

  // Move cursor to middle and delete
  input.setCursorPosition(2);
  simulateKey(input, events::Key::DELETE);
  assert(input.getText() == "Hel");
  assert(input.getCursorPosition() == 2);

  // Test backspace in middle
  simulateKey(input, events::Key::BACKSPACE);
  assert(input.getText() == "Hl");
  assert(input.getCursorPosition() == 1);

  std::cout << "✓ Text editing operations tests passed" << std::endl;
}

void test_cursor_movement() {
  std::cout << "Testing cursor movement..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  input.setText("Hello World");
  input.setCursorPosition(0);

  // Test right arrow
  simulateKey(input, events::Key::RIGHT);
  assert(input.getCursorPosition() == 1);

  // Test left arrow
  simulateKey(input, events::Key::LEFT);
  assert(input.getCursorPosition() == 0);

  // Test Home key
  input.setCursorPosition(5);
  simulateKey(input, events::Key::HOME);
  assert(input.getCursorPosition() == 0);

  // Test End key
  simulateKey(input, events::Key::END);
  assert(input.getCursorPosition() == 11);

  std::cout << "✓ Cursor movement tests passed" << std::endl;
}

void test_selection_handling() {
  std::cout << "Testing selection handling..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  input.setText("Hello World");
  input.setCursorPosition(0);

  // Test select all (Ctrl+A)
  simulateKey(input, events::Key::A, false, true);
  assert(input.hasSelection());
  assert(input.getSelectedText() == "Hello World");

  // Clear selection
  input.clearSelection();
  assert(!input.hasSelection());

  // Test shift+arrow selection
  input.setCursorPosition(0);
  simulateKey(input, events::Key::RIGHT, true, false); // Shift+Right
  assert(input.hasSelection());
  assert(input.getSelectedText() == "H");

  simulateKey(input, events::Key::RIGHT, true, false); // Another Shift+Right
  assert(input.getSelectedText() == "He");

  // Test shift+home selection
  input.setCursorPosition(5);
  simulateKey(input, events::Key::HOME, true, false); // Shift+Home
  assert(input.hasSelection());
  assert(input.getSelectedText() == "Hello");

  // Test shift+end selection
  input.clearSelection();
  input.setCursorPosition(0);
  simulateKey(input, events::Key::END, true, false); // Shift+End
  assert(input.hasSelection());
  assert(input.getSelectedText() == "Hello World");

  // Test typing replaces selection
  input.selectAll();
  simulateText(input, "New");
  assert(input.getText() == "New");
  assert(!input.hasSelection());

  std::cout << "✓ Selection handling tests passed" << std::endl;
}

void test_placeholder_behavior() {
  std::cout << "Testing placeholder behavior..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  // Test placeholder
  input.setPlaceholder("Enter text here...");
  assert(input.getPlaceholder() == "Enter text here...");

  // When text is empty, placeholder should be shown (visually, we can't test rendering)
  assert(input.getText().empty());

  // When text is added, text should be shown instead of placeholder
  simulateText(input, "Some text");
  assert(input.getText() == "Some text");

  // Clear should show placeholder again
  input.clear();
  assert(input.getText().empty());

  std::cout << "✓ Placeholder behavior tests passed" << std::endl;
}

void test_validation_callbacks() {
  std::cout << "Testing validation callbacks..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  // Set validator that only allows alphanumeric
  bool validationCalled = false;
  input.setValidator([&validationCalled](const std::string& text) {
    validationCalled = true;
    for (char c : text) {
      if (!std::isalnum(c))
        return false;
    }
    return true;
  });

  // Try to set valid text
  input.setText("ValidText123");
  assert(validationCalled);
  assert(input.getText() == "ValidText123");

  // Try to set invalid text (with space)
  validationCalled = false;
  input.setText("Invalid Text");
  assert(validationCalled);
  assert(input.getText() == "ValidText123"); // Should not change

  // Test text change callback
  std::string lastText;
  input.setOnTextChanged([&lastText](const std::string& text) { lastText = text; });

  input.setText("NewText");
  assert(lastText == "NewText");

  std::cout << "✓ Validation callbacks tests passed" << std::endl;
}

void test_max_length_enforcement() {
  std::cout << "Testing max length enforcement..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  // Set max length
  input.setMaxLength(5);
  assert(input.getMaxLength() == 5);

  // Try to set text within limit
  input.setText("Hello");
  assert(input.getText() == "Hello");

  // Try to set text exceeding limit
  input.setText("Hello World");
  assert(input.getText() == "Hello"); // Should not change

  // Try to type beyond limit
  input.clear();
  simulateText(input, "12345");
  assert(input.getText() == "12345");

  // Additional character should not be added
  simulateChar(input, '6');
  assert(input.getText() == "12345"); // Still only 5 characters

  std::cout << "✓ Max length enforcement tests passed" << std::endl;
}

void test_cut_operation() {
  std::cout << "Testing cut operation..." << std::endl;

  TextInput input;
  MockClipboard clipboard;
  MockKeyboard keyboard;
  input.setClipboard(&clipboard);
  input.setKeyboard(&keyboard);

  // Set text and select part of it
  input.setText("Hello World");
  input.setCursorPosition(0);
  simulateKey(input, events::Key::RIGHT, true, false); // Select "H"
  simulateKey(input, events::Key::RIGHT, true, false); // Select "He"
  simulateKey(input, events::Key::RIGHT, true, false); // Select "Hel"
  simulateKey(input, events::Key::RIGHT, true, false); // Select "Hell"
  simulateKey(input, events::Key::RIGHT, true, false); // Select "Hello"

  assert(input.hasSelection());
  assert(input.getSelectedText() == "Hello");

  // Cut (Ctrl+X)
  simulateKey(input, events::Key::X, false, true);
  assert(clipboard.getString() == "Hello");
  assert(input.getText() == " World");
  assert(!input.hasSelection());

  std::cout << "✓ Cut operation tests passed" << std::endl;
}

void test_focus_management() {
  std::cout << "Testing focus management..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  // TextInput should be able to receive focus
  assert(input.canReceiveFocus());

  // Simulate click to gain focus
  input.handleMousePress(5, 5, 0); // Left mouse button
  input.requestFocus();            // In real usage, EventDispatcher would call this

  // When disabled, should not receive focus
  input.setEnabled(false);
  assert(!input.canReceiveFocus());

  input.setEnabled(true);
  assert(input.canReceiveFocus());

  std::cout << "✓ Focus management tests passed" << std::endl;
}

void test_mouse_selection() {
  std::cout << "Testing mouse selection..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);
  input.setSize(200, 30);
  input.setPosition(10, 10);

  input.setText("Hello World");

  // Simulate mouse press
  input.handleMousePress(15, 15, 0);
  assert(!input.hasSelection());

  // Note: Full mouse selection testing would require accurate text measurement,
  // which depends on the renderer implementation. We're testing the basic mechanism.

  std::cout << "✓ Mouse selection tests passed" << std::endl;
}

void test_empty_operations() {
  std::cout << "Testing operations on empty input..." << std::endl;

  TextInput input;
  MockClipboard clipboard;
  MockKeyboard keyboard;
  input.setClipboard(&clipboard);
  input.setKeyboard(&keyboard);

  // Test backspace on empty input
  simulateKey(input, events::Key::BACKSPACE);
  assert(input.getText().empty());

  // Test delete on empty input
  simulateKey(input, events::Key::DELETE);
  assert(input.getText().empty());

  // Test copy with no selection
  clipboard.setString("existing");
  simulateKey(input, events::Key::C, false, true);
  assert(clipboard.getString() == "existing"); // Should not change

  // Test paste into empty input
  clipboard.setString("New Text");
  simulateKey(input, events::Key::V, false, true);
  assert(input.getText() == "New Text");

  std::cout << "✓ Empty operations tests passed" << std::endl;
}

void test_style_management() {
  std::cout << "Testing style management..." << std::endl;

  TextInput input;
  MockKeyboard keyboard;
  input.setKeyboard(&keyboard);

  // Test default style
  const auto& defaultStyle = input.getStyle();
  assert(defaultStyle.fontSize == 14);

  // Test custom style
  TextInput::Style customStyle;
  customStyle.fontSize = 18;
  customStyle.backgroundColor = theming::Color::WHITE();
  customStyle.textColor = theming::Color::BLACK();

  input.setStyle(customStyle);
  const auto& retrievedStyle = input.getStyle();
  assert(retrievedStyle.fontSize == 18);

  std::cout << "✓ Style management tests passed" << std::endl;
}

int main() {
  try {
    std::cout << "Running TextInput tests..." << std::endl;
    std::cout << "============================\n" << std::endl;

    test_text_input_and_retrieval();
    test_copy_to_clipboard();
    test_paste_from_clipboard();
    test_text_editing_operations();
    test_cursor_movement();
    test_selection_handling();
    test_placeholder_behavior();
    test_validation_callbacks();
    test_max_length_enforcement();
    test_cut_operation();
    test_focus_management();
    test_mouse_selection();
    test_empty_operations();
    test_style_management();

    std::cout << "\n=============================" << std::endl;
    std::cout << "✓ All TextInput tests passed!" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception" << std::endl;
    return 1;
  }
}
