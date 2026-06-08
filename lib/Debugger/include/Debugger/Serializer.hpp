/**
 * @file Serializer.hpp
 * @brief Runtime helpers and a multi-queue `Serializer` for debug messages.
 *
 * This header provides small utility functions used by the debug system
 * (lookup helpers, a compact timestamp formatter and tuple visitation),
 * and the `Serializer` template which aggregates one or more message
 * queues and prints their oldest messages in-order.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Debugger/Constants.hpp>
#include <Debugger/Message.hpp>
#include <Debugger/Types.hpp>
#include <Types/StaticString.hpp>

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * @namespace dbg
 * @brief Debugging and logging utilities used across the project.
 */
namespace dbg {

/**
 * @brief Lookup a textual representation for an enum-like key in a map.
 *
 * Searches `map` for `key` and returns the mapped C-string if found.
 * If the key is not present the function returns the literal "[?]".
 *
 * @tparam MapT Map type supporting `find()` and `end()` (e.g. std::map).
 * @tparam Key Key type used for lookup.
 * @param map Map to search in.
 * @param key Key to locate in the map.
 * @return Pointer to a NUL-terminated C-string describing `key` or "[?]".
 */
template <typename MapT, typename Key>
constexpr const char* lookup(const MapT& map, const Key& key);


/**
 * @brief Convert a microsecond timestamp into a compact uptime string.
 *
 * `timestamp` is expected to be a monotonic time value in microseconds
 * (for example from `esp_timer_get_time()`). The function formats it as
 * `dddd/HH:mm:SS:ms` where `dddd` are days (zero-padded, capped at 9999).
 *
 * @param timestamp Monotonic timestamp in microseconds.
 * @return `typ::StaticString<18>` containing a NUL-terminated formatted
 *         uptime string.
 */
typ::StaticString<18> timestamp2string(int64_t timestamp);


template <typename Tuple, typename Func, std::size_t... Is>
void tuple_runtime_visit_impl(Tuple&& tuple, size_t index, Func&& func, std::index_sequence<Is...>);

/**
 * @brief Visit an element of a heterogenous `std::tuple` at runtime.
 *
 * This helper invokes `func(element)` where `element` is the tuple item
 * at position `index`. The call is dispatched using an index sequence
 * generated at compile-time while the selection occurs at runtime.
 *
 * @tparam Tuple A tuple-like type.
 * @tparam Func Callable accepting the selected tuple element.
 * @param tuple Tuple to index into.
 * @param index Runtime index of the element to visit (0..N-1).
 * @param func Callable invoked with the selected element.
 */
template <typename Tuple, typename Func>
void tuple_runtime_visit(Tuple&& tuple, size_t index, Func&& func);

/**
 * @brief Multi-queue debug message serializer and printer.
 *
 * The `Serializer` class aggregates references to multiple message queues and
 * provides a method to print the messages across all queues in order of
 * their timestamps.
 *
 * @tparam Queues Variadic list of queue types (must support `empty()`, `peakFront()`, `pop()`, `size()` and `capacity()`).
 */
template <typename... Queues>
class Serializer {

  std::tuple<Queues&...> queues_;

  constexpr static std::size_t N = sizeof...(Queues);

  /**
   * @brief Print a single debug message to stdout using formatted fields.
   *
   * The function decodes the compact argument buffer contained in `msg` and
   * prints a human readable line including level, topic and a formatted
   * timestamp. Supported argument types are handled via a small switch on
   * `ArgType` values.
   *
   * @tparam DebugMessageType Type of the message container (expects fields
   *         `level`, `topic`, `timestamp` and `buffer`).
   * @param msg Message instance to print.
   */
  template <typename DebugMessageType>
  void print(const DebugMessageType& msg) const;

  /**
   * @brief Find which queue currently contains the oldest (earliest)
   *        message timestamp.
   *
   * @return Index of the queue containing the oldest message, or
   *         `std::numeric_limits<std::size_t>::max()` when none are present.
   */
  std::size_t getQueueWithOldestMessage() const;

 public:
  /**
   * @brief Construct a `Serializer` aggregating references to message queues.
   *
   * The serializer stores references to the provided queues and will use them
   * when selecting and printing the next available debug message.
   *
   * @param qs References to queue-like objects (must provide `empty()`,
   *           `peakFront()`, `pop()`, `size()` and `capacity()`).
   */
  explicit Serializer(Queues&... qs);


  void debugQueueLoads() const;

  /**
   * @brief Print a diagnostic warning if a message's internal buffer is
   *        nearly full.
   *
   * @tparam DebugMessageType Type exposing `buffer.size` and
   *         `buffer.capacity()`.
   * @param msg Message to inspect.
   */
  template <typename DebugMessageType>
  void debugMessageBufferSizeUsage(const DebugMessageType& msg) const;

  /**
   * @brief Pop and print the oldest available message across all queues.
   *
   * @return `true` if a message was printed; `false` if all queues were
   *         empty.
   */
  [[nodiscard]] bool printNext();
};

#include "Serializer.tpp"

}  // namespace dbg
