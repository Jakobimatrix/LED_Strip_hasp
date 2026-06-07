#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace typ {

template <typename Key, typename Value, std::size_t Size>
struct Map {
  using Pair     = std::pair<Key, Value>;
  using Data     = std::array<Pair, Size>;
  using Iterator = typename Data::const_iterator;
  Data data;

  constexpr Map(const Data& data_)
      : data(data_) {}

  constexpr Map(std::initializer_list<Pair> init)
      : data{} {
    if (init.size() == Size) {
      std::copy(init.begin(), init.end(), std::begin(data));
    }
  }

  constexpr Iterator end() const { return std::cend(data); }
  constexpr Iterator begin() const { return std::cbegin(data); }

  constexpr Value at(const Key& key) const {
    return std::find_if(
             begin(), end(), [&key](const Pair& v) { return v.first == key; })
      ->second;
  }

  constexpr Iterator find(const Key& key) const {
    return std::find_if(
      begin(), end(), [&key](const Pair& v) { return v.first == key; });
  }
};

}  // namespace typ
