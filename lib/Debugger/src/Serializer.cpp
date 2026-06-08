#include <Debugger/Serializer.hpp>

namespace dbg {

// `timestamp` is provided by `esp_timer_get_time()` and is in microseconds.
// Format as uptime: dddd/HH:mm:SS:ms (days since boot, 4 digits, capped at 9999)
typ::StaticString<18> timestamp2string(int64_t timestamp) {
  int64_t us = timestamp;
  if (us < 0)
    us = 0;
  int64_t ms   = (us / 1000) % 1000;  // milliseconds part
  int64_t secs = us / 1000000;

  int64_t s    = secs % 60;
  int64_t mins = (secs / 60) % 60;
  int64_t hrs  = (secs / 3600) % 24;
  int64_t days = secs / 86400;
  if (days > 9999)
    days = 9999;

  typ::StaticString<18> result;
  result.get<0>()  = '0' + static_cast<char>((days / 1000) % 10);
  result.get<1>()  = '0' + static_cast<char>((days / 100) % 10);
  result.get<2>()  = '0' + static_cast<char>((days / 10) % 10);
  result.get<3>()  = '0' + static_cast<char>(days % 10);
  result.get<4>()  = '/';
  result.get<5>()  = '0' + static_cast<char>((hrs / 10) % 10);
  result.get<6>()  = '0' + static_cast<char>(hrs % 10);
  result.get<7>()  = ':';
  result.get<8>()  = '0' + static_cast<char>((mins / 10) % 10);
  result.get<9>()  = '0' + static_cast<char>(mins % 10);
  result.get<10>() = ':';
  result.get<11>() = '0' + static_cast<char>((s / 10) % 10);
  result.get<12>() = '0' + static_cast<char>(s % 10);
  result.get<13>() = ':';
  result.get<14>() = '0' + static_cast<char>((ms / 100) % 10);
  result.get<15>() = '0' + static_cast<char>((ms / 10) % 10);
  result.get<16>() = '0' + static_cast<char>(ms % 10);
  result.get<17>() = '\0';

  return result;
}

}  // namespace dbg
