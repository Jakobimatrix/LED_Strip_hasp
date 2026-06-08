/**
 * @file Message.hpp
 * @brief Small, efficient message container types used by the debug logger.
 *
 * This header defines two lightweight, constexpr-friendly types used to
 * hold debug/log message payloads and their metadata: a fixed-size
 * `MessageBuffer` and a `DebugMessage` wrapper that stores level, topic
 * and a timestamp alongside the payload buffer.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Types/StaticString.hpp>

#include <array>
#include <cstdint>

/**
 * @namespace dbg
 * @brief Debugging and logging utilities and constants used project-wide.
 */
namespace dbg {



/**
 * @struct MessageBuffer
 * @brief Fixed-size, stack-allocated byte buffer for message payloads.
 *
 * Template parameter `N` selects the compile-time capacity. The type
 * exposes a trivially-copyable `std::array` storage and a `size` field
 * tracking the number of bytes currently used.
 *
 * @tparam N Capacity of the buffer in bytes.
 */
template <std::size_t N>
struct MessageBuffer {
  /** @brief Underlying storage for the buffer (capacity = `N`). */
  std::array<uint8_t, N> data{};

  /** @brief Number of valid bytes currently stored in `data`. */
  std::size_t size = 0;

  /**
   * @brief Obtain a pointer to the first byte of the buffer.
   * @return Pointer to the buffer's data (non-const, may be mutated).
   */
  constexpr uint8_t* ptr() noexcept { return data.data(); }

  /**
   * @brief Get the compile-time capacity of the buffer.
   * @return Maximum number of bytes the buffer can hold (equal to `N`).
   */
  constexpr size_t capacity() const noexcept { return N; }
};

/**
 * @struct DebugMessage
 * @brief Message container combining metadata and a `MessageBuffer` payload.
 *
 * The template parameter `N` configures the size of the embedded
 * `MessageBuffer`. Instances are intended to be lightweight POD-like
 * objects passed through logging and debug interfaces.
 *
 * @tparam N Size of the internal `MessageBuffer` in bytes.
 */
template <std::size_t N>
struct DebugMessage {
  /** @brief Convenience alias for the configured message buffer size. */
  static constexpr std::size_t MSG_BUFFER_SIZE = N;

  /** @brief Severity level for this message (see `dbg::LEVEL`). */
  LEVEL level;

  /** @brief Logical subsystem/topic for this message (see `dbg::TOPIC`). */
  TOPIC topic;

  /** @brief Timestamp for the message (signed 64-bit, units depend on caller).
   *
   * Typically represents a monotonic time value (for example microseconds
   * since boot) used by the logger for sorting/displaying message times.
   */
  int64_t timestamp;

  /** @brief Payload storage for the message. */
  MessageBuffer<MSG_BUFFER_SIZE> buffer{};
};

}  // namespace dbg
