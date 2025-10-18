#pragma once

#include <string>

namespace bombfork::prong::events {

/**
 * @brief Window-agnostic clipboard interface for text operations.
 *
 * This interface provides a platform-independent abstraction for clipboard operations,
 * allowing the framework to work with any windowing system (GLFW, SDL, native APIs, etc.)
 * without direct dependencies.
 *
 * Implementations should handle platform-specific clipboard access and provide
 * graceful fallbacks when clipboard operations are unavailable.
 *
 * @note All methods should be thread-safe if the underlying platform requires it.
 */
class IClipboard {
public:
  /**
   * @brief Virtual destructor for proper cleanup of derived implementations.
   */
  virtual ~IClipboard() = default;

  /**
   * @brief Retrieve text content from the system clipboard.
   *
   * This method reads the current text content from the clipboard. If the clipboard
   * is empty, contains non-text data, or clipboard access fails, an empty string
   * should be returned.
   *
   * @return The text content from the clipboard, or an empty string if unavailable.
   *
   * @note Implementations should handle errors gracefully and never throw exceptions
   *       from this method. Return an empty string on any failure.
   */
  virtual std::string getString() const = 0;

  /**
   * @brief Set text content to the system clipboard.
   *
   * This method writes the provided text to the system clipboard, replacing any
   * existing content. If clipboard access fails, implementations should fail
   * silently without throwing exceptions.
   *
   * @param text The text content to write to the clipboard.
   *
   * @note Implementations should handle errors gracefully. If the operation fails,
   *       the method should return without throwing exceptions.
   * @note The text parameter is passed by const reference to avoid unnecessary copies
   *       for large strings.
   */
  virtual void setString(const std::string& text) = 0;

  /**
   * @brief Check if the clipboard contains text content.
   *
   * This method determines whether the clipboard currently contains text data
   * that can be retrieved via getString(). It should return false if the clipboard
   * is empty, contains non-text data, or clipboard access fails.
   *
   * @return true if text content is available in the clipboard, false otherwise.
   *
   * @note This method provides a quick way to check clipboard state without
   *       retrieving the actual content, which may be more efficient for large
   *       clipboard contents.
   * @note Implementations should handle errors gracefully and return false on failure.
   */
  virtual bool hasText() const = 0;
};

} // namespace bombfork::prong::events
