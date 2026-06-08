/**
 * Implementation for StaticString template.
 */

#include <algorithm>

namespace typ {

template <std::size_t N>
constexpr StaticString<N>::StaticString() noexcept {
  buffer_[N - 1] = static_cast<char>(N - 1);
}

template <std::size_t N>
constexpr StaticString<N>::StaticString(std::string_view str) {
  assign(str);
}

template <std::size_t N>
constexpr StaticString<N>::StaticString(const char* const str) {
  assign(str);
}

template <std::size_t N>
constexpr bool StaticString<N>::assign(std::string_view str) {
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

template <std::size_t N>
constexpr bool StaticString<N>::push_back(char c) {
  if (size() >= MAX_SIZE) {
    return false;
  }
  buffer_[nextEmptyIndex()] = c;

  --buffer_[SIZE_BIT];
  return true;
}

template <std::size_t N>
template <std::size_t i>
constexpr const char& StaticString<N>::get() const noexcept {
  static_assert(i < N, "Index out of bounds");
  return buffer_[i];
}

template <std::size_t N>
template <std::size_t i>
constexpr char& StaticString<N>::get() noexcept {
  static_assert(i < N, "Index out of bounds");
  return buffer_[i];
}

template <std::size_t N>
constexpr std::string_view StaticString<N>::view() const noexcept {
  return std::string_view(buffer_, size());
}

template <std::size_t N>
constexpr std::size_t StaticString<N>::size() const noexcept {
  return MAX_SIZE - static_cast<std::size_t>(buffer_[SIZE_BIT]);
}

template <std::size_t N>
constexpr std::size_t StaticString<N>::capacity() const noexcept {
  return MAX_SIZE;
}

template <std::size_t N>
constexpr bool StaticString<N>::empty() const noexcept {
  return size() == 0;
}

template <std::size_t N>
constexpr const char* StaticString<N>::c_str() const noexcept {
  return buffer_;
}

template <std::size_t N>
constexpr char* StaticString<N>::data() noexcept {
  return buffer_;
}

template <std::size_t N>
constexpr std::size_t StaticString<N>::nextEmptyIndex() const noexcept {
  return size();
}

}  // namespace typ
