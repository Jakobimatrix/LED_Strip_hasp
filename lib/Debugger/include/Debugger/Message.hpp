#pragma once

#include <Types/StaticString.hpp>

#include <array>
#include <cstdint>

namespace dbg {


template <std::size_t N>
struct MessageBuffer {
  std::array<uint8_t, N> data{};
  std::size_t size = 0;

  constexpr uint8_t* ptr() noexcept { return data.data(); }
  constexpr size_t capacity() const noexcept { return N; }
};

template <std::size_t N>
struct DebugMessage {
  static constexpr std::size_t MSG_BUFFER_SIZE = N;
  LEVEL level;
  TOPIC topic;
  int64_t timestamp;
  MessageBuffer<MSG_BUFFER_SIZE> buffer{};
};

}  // namespace dbg
