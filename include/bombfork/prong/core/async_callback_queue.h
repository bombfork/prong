#pragma once

#include <chrono>
#include <compare>
#include <functional>
#include <mutex>
#include <queue>
#include <stddef.h>
#include <string>
#include <utility>

namespace bombfork::prong::core {

/**
 * @brief Thread-safe callback queue for async operations
 *
 * Manages callbacks from async operations (like texture loading, map loading)
 * that need to be executed on the main UI thread.
 */
class AsyncCallbackQueue {
public:
  using Callback = std::function<void()>;

  struct QueuedCallback {
    Callback callback;
    std::chrono::steady_clock::time_point queueTime;
    std::string debugName;
    int priority = 0; // Higher priority executed first

    QueuedCallback(Callback cb, const std::string& name = "", int prio = 0)
      : callback(std::move(cb)), queueTime(std::chrono::steady_clock::now()), debugName(name), priority(prio) {}

    // Priority comparison for priority queue
    bool operator<(const QueuedCallback& other) const {
      if (priority != other.priority) {
        return priority < other.priority; // priority_queue is max-heap
      }
      return queueTime > other.queueTime; // Earlier callbacks first
    }
  };

private:
  std::priority_queue<QueuedCallback> callbackQueue;
  mutable std::mutex queueMutex;

  // Statistics
  size_t totalQueued = 0;
  size_t totalProcessed = 0;
  size_t currentQueueSize = 0;

public:
  AsyncCallbackQueue() = default;
  ~AsyncCallbackQueue() = default;

  // Non-copyable, non-movable
  AsyncCallbackQueue(const AsyncCallbackQueue&) = delete;
  AsyncCallbackQueue& operator=(const AsyncCallbackQueue&) = delete;

  /**
   * @brief Queue a callback for execution on the main thread
   *
   * @param callback Function to execute
   * @param debugName Optional name for debugging
   * @param priority Higher priority callbacks execute first (default: 0)
   */
  void queueCallback(Callback callback, const std::string& debugName = "", int priority = 0);

  /**
   * @brief Process all queued callbacks (call from main thread)
   *
   * @param maxCallbacks Maximum number of callbacks to process per call (0 = all)
   * @return Number of callbacks processed
   */
  size_t processCallbacks(size_t maxCallbacks = 0);

  /**
   * @brief Get number of callbacks currently in queue
   */
  size_t getQueueSize() const;

  /**
   * @brief Check if queue is empty
   */
  bool isEmpty() const;

  /**
   * @brief Clear all queued callbacks
   */
  void clear();

  /**
   * @brief Get queue statistics
   */
  struct Statistics {
    size_t totalQueued;
    size_t totalProcessed;
    size_t currentSize;
    double averageProcessingTime = 0.0; // milliseconds
  };

  Statistics getStatistics() const;

  /**
   * @brief Remove old callbacks that haven't been processed
   *
   * @param maxAgeSeconds Maximum age in seconds before callback is discarded
   * @return Number of callbacks removed
   */
  size_t removeExpiredCallbacks(int maxAgeSeconds = 30);

private:
  void updateStatistics();
};

} // namespace bombfork::prong::core