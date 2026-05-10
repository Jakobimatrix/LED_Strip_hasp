#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace typ {

template <typename Key, typename Value, std::size_t Size>
struct Map {
  using Pair    = std::pair<Key, Value>;
  using Data    = std::array<Pair, Size>;
  using Type    = typename std::decay<Data>::type;
  using Pointer = typename std::decay<Type>::pointer;
  Data data;

  constexpr Map(const Type data_)
      : data(data_) {}

  constexpr Pointer end() const { return data.end(); }

  constexpr Value at(const Key& key) const {
    return std::find_if(begin(data),
                        end(data),
                        [&key](const Pair& v) { return v.first == key; })
      ->second;
  }

  constexpr Pointer find(const Key& key) const {
    return std::find_if(
      begin(data), end(data), [&key](const Pair& v) { return v.first == key; });
  }
};

}  // namespace typ
