/**
 * Implementation file for Map template in CMap.hpp.
 *
 * Definitions are placed here to keep the class declaration minimal while
 * preserving constexpr semantics and inlining opportunities.
 */

#include <algorithm>

namespace typ {

template <typename Key, typename Value, std::size_t Size>
constexpr Map<Key, Value, Size>::Map(const Data& data_)
    : data(data_) {}

template <typename Key, typename Value, std::size_t Size>
constexpr Map<Key, Value, Size>::Map(std::initializer_list<Pair> init)
    : data{} {
  if (init.size() == Size) {
    std::copy(init.begin(), init.end(), std::begin(data));
  }
}

template <typename Key, typename Value, std::size_t Size>
constexpr Value Map<Key, Value, Size>::at(const Key& key) const {
  return std::find_if(
           begin(), end(), [&key](const Pair& v) { return v.first == key; })
    ->second;
}


}  // namespace typ
