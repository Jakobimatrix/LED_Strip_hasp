#pragma once

#include <string_view>
#include <cstring>
#include <stdexcept>

namespace typ {

template <std::size_t N>
class StaticString {
 public:
  constexpr StaticString() noexcept = default;

  constexpr StaticString(std::string_view str) { assign(str); }

  constexpr StaticString(const char* const str) { assign(str); }

  constexpr bool assign(std::string_view str) {
    if (str.size() > N) {
      return false;
    }
    std::memcpy(buffer_, str.data(), str.size());
    size_ = str.size();
    return true;
  }

  constexpr bool push_back(char c) {
    if (size_ >= N) {
      return false;
    }
    buffer_[size_++] = c;
    return true;
  }

  constexpr char& operator[](std::size_t i) {
    if (i >= size_) {
      throw std::out_of_range("StaticString index out of range");
    }
    return buffer_[i];
  }

  constexpr std::string_view view() const noexcept {
    return std::string_view(buffer_, size_);
  }

  constexpr const char* data() const noexcept { return buffer_; }

  constexpr std::size_t size() const noexcept { return size_; }

  constexpr std::size_t capacity() const noexcept { return N; }

  constexpr bool empty() const noexcept { return size_ == 0; }

 private:
  char buffer_[N]   = {0};
  std::size_t size_ = 0;
};

}  // namespace typ
