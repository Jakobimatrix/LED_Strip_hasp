/**
 * @file SpscQueue.tpp
 * @brief Implementation of SpscQueue template methods.
 */

#include <utility>

namespace typ {

template <std::size_t Capacity, typename T>
[[nodiscard]] bool SpscQueue<Capacity, T>::push(const T& value) noexcept {
  const std::size_t head = head_.load(std::memory_order_relaxed);
  const std::size_t next = increment(head);

  if (next == tail_.load(std::memory_order_acquire)) {
    ++num_dropped_messages_;
    return false;  // queue full
  }

  buffer_[head] = value;

  head_.store(next, std::memory_order_release);

  return true;
}

template <std::size_t Capacity, typename T>
[[nodiscard]] bool SpscQueue<Capacity, T>::pop(T& out) noexcept {
  const std::size_t tail = tail_.load(std::memory_order_relaxed);

  if (tail == head_.load(std::memory_order_acquire)) {
    return false;  // queue empty
  }

  out = buffer_[tail];

  tail_.store(increment(tail), std::memory_order_release);

  return true;
}

template <std::size_t Capacity, typename T>
[[nodiscard]] const T& SpscQueue<Capacity, T>::peakFront() const noexcept {
  const std::size_t tail = tail_.load(std::memory_order_acquire);
  return buffer_[tail];
}

template <std::size_t Capacity, typename T>
[[nodiscard]] bool SpscQueue<Capacity, T>::empty() const noexcept {
  return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
}

template <std::size_t Capacity, typename T>
[[nodiscard]] bool SpscQueue<Capacity, T>::full() const noexcept {
  const std::size_t head = head_.load(std::memory_order_acquire);
  const std::size_t next = increment(head);

  return next == tail_.load(std::memory_order_acquire);
}

template <std::size_t Capacity, typename T>
[[nodiscard]] std::size_t SpscQueue<Capacity, T>::size() const noexcept {
  const std::size_t head = head_.load(std::memory_order_acquire);
  const std::size_t tail = tail_.load(std::memory_order_acquire);
  return (head >= tail) ? (head - tail) : (Capacity - tail + head);
}

template <std::size_t Capacity, typename T>
[[nodiscard]] constexpr std::size_t SpscQueue<Capacity, T>::increment(const std::size_t index) noexcept {
  return (index + 1U) & (Capacity - 1U);
}

}  // namespace typ
