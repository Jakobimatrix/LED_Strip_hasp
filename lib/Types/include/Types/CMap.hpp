#/**
 * @file CMap.hpp
 * @brief Small constexpr-friendly fixed-size associative map wrapper.
 *
 * A tiny header-only convenience wrapper around a fixed-size
 * `std::array` of `std::pair<Key, Value>` that exposes iterator-style
 * lookup and simple accessors. Designed for use in constexpr contexts
 * where a small compile-time map is convenient.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 */

#pragma once

#include <algorithm>
#include <initializer_list>
#include <array>
#include <cstdint>
#include <utility>

namespace typ {

/**
 * @brief Fixed-size associative map.
 *
 * Template parameters:
 * @tparam Key   Type of the map key. Must be EqualityComparable with `==`.
 * @tparam Value Type of the stored value.
 * @tparam Size  Number of key/value pairs contained in the map.
 *
 * The `Map` stores exactly `Size` entries in a contiguous `std::array`.
 * It provides `begin`/`end` iterators, a `find` returning a const iterator,
 * and `at` returning the mapped `Value` for a matching key.
 *
 * Notes:
 * - `at` assumes the key exists; calling it with a missing key yields
 *   undefined behaviour (it dereferences the result of `std::find_if`).
 * - The type aliases `Pair`, `Data`, and `Iterator` mirror the underlying
 *   storage types for convenience.
 */
template <typename Key, typename Value, std::size_t Size>
struct Map {
  /**
   * @brief Alias for a single stored key/value pair.
   */
  using Pair = std::pair<Key, Value>;

  /**
   * @brief Underlying fixed-size storage type.
   */
  using Data = std::array<Pair, Size>;

  /**
   * @brief Const iterator type over the underlying storage.
   */
  using Iterator = typename Data::const_iterator;

  /**
   * @brief Internal storage containing exactly `Size` pairs.
   */
  Data data;

  /**
   * @brief Construct from an existing `Data` array.
   *
   * Performs a direct copy of the provided `data_` into internal storage.
   *
   * @param data_ Array of `Size` pairs to initialize the map with.
   */
  constexpr Map(const Data& data_);

  /**
   * @brief Construct from an initializer list.
   *
   * If the initializer list contains exactly `Size` elements those elements
   * are copied into the internal storage. Otherwise the internal storage
   * remains value-initialized.
   *
   * @param init Initializer list of `Pair` elements.
   */
  constexpr Map(std::initializer_list<Pair> init);

  /**
   * @brief Return iterator to the end of the map.
   *
   * @return Iterator Const iterator pointing past the last element.
   */
  constexpr Iterator end() const { return std::cend(data); }

  /**
   * @brief Return iterator to the beginning of the map.
   *
   * @return Iterator Const iterator pointing to the first element.
   */
  constexpr Iterator begin() const { return std::cbegin(data); }

  /**
   * @brief Access the value associated with `key`.
   *
   * This method searches for a pair whose `.first` equals `key` and
   * returns the corresponding `.second` value.
   *
   * @warning Calling this function with a key that is not present results
   *          in undefined behaviour because the implementation dereferences
   *          the iterator returned from `std::find_if` without checking.
   *
   * @param key Key to search for.
   * @return Value The value mapped to `key`.
   */
  constexpr Value at(const Key& key) const;

  /**
   * @brief Find the pair with matching `key`.
   *
   * Performs a linear search over the internal array and returns a const
   * iterator to the matching element or `end()` if no match is found.
   *
   * @param key Key to search for.
   * @return Iterator Const iterator pointing to the found pair or `end()`.
   */
  constexpr Iterator find(const Key& key) const {
    return std::find_if(
      begin(), end(), [&key](const Pair& v) { return v.first == key; });
  }
};

}  // namespace typ

#include "CMap.tpp"
