#include <bombfork/prong/core/async_callback_queue.h>

#include <algorithm>
#include <exception>
#include <iostream>

namespace bombfork::prong::core {

void AsyncCallbackQueue::queueCallback(Callback callback, const std::string& debugName, int priority) {
  if (!callback) {
    std::cerr << "AsyncCallbackQueue: Attempted to queue null callback" << std::endl;
    return;
  }

  std::lock_guard<std::mutex> lock(queueMutex);

  callbackQueue.emplace(std::move(callback), debugName, priority);
  totalQueued++;
  currentQueueSize = callbackQueue.size();

  if (!debugName.empty()) {
    std::cout << "AsyncCallbackQueue: Queued callback '" << debugName << "' (priority: " << priority
              << ", queue size: " << currentQueueSize << ")" << std::endl;
  }
}

size_t AsyncCallbackQueue::processCallbacks(size_t maxCallbacks) {
  std::queue<QueuedCallback> callbacks;
  size_t processCount = 0;

  // Move callbacks to local queue to minimize lock time
  {
    std::lock_guard<std::mutex> lock(queueMutex);

    size_t toProcess = maxCallbacks == 0 ? callbackQueue.size() : std::min(maxCallbacks, callbackQueue.size());

    while (!callbackQueue.empty() && processCount < toProcess) {
      callbacks.push(callbackQueue.top());
      callbackQueue.pop();
      processCount++;
    }

    currentQueueSize = callbackQueue.size();
  }

  // Process callbacks outside the lock
  auto startTime = std::chrono::steady_clock::now();
  size_t actuallyProcessed = 0;

  while (!callbacks.empty()) {
    const auto& queuedCallback = callbacks.front();

    try {
      if (!queuedCallback.debugName.empty()) {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - queuedCallback.queueTime).count();
        std::cout << "AsyncCallbackQueue: Executing callback '" << queuedCallback.debugName << "' (age: " << age
                  << "ms)" << std::endl;
      }

      queuedCallback.callback();
      actuallyProcessed++;

    } catch (const std::exception& e) {
      std::cerr << "AsyncCallbackQueue: Exception in callback '" << queuedCallback.debugName << "': " << e.what()
                << std::endl;
    } catch (...) {
      std::cerr << "AsyncCallbackQueue: Unknown exception in callback '" << queuedCallback.debugName << "'"
                << std::endl;
    }

    callbacks.pop();
  }

  // Update statistics
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    totalProcessed += actuallyProcessed;
  }

  return actuallyProcessed;
}

size_t AsyncCallbackQueue::getQueueSize() const {
  std::lock_guard<std::mutex> lock(queueMutex);
  return callbackQueue.size();
}

bool AsyncCallbackQueue::isEmpty() const {
  std::lock_guard<std::mutex> lock(queueMutex);
  return callbackQueue.empty();
}

void AsyncCallbackQueue::clear() {
  std::lock_guard<std::mutex> lock(queueMutex);

  size_t clearedCount = callbackQueue.size();
  while (!callbackQueue.empty()) {
    callbackQueue.pop();
  }

  currentQueueSize = 0;

  if (clearedCount > 0) {
    std::cout << "AsyncCallbackQueue: Cleared " << clearedCount << " callbacks" << std::endl;
  }
}

AsyncCallbackQueue::Statistics AsyncCallbackQueue::getStatistics() const {
  std::lock_guard<std::mutex> lock(queueMutex);

  Statistics stats;
  stats.totalQueued = totalQueued;
  stats.totalProcessed = totalProcessed;
  stats.currentSize = callbackQueue.size();

  return stats;
}

size_t AsyncCallbackQueue::removeExpiredCallbacks(int maxAgeSeconds) {
  std::lock_guard<std::mutex> lock(queueMutex);

  if (callbackQueue.empty()) {
    return 0;
  }

  auto now = std::chrono::steady_clock::now();
  auto maxAge = std::chrono::seconds(maxAgeSeconds);

  // Create a new queue with only non-expired callbacks
  std::priority_queue<QueuedCallback> newQueue;
  size_t removedCount = 0;

  while (!callbackQueue.empty()) {
    const auto& callback = callbackQueue.top();

    if (now - callback.queueTime <= maxAge) {
      newQueue.push(callback);
    } else {
      removedCount++;
      if (!callback.debugName.empty()) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - callback.queueTime).count();
        std::cout << "AsyncCallbackQueue: Removed expired callback '" << callback.debugName << "' (age: " << age << "s)"
                  << std::endl;
      }
    }

    callbackQueue.pop();
  }

  callbackQueue = std::move(newQueue);
  currentQueueSize = callbackQueue.size();

  if (removedCount > 0) {
    std::cout << "AsyncCallbackQueue: Removed " << removedCount << " expired callbacks (remaining: " << currentQueueSize
              << ")" << std::endl;
  }

  return removedCount;
}

void AsyncCallbackQueue::updateStatistics() {
  currentQueueSize = callbackQueue.size();
}

} // namespace bombfork::prong::core