#pragma once

#include <GLFW/glfw3.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/rendering/irenderer.h>
#include <bombfork/prong/theming/color.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <string>

namespace bombfork::prong {

/**
 * @brief Modern TextInput component for the new UI system
 *
 * A single-line text input field with full editing capabilities:
 * - Text input with cursor positioning
 * - Selection support (mouse drag, shift+arrows)
 * - Keyboard navigation (arrows, home, end)
 * - Copy/paste/cut via clipboard
 * - Placeholder text
 * - Focus states with themed borders
 * - Cursor blinking animation
 * - Horizontal scrolling for long text
 * - Text validation callbacks
 *
 * This is the canonical TextInput class for t4c::ui namespace components.
 */
class TextInput : public Component {
public:
  using TextChangeCallback = std::function<void(const std::string&)>;
  using ValidationCallback = std::function<bool(const std::string&)>;

  struct Style {
    theming::Color backgroundColor = theming::Color(0.94f, 0.94f, 0.94f, 1.0f);
    theming::Color textColor = theming::Color::BLACK();
    theming::Color selectionColor = theming::Color(0.39f, 0.59f, 1.0f, 0.5f);
    theming::Color cursorColor = theming::Color::BLACK();
    theming::Color borderColor = theming::Color(0.5f, 0.5f, 0.5f, 1.0f);
    theming::Color focusBorderColor = theming::Color(0.2f, 0.59f, 1.0f, 1.0f);
    theming::Color placeholderColor = theming::Color(0.5f, 0.5f, 0.5f, 1.0f);

    float borderWidth = 1.0f;
    float focusBorderWidth = 2.0f;

    int paddingLeft = 8;
    int paddingRight = 8;
    int paddingTop = 4;
    int paddingBottom = 4;

    int fontSize = 14;
    int cursorWidth = 1;
    float cursorBlinkRate = 1.0f; // Blinks per second

    int scrollMargin = 20; // Pixels from edge to start scrolling
  };

private:
  Style style;

  // Text state
  std::string textBuffer;
  std::string placeholderText;
  size_t maxLength = 0; // 0 = no limit

  // Cursor and selection
  int cursorPosition = 0;
  int selectionStart = 0;
  int selectionEnd = 0;
  bool isDragging = false;

  // Scrolling
  int scrollOffset = 0;

  // Cursor blinking
  std::chrono::steady_clock::time_point lastCursorBlink;
  bool cursorVisible = true;

  // Callbacks
  TextChangeCallback textChangeCallback;
  ValidationCallback validationCallback;

public:
  explicit TextInput(bombfork::prong::rendering::IRenderer* renderer = nullptr,
                     const std::string& debugName = "TextInput")
    : Component(renderer, debugName), lastCursorBlink(std::chrono::steady_clock::now()) {}

  virtual ~TextInput() = default;

  // === Text Management ===

  /**
   * @brief Set text content
   */
  void setText(const std::string& text) {
    if (validateText(text)) {
      textBuffer = text;
      cursorPosition = std::min(cursorPosition, static_cast<int>(textBuffer.length()));
      selectionStart = selectionEnd = cursorPosition;
      scrollOffset = 0;
      notifyTextChanged();
    }
  }

  /**
   * @brief Get current text content
   */
  const std::string& getText() const { return textBuffer; }

  /**
   * @brief Set placeholder text shown when input is empty
   */
  void setPlaceholder(const std::string& placeholder) { placeholderText = placeholder; }

  /**
   * @brief Get placeholder text
   */
  const std::string& getPlaceholder() const { return placeholderText; }

  /**
   * @brief Clear all text
   */
  void clear() {
    textBuffer.clear();
    cursorPosition = 0;
    selectionStart = selectionEnd = 0;
    scrollOffset = 0;
    notifyTextChanged();
  }

  // === Callback Management ===

  /**
   * @brief Set text change callback
   */
  void setOnTextChanged(TextChangeCallback callback) { textChangeCallback = std::move(callback); }

  /**
   * @brief Set text validation callback
   */
  void setValidator(ValidationCallback validator) { validationCallback = std::move(validator); }

  /**
   * @brief Set maximum text length (0 = no limit)
   */
  void setMaxLength(size_t maxLen) { maxLength = maxLen; }

  /**
   * @brief Get maximum text length
   */
  size_t getMaxLength() const { return maxLength; }

  // === Selection Management ===

  /**
   * @brief Set cursor position
   */
  void setCursorPosition(int position) {
    cursorPosition = std::max(0, std::min(position, static_cast<int>(textBuffer.length())));
    selectionStart = selectionEnd = cursorPosition;
    ensureCursorVisible();
  }

  /**
   * @brief Get current cursor position
   */
  int getCursorPosition() const { return cursorPosition; }

  /**
   * @brief Select all text
   */
  void selectAll() {
    selectionStart = 0;
    selectionEnd = static_cast<int>(textBuffer.length());
    cursorPosition = selectionEnd;
    ensureCursorVisible();
  }

  /**
   * @brief Clear selection
   */
  void clearSelection() { selectionStart = selectionEnd = cursorPosition; }

  /**
   * @brief Check if text is selected
   */
  bool hasSelection() const { return selectionStart != selectionEnd; }

  /**
   * @brief Get selected text
   */
  std::string getSelectedText() const {
    if (!hasSelection())
      return "";
    int start = std::min(selectionStart, selectionEnd);
    int end = std::max(selectionStart, selectionEnd);
    return textBuffer.substr(start, end - start);
  }

  // === Style Management ===

  /**
   * @brief Set input style
   */
  void setStyle(const Style& newStyle) { style = newStyle; }

  /**
   * @brief Get current style
   */
  const Style& getStyle() const { return style; }

  // === Focus Management ===

  bool canReceiveFocus() const override { return enabled && visible; }

  // === Event Handling ===

  bool handleClick(int localX, int /* localY */) override {
    if (!enabled)
      return false;

    // Calculate click position in text
    int clickPos = getTextPositionFromPoint(localX);
    cursorPosition = clickPos;
    selectionStart = selectionEnd = cursorPosition;

    // Reset cursor blink
    resetCursorBlink();

    return true;
  }

  bool handleMousePress(int localX, int /* localY */, int button) override {
    if (!enabled || button != 0)
      return false; // Left click only

    // Start selection
    int clickPos = getTextPositionFromPoint(localX);
    cursorPosition = selectionStart = selectionEnd = clickPos;
    isDragging = true;

    // Reset cursor blink
    resetCursorBlink();

    // Request focus
    requestFocus();

    return true;
  }

  bool handleMouseRelease(int /* localX */, int /* localY */, int button) override {
    if (button == 0) {
      isDragging = false;
      return true;
    }
    return false;
  }

  bool handleMouseMove(int localX, int /* localY */) override {
    if (!isDragging)
      return false;

    // Update selection end
    int movePos = getTextPositionFromPoint(localX);
    selectionEnd = movePos;
    cursorPosition = movePos;
    ensureCursorVisible();

    return true;
  }

  bool handleKey(int key, int action, int mods) override {
    if (!enabled || action == GLFW_RELEASE)
      return false;

    bool consumed = false;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;

    // Reset cursor blink
    resetCursorBlink();

    switch (key) {
    case GLFW_KEY_LEFT:
      handleCursorMove(-1, shift, ctrl);
      consumed = true;
      break;

    case GLFW_KEY_RIGHT:
      handleCursorMove(1, shift, ctrl);
      consumed = true;
      break;

    case GLFW_KEY_HOME:
      handleHome(shift);
      consumed = true;
      break;

    case GLFW_KEY_END:
      handleEnd(shift);
      consumed = true;
      break;

    case GLFW_KEY_BACKSPACE:
      handleBackspace();
      consumed = true;
      break;

    case GLFW_KEY_DELETE:
      handleDelete();
      consumed = true;
      break;

    case GLFW_KEY_A:
      if (ctrl) {
        selectAll();
        consumed = true;
      }
      break;

    case GLFW_KEY_C:
      if (ctrl && hasSelection()) {
        copyToClipboard();
        consumed = true;
      }
      break;

    case GLFW_KEY_V:
      if (ctrl) {
        pasteFromClipboard();
        consumed = true;
      }
      break;

    case GLFW_KEY_X:
      if (ctrl && hasSelection()) {
        copyToClipboard();
        deleteSelection();
        consumed = true;
      }
      break;

    case GLFW_KEY_ENTER:
    case GLFW_KEY_KP_ENTER:
      // Single-line input - ignore enter
      consumed = true;
      break;
    }

    return consumed;
  }

  bool handleChar(unsigned int codepoint) override {
    if (!enabled)
      return false;

    // Check if character is printable
    if (codepoint < 32 || codepoint == 127)
      return false;

    // Convert codepoint to UTF-8 string
    std::string character = codepointToUTF8(codepoint);
    insertText(character);

    // Reset cursor blink
    resetCursorBlink();

    return true;
  }

  // === Update ===

  void update(double deltaTime) override {
    // Update cursor blinking animation
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCursorBlink).count();
    double blinkInterval = 1000.0 / style.cursorBlinkRate / 2.0; // Half period for blink cycle

    if (elapsed >= blinkInterval) {
      cursorVisible = !cursorVisible;
      lastCursorBlink = now;
    }

    (void)deltaTime; // Not used - we use wall clock time for cursor blinking
  }

  // === Rendering ===

  void render() override {
    if (!visible || !renderer)
      return;

    bool focused = hasFocus();

    // Render background
    renderer->drawRect(x, y, width, height, style.backgroundColor.r, style.backgroundColor.g, style.backgroundColor.b,
                       style.backgroundColor.a);

    // Render border
    renderBorder(focused);

    // Calculate text area (inside padding)
    int textX = x + style.paddingLeft;
    int textY = y + style.paddingTop;
    // int textWidth = width - style.paddingLeft - style.paddingRight; // Currently unused
    int textHeight = height - style.paddingTop - style.paddingBottom;

    // Enable clipping for text area
    // Note: Clipping would need to be implemented in renderer
    // For now, we'll just be careful with rendering

    // Update cursor blink
    updateCursorBlink();

    // Render selection highlight
    if (hasSelection() && focused) {
      renderSelection(textX, textY, textHeight);
    }

    // Render text or placeholder
    if (!textBuffer.empty()) {
      renderText(textX, textY, textHeight, style.textColor);
    } else if (!placeholderText.empty()) {
      renderText(textX, textY, textHeight, style.placeholderColor, true);
    }

    // Render cursor
    if (focused && cursorVisible) {
      renderCursor(textX, textY, textHeight);
    }
  }

private:
  // === Helper Methods ===

  /**
   * @brief Validate text using validation callback
   */
  bool validateText(const std::string& text) const {
    if (maxLength > 0 && text.length() > maxLength)
      return false;
    if (validationCallback)
      return validationCallback(text);
    return true;
  }

  /**
   * @brief Notify text change callback
   */
  void notifyTextChanged() {
    if (textChangeCallback) {
      textChangeCallback(textBuffer);
    }
  }

  /**
   * @brief Insert text at cursor position
   */
  void insertText(const std::string& text) {
    // Check if inserting would exceed max length
    std::string newText = textBuffer;
    if (hasSelection()) {
      int start = std::min(selectionStart, selectionEnd);
      int end = std::max(selectionStart, selectionEnd);
      newText.erase(start, end - start);
      newText.insert(start, text);
    } else {
      newText.insert(cursorPosition, text);
    }

    if (!validateText(newText))
      return;

    deleteSelection(); // Remove selection if any

    textBuffer.insert(cursorPosition, text);
    cursorPosition += static_cast<int>(text.length());
    selectionStart = selectionEnd = cursorPosition;

    ensureCursorVisible();
    notifyTextChanged();
  }

  /**
   * @brief Delete current selection
   */
  void deleteSelection() {
    if (!hasSelection())
      return;

    int start = std::min(selectionStart, selectionEnd);
    int end = std::max(selectionStart, selectionEnd);

    textBuffer.erase(start, end - start);
    cursorPosition = selectionStart = selectionEnd = start;

    ensureCursorVisible();
    notifyTextChanged();
  }

  /**
   * @brief Handle cursor movement
   */
  void handleCursorMove(int direction, bool shift, bool ctrl) {
    int newPos = cursorPosition;

    if (ctrl) {
      // Word-based movement
      newPos = findWordBoundary(cursorPosition, direction > 0);
    } else {
      // Character-based movement
      newPos += direction;
      newPos = std::max(0, std::min(newPos, static_cast<int>(textBuffer.length())));
    }

    if (shift) {
      // Extend selection
      if (selectionStart == selectionEnd) {
        selectionStart = cursorPosition;
      }
      selectionEnd = newPos;
    } else {
      // Clear selection
      selectionStart = selectionEnd = newPos;
    }

    cursorPosition = newPos;
    ensureCursorVisible();
  }

  /**
   * @brief Handle Home key
   */
  void handleHome(bool shift) {
    int newPos = 0;

    if (shift) {
      if (selectionStart == selectionEnd) {
        selectionStart = cursorPosition;
      }
      selectionEnd = newPos;
    } else {
      selectionStart = selectionEnd = newPos;
    }

    cursorPosition = newPos;
    scrollOffset = 0;
  }

  /**
   * @brief Handle End key
   */
  void handleEnd(bool shift) {
    int newPos = static_cast<int>(textBuffer.length());

    if (shift) {
      if (selectionStart == selectionEnd) {
        selectionStart = cursorPosition;
      }
      selectionEnd = newPos;
    } else {
      selectionStart = selectionEnd = newPos;
    }

    cursorPosition = newPos;
    ensureCursorVisible();
  }

  /**
   * @brief Handle Backspace key
   */
  void handleBackspace() {
    if (hasSelection()) {
      deleteSelection();
    } else if (cursorPosition > 0) {
      textBuffer.erase(cursorPosition - 1, 1);
      cursorPosition--;
      selectionStart = selectionEnd = cursorPosition;
      ensureCursorVisible();
      notifyTextChanged();
    }
  }

  /**
   * @brief Handle Delete key
   */
  void handleDelete() {
    if (hasSelection()) {
      deleteSelection();
    } else if (cursorPosition < static_cast<int>(textBuffer.length())) {
      textBuffer.erase(cursorPosition, 1);
      ensureCursorVisible();
      notifyTextChanged();
    }
  }

  /**
   * @brief Copy selection to clipboard
   */
  void copyToClipboard() {
    if (!hasSelection())
      return;

    std::string selectedText = getSelectedText();
    if (!selectedText.empty()) {
      auto* window = glfwGetCurrentContext();
      if (window) {
        glfwSetClipboardString(window, selectedText.c_str());
      }
    }
  }

  /**
   * @brief Paste from clipboard
   */
  void pasteFromClipboard() {
    auto* window = glfwGetCurrentContext();
    if (!window)
      return;

    const char* clipboardText = glfwGetClipboardString(window);
    if (clipboardText) {
      insertText(std::string(clipboardText));
    }
  }

  /**
   * @brief Find word boundary for word-based navigation
   */
  int findWordBoundary(int pos, bool forward) const {
    if (forward) {
      // Find next word start
      while (pos < static_cast<int>(textBuffer.length()) && !std::isalnum(textBuffer[pos]))
        pos++;
      while (pos < static_cast<int>(textBuffer.length()) && std::isalnum(textBuffer[pos]))
        pos++;
    } else {
      // Find previous word start
      while (pos > 0 && !std::isalnum(textBuffer[pos - 1]))
        pos--;
      while (pos > 0 && std::isalnum(textBuffer[pos - 1]))
        pos--;
    }
    return pos;
  }

  /**
   * @brief Get text position from screen point
   */
  int getTextPositionFromPoint(int localX) const {
    // Note: localX is in absolute/global coordinates (despite the parameter name)
    // Convert to component-relative coordinates first
    int componentRelativeX = localX - x;
    int relativeX = componentRelativeX - style.paddingLeft + scrollOffset;

    // Estimate character width (rough approximation)
    float charWidth = style.fontSize * 0.6f;

    int position = std::max(0, static_cast<int>(relativeX / charWidth));
    return std::min(position, static_cast<int>(textBuffer.length()));
  }

  /**
   * @brief Ensure cursor is visible by adjusting scroll
   */
  void ensureCursorVisible() {
    int textAreaWidth = width - style.paddingLeft - style.paddingRight;

    // Estimate cursor position in pixels
    float charWidth = style.fontSize * 0.6f;
    int cursorX = static_cast<int>(cursorPosition * charWidth);

    // Adjust scroll to keep cursor visible
    if (cursorX - scrollOffset < style.scrollMargin) {
      scrollOffset = std::max(0, cursorX - style.scrollMargin);
    } else if (cursorX - scrollOffset > textAreaWidth - style.scrollMargin) {
      scrollOffset = cursorX - textAreaWidth + style.scrollMargin;
    }
  }

  /**
   * @brief Reset cursor blink timer
   */
  void resetCursorBlink() {
    lastCursorBlink = std::chrono::steady_clock::now();
    cursorVisible = true;
  }

  /**
   * @brief Update cursor blink state
   */
  void updateCursorBlink() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCursorBlink);

    float blinkInterval = 1000.0f / (2.0f * style.cursorBlinkRate);

    if (elapsed.count() > blinkInterval) {
      cursorVisible = !cursorVisible;
      lastCursorBlink = now;
    }
  }

  /**
   * @brief Render border
   */
  void renderBorder(bool focused) {
    if (!renderer)
      return;

    const auto& borderColor = focused ? style.focusBorderColor : style.borderColor;
    int borderWidth = focused ? static_cast<int>(style.focusBorderWidth) : static_cast<int>(style.borderWidth);

    // Top border
    renderer->drawRect(x, y, width, borderWidth, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Bottom border
    renderer->drawRect(x, y + height - borderWidth, width, borderWidth, borderColor.r, borderColor.g, borderColor.b,
                       borderColor.a);

    // Left border
    renderer->drawRect(x, y, borderWidth, height, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    // Right border
    renderer->drawRect(x + width - borderWidth, y, borderWidth, height, borderColor.r, borderColor.g, borderColor.b,
                       borderColor.a);
  }

  /**
   * @brief Render selection highlight
   */
  void renderSelection(int textX, int textY, int textHeight) {
    if (!hasSelection() || !renderer)
      return;

    int start = std::min(selectionStart, selectionEnd);
    int end = std::max(selectionStart, selectionEnd);

    // Estimate selection bounds
    float charWidth = style.fontSize * 0.6f;

    int startX = textX + static_cast<int>(start * charWidth) - scrollOffset;
    int endX = textX + static_cast<int>(end * charWidth) - scrollOffset;

    renderer->drawRect(startX, textY, endX - startX, textHeight, style.selectionColor.r, style.selectionColor.g,
                       style.selectionColor.b, style.selectionColor.a);
  }

  /**
   * @brief Render text content
   */
  void renderText(int textX, int textY, int textHeight, const theming::Color& color, bool isPlaceholder = false) {
    if (!renderer)
      return;

    const std::string& text = isPlaceholder ? placeholderText : textBuffer;

    // Measure actual text dimensions for proper vertical centering
    auto [textWidth, measuredTextHeight] = renderer->measureText(text.empty() ? "A" : text);

    float posX = static_cast<float>(textX - scrollOffset);
    float posY = static_cast<float>(textY + (textHeight - measuredTextHeight) / 2);

    renderer->drawText(text, static_cast<int>(posX), static_cast<int>(posY), color.r, color.g, color.b, color.a);
  }

  /**
   * @brief Render cursor
   */
  void renderCursor(int textX, int textY, int textHeight) {
    if (!renderer)
      return;

    // Estimate cursor position
    float charWidth = style.fontSize * 0.6f;
    float cursorX = textX + cursorPosition * charWidth - scrollOffset;

    renderer->drawRect(static_cast<int>(cursorX), textY + 2, style.cursorWidth, textHeight - 4, style.cursorColor.r,
                       style.cursorColor.g, style.cursorColor.b, style.cursorColor.a);
  }

  /**
   * @brief Convert Unicode codepoint to UTF-8 string
   */
  static std::string codepointToUTF8(unsigned int codepoint) {
    std::string result;

    if (codepoint <= 0x7F) {
      result += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
      result += static_cast<char>(0xC0 | (codepoint >> 6));
      result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
      result += static_cast<char>(0xE0 | (codepoint >> 12));
      result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
      result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0x10FFFF) {
      result += static_cast<char>(0xF0 | (codepoint >> 18));
      result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
      result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
      result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }

    return result;
  }
};

} // namespace bombfork::prong
