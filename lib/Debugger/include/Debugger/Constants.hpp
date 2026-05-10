#pragma once

#include <Types/CMap.hpp>

#include <array>
#include <cstdint>
#include <utility>

namespace dbg {

enum class LEVEL : unsigned long {
  NONE  = 0x00,
  INFO  = 0x01,
  WARN  = 0x02,
  ERROR = 0x04
};

enum class TOPIC : unsigned long {
  NONE     = 0x00000000,
  MQTT     = 0x00000001,
  BOOT     = 0x00000004,
  DEBUG    = 0x00000008,
  WIFI     = 0x00000010,
  LED      = 0x00000100,
  SCHEDULE = 0x00000200,
  PERF     = 0x00004000
};

// clang-format off
static constexpr unsigned NUM_LEVELS = 4;
static constexpr typ::Map<LEVEL, const char*, NUM_LEVELS> LEVEL_LOOKUP{{
  {LEVEL::NONE,  "[?]"},
  {LEVEL::INFO,  "\033[0;37m[I]\033[0m"},     // white
  {LEVEL::WARN,  "\033[0;33m[W]\033[0m"},     // yellow
  {LEVEL::ERROR, "\033[0;31m[E]\033[0m"}    // red
}};

static constexpr unsigned NUM_TOPICS = 8;
static constexpr typ::Map<TOPIC, const char *, NUM_TOPICS> TOPIC_LOOKUP{{
  {TOPIC::NONE,      "[?]          "},
  {TOPIC::MQTT,      "[MQTT]       "},
  {TOPIC::BOOT,      "[BOOT]       "},
  {TOPIC::DEBUG,     "[DEBUG]      "},
  {TOPIC::WIFI,      "[WIFI]       "},
  {TOPIC::LED,       "[LED]        "},
  {TOPIC::SCHEDULE,  "[SCHEDULER]  "},
  {TOPIC::PERF,      "[PERFORMANCE]"}
}};

// clang-format on

}  // namespace dbg
