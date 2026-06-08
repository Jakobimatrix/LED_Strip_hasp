/**
 * @file StaticString.hpp
 * @brief Small constexpr-friendly fixed-capacity string with embedded metadata.
 *
 * `StaticString<N>` provides a fixed-size, header-only string-like container
 * intended for constexpr use. The implementation stores up to `N-1` bytes of
 * character data in `buffer_` and uses the final byte as a metadata field
 * representing the number of free bytes remaining after the current content.
 * The class is deliberately simple and avoids dynamic allocation. It is
 * compatible with std::string_view and alway null-terminated.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */

#pragma once

#include <string_view>
#include <cstring>
#include <stdexcept>
#include <iterator>

namespace typ {

/**
 * @brief Fixed-size static string using an internal metadata byte.
 *
 * Template parameter `N` denotes the total buffer size including the
 * metadata byte. The usable character capacity is `N-1` (alias `MAX_SIZE`).
 * The implementation stores characters in `buffer_[0..N-2]` and places a
 * single metadata byte at `buffer_[N-1]` containing the number of free
 * bytes remaining after the stored content. The buffer is always null-terminated.
 *
 * @tparam N Total buffer size (includes metadata byte). Usable characters = `N-1`.
 */
template <std::size_t N>
class StaticString {
  static_assert(N > 1, "StaticString capacity must be greater than 1");
  static_assert(
    N < 255, "StaticString capacity must be less than 255 for metadata byte");

 public:
  /**
   * @brief Default-construct an empty StaticString.
   *
   * The metadata byte is initialized to indicate the full free capacity.
   */
  constexpr StaticString() noexcept;

  /**
   * @brief Construct from a `std::string_view`.
   *
   * @param str Source characters to copy into the static buffer.
   *
   * @note Construction will attempt to assign the full view; if it does
   * not fit, `assign` will return false (constructor does not propagate
   * failure).
   */
  constexpr StaticString(std::string_view str);

  /**
   * @brief Construct from a null-terminated C-string.
   *
   * @param str Null-terminated source string.
   */
  constexpr StaticString(const char* const str);

  /**
   * @brief Assign content from a `std::string_view` into the buffer.
   *
   * Copies up to `MAX_SIZE` characters from `str` into the internal buffer.
   * If `str.size()` exceeds `MAX_SIZE` the function returns `false` and the
   * buffer is left in its last state.
   *
   * @param str Source characters to copy.
   * @return `true` on success (all characters fit), `false` if the input
   *         length exceeds the available capacity.
   */
  constexpr bool assign(std::string_view str);

  /**
   * @brief Append a single character to the end of the stored content.
   *
   * @param c Character to append.
   * @return `true` if the character was appended, `false` if the buffer was full.
   */
  constexpr bool push_back(char c);

  /**
   * @brief Compile-time indexed access to the underlying buffer (const).
   *
   * @tparam i Index into the buffer. Must satisfy `i < N`.
   * @return const reference to the character at index `i`.
   */
  template <std::size_t i>
  constexpr const char& get() const noexcept;

  /**
   * @brief Compile-time indexed access to the underlying buffer (mutable).
   * @details USING THIS IS DANGEROUS: Mutating the buffer directly can break internal assumptions about null-termination and metadata integrity. Use with caution.
   *
   * @tparam i Index into the buffer. Must satisfy `i < N`.
   * @return reference to the character at index `i`.
   */
  template <std::size_t i>
  constexpr char& get() noexcept;

  /**
   * @brief Obtain a `std::string_view` referencing the stored characters.
   *
   * @return `std::string_view` covering the active content (length = `size()`).
   */
  constexpr std::string_view view() const noexcept;

  /**
   * @brief Get number of stored characters.
   *
   * The size is derived from the metadata byte stored in `buffer_[SIZE_BIT]`.
   *
   * @return Number of characters currently stored (0..`MAX_SIZE`).
   */
  constexpr std::size_t size() const noexcept;


  /**
   * @brief Return the maximum number of characters that can be stored.
   *
   * @return Capacity in characters (equals `N-1`).
   */
  constexpr std::size_t capacity() const noexcept;

  /**
   * @brief Test whether the string is empty.
   *
   * @return `true` if no characters are stored, otherwise `false`.
   */
  constexpr bool empty() const noexcept;

  /**
   * @brief Access the internal buffer as a C-string pointer.
   *
   * The buffer is guaranteed to be null-terminated except get was used to mutate characters without maintaining null-termination.
   *
   * @return Pointer to the internal character buffer.
   */
  constexpr const char* c_str() const noexcept;

 private:
  /**
   * @brief Mutable pointer to internal data.
   *
   * @return Pointer to the internal buffer.
   */
  constexpr char* data() noexcept;

  /**
   * @brief Index of the first free slot for appending.
   *
   * @return Index in `buffer_` where the next character will be written.
   */
  constexpr std::size_t nextEmptyIndex() const noexcept;

  /**
   * @brief Index of the metadata byte inside `buffer_`.
   */
  static constexpr std::size_t SIZE_BIT = N - 1;

  /**
   * @brief Maximum number of storable characters (excludes metadata byte).
   */
  static constexpr std::size_t MAX_SIZE = N - 1;

  /**
   * @brief Internal storage including metadata byte.
   *
   * Layout: characters at indices `[0..MAX_SIZE-1]`, metadata at `SIZE_BIT`.
   */
  char buffer_[N] = {0};
};
}  // namespace typ

#include "StaticString.tpp"
