/**
 * @file Constants.hpp
 * @brief Small set of debugging constants: log levels, topics and lookups.
 *
 * This header defines the `dbg` namespace's logging enums and small
 * constexpr lookup tables used by the project's debug/log formatting
 * helpers. The lookup tables map enum values to short, human-readable
 * labels (the level strings include ANSI color escape sequences used
 * when the logger outputs to a terminal).
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Types/CMap.hpp>

#include <array>
#include <cstdint>
#include <utility>



namespace dbg {

/**
 * @namespace dbg
 * @brief Debugging and logging utilities and constants.
 */

/**
 * @enum LEVEL
 * @brief Log severity level used by the debug logger.
 *
 * The values are bitflags to allow simple masking/combination when
 * filtering output.
 */
enum class LEVEL : unsigned long {
  NONE  = 0x00, /**< No logging output. */
  INFO  = 0x01, /**< Informational messages. */
  WARN  = 0x02, /**< Warning conditions that are noteworthy. */
  ERROR = 0x04  /**< Error conditions requiring attention. */
};

/**
 * @enum TOPIC
 * @brief Logical subsystem/topic identifiers for log messages.
 *
 * Topics let the logger group and optionally filter messages by
 * subsystem (MQTT, BOOT, TASK, etc.). Values are distinct bitflags
 * for efficient combination and testing.
 */
enum class TOPIC : unsigned long {
  NONE     = 0x00000000, /**< No specific topic. */
  MQTT     = 0x00000001, /**< MQTT subsystem. */
  BOOT     = 0x00000002, /**< Boot / initialization code. */
  DEBUG    = 0x00000004, /**< Miscellaneous debug messages. */
  TASK     = 0x00000008, /**< Task scheduler / task-related messages. */
  WIFI     = 0x00000010, /**< WiFi subsystem. */
  WIFIPROV = 0x00000020, /**< WiFi provisioning. */
  LED      = 0x00000040, /**< LED handling / animations. */
  PERF     = 0x00000080  /**< Performance / profiling related messages. */
};

// clang-format off

/**
 * @brief Number of entries in `LEVEL_LOOKUP`.
 */
static constexpr unsigned NUM_LEVELS = 4;

/**
 * @brief Lookup table mapping `LEVEL` values to printable short labels.
 *
 * The mapped strings include optional ANSI color escape sequences used
 * to render colored level prefixes when outputting to a terminal. The
 * `typ::Map<Key, Value, N>` type is a small, constexpr-friendly fixed
 * size associative container (see `lib/Types/include/Types/CMap.hpp`).
 *
 * @note Key: `LEVEL`, Value: `const char*`, Size: `NUM_LEVELS`.
 */
static constexpr typ::Map<LEVEL, const char*, NUM_LEVELS> LEVEL_LOOKUP{{
  {LEVEL::NONE,  "[?]"},
  {LEVEL::INFO,  "\033[0;37m[I]\033[0m"},     // white
  {LEVEL::WARN,  "\033[0;33m[W]\033[0m"},     // yellow
  {LEVEL::ERROR, "\033[0;31m[E]\033[0m"}      // red
}};

/**
 * @brief Number of entries in `TOPIC_LOOKUP`.
 */
static constexpr unsigned NUM_TOPICS = 9;

/**
 * @brief Lookup table mapping `TOPIC` values to fixed-width labels.
 *
 * The labels are formatted to a fixed width so they align nicely in
 * the project's log output. Key: `TOPIC`, Value: `const char*`, Size:
 * `NUM_TOPICS`.
 */
static constexpr typ::Map<TOPIC, const char *, NUM_TOPICS> TOPIC_LOOKUP{{
  {TOPIC::NONE,      "[?]          "},
  {TOPIC::MQTT,      "[MQTT]       "},
  {TOPIC::BOOT,      "[BOOT]       "},
  {TOPIC::DEBUG,     "[DEBUG]      "},
  {TOPIC::TASK,      "[TASK]       "},
  {TOPIC::WIFI,      "[WIFI]       "},
  {TOPIC::WIFIPROV,  "[WIFIPROVISN]"},
  {TOPIC::LED,       "[LED]        "},
  {TOPIC::PERF,      "[PERFORMANCE]"}
}};

// clang-format on

}  // namespace dbg
