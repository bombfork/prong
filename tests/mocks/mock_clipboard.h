#pragma once

#include <bombfork/prong/events/iclipboard.h>

#include <string>

namespace bombfork::prong::tests {

/**
 * @brief Mock clipboard implementation for testing
 *
 * This mock provides a simple in-memory clipboard for unit tests,
 * allowing tests to verify copy/paste operations without requiring
 * a real windowing system.
 */
class MockClipboard : public events::IClipboard {
private:
  std::string content;

public:
  /**
   * @brief Retrieve text content from the mock clipboard
   * @return The current clipboard content
   */
  std::string getString() const override { return content; }

  /**
   * @brief Set text content to the mock clipboard
   * @param text The text to store in the clipboard
   */
  void setString(const std::string& text) override { content = text; }

  /**
   * @brief Check if the clipboard contains text
   * @return true if clipboard is not empty, false otherwise
   */
  bool hasText() const override { return !content.empty(); }

  /**
   * @brief Clear the clipboard (test utility)
   */
  void clear() { content.clear(); }
};

} // namespace bombfork::prong::tests
