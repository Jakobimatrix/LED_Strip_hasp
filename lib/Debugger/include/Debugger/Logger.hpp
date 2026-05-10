#pragma once

#include <Arduino.h>

#include <Debugger/Constants.hpp>
#include <Debugger/Types.hpp>
#include <Enums/ExpandEnums.hpp>

namespace dbg {


template <typename T>
  requires SerializableArg<T>
constexpr bool write_arg(uint8_t*& ptr, const uint8_t*& start, size_t size, const T& value) noexcept {
  constexpr ArgType type = ArgTraits<std::decay_t<T>>::type;

  if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
    const uint16_t len = static_cast<uint16_t>(std::strlen(value));
    if (ptr - start + sizeof(len) + len + 1 > size) {
      return false;  // buffer overflow
    }
    *ptr++ = static_cast<uint8_t>(type);
    std::memcpy(ptr, &len, sizeof(len));
    ptr += sizeof(len);

    std::memcpy(ptr, value, len);
    ptr += len;
  } else {
    if (ptr - start + sizeof(T) + 1 > size) {
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
    if (!(lvl & glob::globalLogLevel) || !(topic & glob::globalLogTopicMask)) {
      return;  // filter out messages below global log level or not in global topic mask
    }
    Queue::ValueType msg{};
    msg.level     = lvl;
    msg.topic     = topic;
    msg.timestamp = esp_timer_get_time();

    uint8_t* ptr         = msg.buffer.ptr();
    const uint8_t* start = ptr;

    bool ok = true;
    (void)((ok &= write_arg(ptr, start, msg.buffer.capacity(), args)), ...);

    msg.size = ptr - start;

    if (ok) {
      const char* error_msg = "buffer overflow in next message!";
      log(LEVEL::ERROR, TOPIC::DEBUG, error_msg);
    }
    queue_.push(msg);
  }

 private:
  Queue& queue_;
};

}  // namespace dbg
