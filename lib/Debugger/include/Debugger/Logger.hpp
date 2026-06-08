/**
 * @file Logger.hpp
 * @brief Lightweight, queue-backed logging helper and argument serialization.
 *
 * This header provides a small, constexpr-friendly logging facility that
 * serializes variadic arguments into a fixed-size buffer and pushes log
 * messages into a user-supplied queue type.
 *
 * The `write_arg` template encodes a single argument into the provided
 * byte buffer using the project's `ArgTraits`/`ArgType` machinery. The
 * `Logger<Queue>` class formats a message header (level, topic,
 * timestamp) and serializes an arbitrary number of arguments before
 * attempting to push the message into the provided queue instance.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Arduino.h>
#include <cstring>
#include <cstdio>

#include <Debugger/Constants.hpp>
#include <Debugger/Types.hpp>
#include <Enums/ExpandEnums.hpp>


namespace glob {
/**
 * @brief Global runtime log configuration.
 *
 * These globals control which messages are emitted by `Logger` instances.
 */
extern dbg::LEVEL LogLevel;     /**< Global minimum log level filter. */
extern dbg::TOPIC LogTopicMask; /**< Global topic bitmask filter. */
}  // namespace glob

namespace dbg {


/**
 * @brief Serialize a single argument into a buffer.
 *
 * The function writes a one-byte `ArgType` discriminator followed by the
 * argument payload into the buffer pointed to by `ptr`. For C-strings the
 * serialized layout is: `ArgType` (1 byte) + `len` (uint16_t) + `data` (len
 * bytes). For other POD-like types the layout is: `ArgType` (1 byte) + raw
 * bytes of `T` (sizeof(T)). The function advances `ptr` to the first free
 * byte after the written payload.
 *
 * @tparam T Type of the argument to serialize. Must satisfy
 *           `SerializableArg<T>` (see `Debugger/Types.hpp`).
 * @param ptr[in,out] Pointer into the buffer where serialization begins; is
 *                    advanced on success.
 * @param start[in]   Pointer to the start of the buffer used to compute
 *                    the already-used size (used for overflow checks).
 * @param size[in]    Total capacity of the buffer (in bytes).
 * @param value[in]   The argument value to serialize.
 * @return `true` when the argument was successfully written; `false` if
 *         there was insufficient space (buffer overflow would occur).
 */
template <typename T>
  requires SerializableArg<T>
constexpr bool write_arg(uint8_t*& ptr, const uint8_t* start, size_t size, const T& value) noexcept;


/**
 * @brief Queue-backed logger.
 *
 * The `Logger` serializes messages into the queue's `ValueType` and calls
 * `queue_.push(...)` to submit them. The queue type must expose a
 * `ValueType` with members used below (`level`, `topic`, `timestamp`, and
 * `buffer`) and provide `push(ValueType)` returning `bool`.
 *
 * @tparam Queue A queue-like type used to transport `ValueType` messages.
 *               Must provide `ValueType`, `push(...)`, and a fixed-size
 *               buffer API used by this logger.
 */
template <typename Queue>
class Logger {
 public:
  /**
   * @brief Construct a logger that pushes into `q`.
   * @param q Reference to the queue instance where messages are pushed.
   */
  explicit Logger(Queue& q);

  /**
   * @brief Emit a log message with variadic, serializable arguments.
   *
   * The method performs three main steps:
   * 1. Applies global filters (`glob::LogLevel` and `glob::LogTopicMask`).
   * 2. Fills a `ValueType` message: level, topic, timestamp, and serialized
   *    argument payload.
   * 3. Attempts to push the message into the configured queue.
   *
   * If the serialized arguments exceed the message buffer capacity the call
   * records the partial write (the `ok` flag becomes `false`) but still
   * attempts to push whatever fits. The method does not throw and is marked
   * `noexcept` for use in constrained runtime contexts.
   *
   * @tparam Args Types of the variadic arguments; each must satisfy
   *              `SerializableArg<Arg>`.
   * @param lvl   Severity level for this message (see `dbg::LEVEL`).
   * @param topic Logical topic bit(s) for this message (see `dbg::TOPIC`).
   * @param args  Variadic list of arguments to serialize into the message
   *              buffer.
   */
  template <typename... Args>
  constexpr void log(LEVEL lvl, TOPIC topic, const Args&... args) noexcept;

 private:
  Queue& queue_; /**< Reference to the underlying queue used to submit messages. */
};

}  // namespace dbg

#include "Logger.tpp"
