#pragma once

#include <Arduino.h>
#include <cstring>
#include <cstdio>
#include <initializer_list>

#include <Debugger/Constants.hpp>
#include <Debugger/Types.hpp>
#include <Enums/ExpandEnums.hpp>

namespace glob {
extern dbg::LEVEL LogLevel;
extern dbg::TOPIC LogTopicMask;
}  // namespace glob

namespace dbg {


template <typename T>
  requires SerializableArg<T>
constexpr bool write_arg(uint8_t*& ptr, const uint8_t* start, size_t size, const T& value) noexcept {
  constexpr ArgType type = ArgTraits<std::decay_t<T>>::type;

  const size_t used = static_cast<size_t>(ptr - start);
  if (used > size) {
    return false;
  }

  if constexpr (ArgTraits<std::decay_t<T>>::type == ArgType::CString) {
    const uint16_t len =
      static_cast<uint16_t>(std::strlen(reinterpret_cast<const char*>(value)));
    const size_t need = 1 + sizeof(len) + static_cast<size_t>(len);
    if (need > (size - used)) {
      return false;  // buffer overflow
    }
    *ptr++ = static_cast<uint8_t>(type);
    std::memcpy(ptr, &len, sizeof(len));
    ptr += sizeof(len);

    std::memcpy(ptr, value, len);
    ptr += len;
  } else {
    const size_t need = 1 + sizeof(T);
    if (need > (size - used)) {
      return false;  // buffer overflow
    }
    *ptr++ = static_cast<uint8_t>(type);
    std::memcpy(ptr, &value, sizeof(T));
    ptr += sizeof(T);
  }

  return true;
}


template <typename Queue>
class Logger {
 public:
  explicit Logger(Queue& q)
      : queue_(q) {}

  template <typename... Args>
  constexpr void log(LEVEL lvl, TOPIC topic, const Args&... args) noexcept {
    if (!isSet(glob::LogLevel, lvl) || !isSet(glob::LogTopicMask, topic)) {
      return;  // filter out messages below global log level or not in global topic mask
    }
    typename Queue::ValueType msg{};
    msg.level     = lvl;
    msg.topic     = topic;
    msg.timestamp = esp_timer_get_time();

    uint8_t* ptr         = msg.buffer.ptr();
    const uint8_t* start = ptr;

    bool ok = true;
    (void)std::initializer_list<int>{
      (ok = ok & write_arg(ptr, start, msg.buffer.capacity(), args), 0)...};

    msg.buffer.size = ptr - start;

    if (!ok) {
      // const char* error_msg = "buffer overflow in next message!";
      // log(LEVEL::ERROR, TOPIC::DEBUG, error_msg);
      //  we can still print the first bits that did fit.
    }
    if (!queue_.push(msg)) {
      // handle push failure
    }
  }

 private:
  Queue& queue_;
};

}  // namespace dbg
