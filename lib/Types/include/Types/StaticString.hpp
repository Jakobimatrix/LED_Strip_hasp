#pragma once

#include <string_view>
#include <cstring>
#include <stdexcept>
#include <iterator>

namespace typ {

template <std::size_t N>
class StaticString {
  static_assert(N > 1, "StaticString capacity must be greater than 1");
  static_assert(
    N < 255, "StaticString capacity must be less than 255 for metadata byte");

 public:
  constexpr StaticString() noexcept {
    buffer_[N - 1] = static_cast<char>(N - 1);
  }

  constexpr StaticString(std::string_view str) { assign(str); }

  constexpr StaticString(const char* const str) { assign(str); }

  constexpr bool assign(std::string_view str) {
    if (str.size() > (MAX_SIZE)) {
      return false;
    }
    if (str.size() > 0) {
      std::memcpy(buffer_, str.data(), str.size());
    }
    const std::size_t freeSpaceAfterInsert = MAX_SIZE - str.size();

    if (freeSpaceAfterInsert > 0) {
      auto begin = std::begin(buffer_);
      std::advance(begin, static_cast<std::ptrdiff_t>(str.size()));
      std::fill(begin, std::prev(std::end(buffer_)), '\0');
    }
    buffer_[SIZE_BIT] = static_cast<char>(freeSpaceAfterInsert);
    return true;
  }

  constexpr bool push_back(char c) {
    if (size() >= MAX_SIZE) {
      return false;
    }
    buffer_[nextEmptyIndex()] = c;

    --buffer_[SIZE_BIT];
    return true;
  }

  template <std::size_t i>
  constexpr const char& get() const noexcept {
    static_assert(i < N, "Index out of bounds");
    return buffer_[i];
  }

  template <std::size_t i>
  constexpr char& get() noexcept {
    static_assert(i < N, "Index out of bounds");
    return buffer_[i];
  }

  constexpr std::string_view view() const noexcept {
    return std::string_view(buffer_, size());
  }

  constexpr std::size_t size() const noexcept {
    return MAX_SIZE - static_cast<std::size_t>(buffer_[SIZE_BIT]);
  }


  constexpr std::size_t capacity() const noexcept { return MAX_SIZE; }

  constexpr bool empty() const noexcept { return size() == 0; }

  constexpr const char* c_str() const noexcept { return buffer_; }

 private:
  constexpr char* data() noexcept { return buffer_; }
  constexpr std::size_t nextEmptyIndex() const noexcept { return size(); }

  static constexpr std::size_t SIZE_BIT = N - 1;
  static constexpr std::size_t MAX_SIZE = N - 1;
  char buffer_[N]                       = {0};
};

}  // namespace typ
