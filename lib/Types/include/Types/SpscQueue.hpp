/**
 * @file SpscQueue.hpp
 * @brief Lightweight single-producer single-consumer ring buffer queue.
 *
 * A wait-free SPSC queue implementation that uses a power-of-two capacity
 * and atomic indices for lockless producer/consumer access. This queue
 * sacrifices one slot to differentiate full vs empty states so the
 * effective usable capacity is `Capacity - 1`.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */

#pragma once

#include <array>
#include <atomic>
#include <cstdint>

/**
 * @file SpscQueue.hpp
 * @brief Lightweight single-producer single-consumer ring buffer queue.
 *
 * A wait-free SPSC queue implementation that uses a power-of-two capacity
 * and atomic indices for lockless producer/consumer access. This queue
 * sacrifices one slot to differentiate full vs empty states so the
 * effective usable capacity is `Capacity - 1`.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */

#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace typ {

/**
 * @brief Single Producer Single Consumer Queue.
 *
 * Template parameters:
 * @tparam Capacity Size of the internal ring buffer. Must be a power of two
 *                  and greater than 1. The usable capacity is `Capacity - 1`.
 * @tparam T        Type of elements stored in the queue.
 *
 * Notes:
 * - Uses atomics with relaxed/acquire/release memory orders for efficiency.
 * - One slot is reserved to disambiguate full vs empty states.
 */
template <std::size_t Capacity, typename T>
class SpscQueue {
  static_assert(Capacity > 1, "SpscQueue Capacity must be greater than 1");

  static_assert(
    Capacity % 2 == 0,
    "SpscQueue Capacity must be a power of 2 for optimal performance");

 public:
  /**
   * @brief Alias for the stored value type.
   */
  using ValueType = T;

  /**
   * @brief Push a value into the queue (producer side).
   *
   * Attempts to enqueue a copy of `value`. This method is safe for a single
   * producer when paired with a single consumer calling `pop` concurrently.
   *
   * @param value Value to push into the queue.
   * @return true  if the value was enqueued successfully.
   * @return false if the queue is full and the value could not be enqueued.
   */
  [[nodiscard]] bool push(const T& value) noexcept {
    const std::size_t head = head_.load(std::memory_order_relaxed);
    const std::size_t next = increment(head);

    if (next == tail_.load(std::memory_order_acquire)) {
      return false;  // queue full
    }

    buffer_[head] = value;

    head_.store(next, std::memory_order_release);

    return true;
  }

  /**
   * @brief Pop a value from the queue (consumer side).
   *
   * Attempts to dequeue the front value into `out`.
   *
   * @param[out] out Reference where the dequeued value will be stored on success.
   * @return true  if a value was dequeued and written to `out`.
   * @return false if the queue was empty.
   */
  [[nodiscard]] bool pop(T& out) noexcept {
    const std::size_t tail = tail_.load(std::memory_order_relaxed);

    if (tail == head_.load(std::memory_order_acquire)) {
      return false;  // queue empty
    }

    out = buffer_[tail];

    tail_.store(increment(tail), std::memory_order_release);

    return true;
  }

  /**
   * @brief Access the element at the front of the queue without removing it.
   *
   * The returned reference becomes invalid if a concurrent `pop` removes
   * the element. Use only when caller can guarantee the element won't be
   * removed concurrently or when observing for read-only purposes.
   *
   * @return const T& Reference to the front element.
   */
  [[nodiscard]] const T& peakFront() const noexcept {
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    return buffer_[tail];
  }

  /**
   * @brief Check whether the queue is empty.
   *
   * @return true  if the queue contains no elements.
   * @return false otherwise.
   */
  [[nodiscard]] bool empty() const noexcept {
    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
  }

  /**
   * @brief Check whether the queue is full.
   *
   * @return true  if the queue cannot accept new elements.
   * @return false otherwise.
   */
  [[nodiscard]] bool full() const noexcept {
    const std::size_t head = head_.load(std::memory_order_acquire);
    const std::size_t next = increment(head);

    return next == tail_.load(std::memory_order_acquire);
  }

  /**
   * @brief Current number of stored elements.
   *
   * Computes size using atomic head/tail indices. This is a snapshot and
   * may change immediately in concurrent contexts.
   *
   * @return std::size_t Number of elements currently in the queue.
   */
  [[nodiscard]] std::size_t size() const noexcept {
    const std::size_t head = head_.load(std::memory_order_acquire);
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    return (head >= tail) ? (head - tail) : (Capacity - tail + head);
  }

  /**
   * @brief Usable capacity of the queue.
   *
   * The implementation reserves one slot to distinguish full/empty states.
   *
   * @return std::size_t Maximum number of elements that can be stored.
   */
  [[nodiscard]] std::size_t capacity() const noexcept { return Capacity - 1; }

 private:
  /**
   * @brief Increment index in a wrap-around (power-of-two optimized) manner.
   *
   * If Capacity is a power of two this is equivalent to `(index + 1) %
   * Capacity` but faster due to bitmasking.
   *
   * @param index Current index to increment.
   * @return std::size_t Next index (wrapped).
   */
  [[nodiscard]] static constexpr std::size_t increment(const std::size_t index) noexcept {
    // If capacity is a power of two then the following equals (index + 1) % Capacity but is much faster.
    return (index + 1U) & (Capacity - 1U);
  }

  /**
   * @brief Internal circular storage buffer.
   */
  alignas(64) std::array<T, Capacity> buffer_{};

  /**
   * @brief Producer index (head). Points to next write position.
   */
  alignas(64) std::atomic<std::size_t> head_{0};

  /**
   * @brief Consumer index (tail). Points to next read position.
   */
  alignas(64) std::atomic<std::size_t> tail_{0};
};

}  // namespace typ
