#pragma once

#include <bombfork/prong/events/ikeyboard.h>

namespace bombfork::prong::tests {

/**
 * @brief Mock keyboard implementation for testing
 *
 * This mock provides a simple 1:1 mapping between platform keys
 * and Prong keys for unit tests, allowing tests to verify keyboard
 * interactions without requiring a real windowing system.
 */
class MockKeyboard : public events::IKeyboard {
public:
  /**
   * @brief Convert platform-specific key code to Prong key code
   *
   * In this mock, we use a simple 1:1 mapping where the platform key
   * is cast directly to the Prong Key enum. This works for tests where
   * we control the input values.
   *
   * @param platformKey Platform-specific key code (in tests, use static_cast<int>(Key))
   * @return Prong Key enum value
   */
  events::Key toProngKey(int platformKey) const override { return static_cast<events::Key>(platformKey); }

  /**
   * @brief Convert Prong key code to platform-specific key code
   *
   * In this mock, we use a simple 1:1 mapping where the Prong key
   * is cast directly to an integer.
   *
   * @param key Prong Key enum value
   * @return Platform-specific key code
   */
  int fromProngKey(events::Key key) const override { return static_cast<int>(key); }
};

} // namespace bombfork::prong::tests
