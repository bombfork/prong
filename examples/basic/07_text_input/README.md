# 07 - Text Input (Bonus)

Interactive text entry component with full keyboard support and copy/paste functionality.

## What This Demonstrates
- Using TextInput component for user text entry
- Setting up GLFW adapters (clipboard and keyboard)
- Handling text change callbacks
- Using placeholder text
- Copy/paste functionality (Ctrl+C, Ctrl+V)
- Keyboard navigation (arrow keys, Home/End)

## Key Concepts

**TextInput**: A single-line text entry component with full text editing capabilities. Supports cursor movement, text selection, copy/paste, and standard keyboard shortcuts.

**Platform Adapters**: TextInput requires two platform-specific adapters:
- **IClipboard**: Provides clipboard access for copy/paste operations
- **IKeyboard**: Converts platform key codes to Prong's agnostic Key enum

**GLFW Adapters**: Prong provides reference implementations for GLFW:
```cpp
auto adapters = glfw::GLFWAdapters::create(glfwWindow);
textInput->setClipboard(adapters.clipboard.get());
textInput->setKeyboard(adapters.keyboard.get());
```

**Keyboard Shortcuts**:
- `Ctrl+C`: Copy selected text
- `Ctrl+V`: Paste from clipboard
- `Ctrl+X`: Cut selected text
- `Ctrl+A`: Select all text
- `Arrow keys`: Move cursor
- `Home/End`: Jump to start/end
- `Shift+Arrow`: Select text
- `Backspace/Delete`: Remove characters

## Building
```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/07_text_input/07_text_input
```

## Code Walkthrough

1. **GLFW Adapters** (line 66): Create adapters for the window to enable clipboard and keyboard support

2. **Text Inputs** (lines 73-125): Create multiple TextInput components with different placeholders and callbacks

3. **Adapter Injection** (lines 85-86): Inject clipboard and keyboard adapters into each TextInput

4. **Text Change Callback**: Use `withTextChangedCallback()` to be notified when text changes

5. **Important**: Keep the `adapters` object alive for the lifetime of the TextInputs!

## Try This

1. **Click to focus**: Click on any text input to focus it and start typing
2. **Copy/paste**: Type text, select with Shift+Arrows, Ctrl+C to copy, Ctrl+V to paste
3. **Move between fields**: Use Tab to move focus to the next field (if implemented by scene)
4. **Keyboard navigation**: Try Home, End, arrow keys, and Ctrl+A
5. **Watch callbacks**: See console output as you type in each field

**Form Validation Example**:
```cpp
auto emailInput = create<TextInput>()
  .withPlaceholder("user@example.com")
  .withTextChangedCallback([](const std::string& text) {
    bool valid = text.find('@') != std::string::npos;
    std::cout << (valid ? "Valid" : "Invalid") << " email" << std::endl;
  })
  .build();
```

**Common Use Cases**:
- Login forms (username, password)
- Search boxes
- Settings panels (name, URL, path inputs)
- Chat message entry
- Command line interfaces
- Filter/query builders

**Note**: TextInput is single-line only. For multi-line text editing, you would need to implement a TextArea component (not included in basic examples).

**Pro Tip**: Always inject the adapters! Without them, copy/paste and some keyboard shortcuts won't work. The TextInput will still function for basic typing, but the user experience will be degraded.
