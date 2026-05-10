#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace typ {

// Single Producer Single Consumer Queue
template <std::size_t Capacity, typename T>
class SpscQueue {
  static_assert(Capacity > 1, "SpscQueue Capacity must be greater than 1");

  static_assert(
    Capacity % 2 == 0,
    "SpscQueue Capacity must be a power of 2 for optimal performance");

 public:
  using ValueType = T;


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

  [[nodiscard]] bool pop(T& out) noexcept {
    const std::size_t tail = tail_.load(std::memory_order_relaxed);

    if (tail == head_.load(std::memory_order_acquire)) {
      return false;  // queue empty
    }

    out = buffer_[tail];

    tail_.store(increment(tail), std::memory_order_release);

    return true;
  }

  [[nodiscard]] const T& peakFront() const noexcept {
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    return buffer_[tail];
  }

  [[nodiscard]] bool empty() const noexcept {
    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
  }

  [[nodiscard]] bool full() const noexcept {
    const std::size_t head = head_.load(std::memory_order_acquire);
    const std::size_t next = increment(head);

    return next == tail_.load(std::memory_order_acquire);
  }

  [[nodiscard]] std::size_t size() const noexcept {
    const std::size_t head = head_.load(std::memory_order_acquire);
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    return (head >= tail) ? (head - tail) : (Capacity - tail + head);
  }

  [[nodiscard]] std::size_t capacity() const noexcept { return Capacity - 1; }

 private:
  [[nodiscard]] static constexpr std::size_t increment(const std::size_t index) noexcept {
    // If capacity is a power of two then the following equals (index + 1) % Capacity but is much faster.
    return (index + 1U) & (Capacity - 1U);
  }

  alignas(64) std::array<T, Capacity> buffer_{};

  alignas(64) std::atomic<std::size_t> head_{0};

  alignas(64) std::atomic<std::size_t> tail_{0};
};

};  // namespace typ
