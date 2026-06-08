/**
 * @file ExpandEnums.hpp
 * @brief Lightweight helpers to treat C++ enum classes as bitmasks/integers.
 *
 * This header provides a small set of constexpr overloaded operators and a
 * utility alias that enable performing bitwise and shift operations on
 * scoped enum types (enum class) by forwarding operations to the
 * underlying integer representation. All templates are constrained via
 * SFINAE to accept only enumeration types.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 */

#pragma once

#include <type_traits>

// The following template functions allow enum classes to be treated as integers

/**
 * @brief Helper alias mirroring `std::enable_if`'s typical `enable_if_t`.
 *
 * This alias is used as the (unused) second template parameter in the
 * operator templates below to constrain them to enumeration types.
 *
 * @tparam B Compile-time boolean condition.
 * @tparam T Type returned when `B` is true (defaults to `void`).
 */
template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

/**
 * @brief Bitwise NOT for enum classes.
 *
 * Performs a bitwise inversion on the underlying integer representation and
 * returns the result cast back to the enum type.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param value Enum value to invert.
 * @return Resulting enum value after bitwise NOT.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator~(const T& value) {
  return static_cast<T>(~static_cast<typename std::underlying_type<T>::type>(value));
}

/**
 * @brief Bitwise OR for enum classes.
 *
 * Combines two enum bitmasks by performing `|` on their underlying values.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param left Left operand.
 * @param right Right operand.
 * @return New enum value containing the bitwise OR of the operands.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator|(const T& left, const T& right) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(left) |
                        static_cast<typename std::underlying_type<T>::type>(right));
}

/**
 * @brief Bitwise OR-assignment for enum classes.
 *
 * Performs `left = left | right` and returns a reference to the modified
 * left-hand operand.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param left Left operand to modify.
 * @param right Right operand to OR into `left`.
 * @return Reference to the modified `left`.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator|=(T& left, const T& right) {
  return left = left | right;
}

/**
 * @brief Bitwise AND for enum classes.
 *
 * Computes the bitwise AND of two enum values.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param left Left operand.
 * @param right Right operand.
 * @return New enum value containing the bitwise AND of the operands.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator&(const T& left, const T& right) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(left) &
                        static_cast<typename std::underlying_type<T>::type>(right));
}

/**
 * @brief Bitwise AND-assignment for enum classes.
 *
 * Performs `left = left & right` and returns a reference to the modified
 * left-hand operand.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param left Left operand to modify.
 * @param right Right operand to AND into `left`.
 * @return Reference to the modified `left`.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator&=(T& left, const T& right) {
  return left = left & right;
}

/**
 * @brief Right shift operator for enum classes.
 *
 * Shifts the underlying integer representation to the right by `shift`
 * and returns the resulting enum value.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param value Enum value to shift.
 * @param shift Number of bits to shift (type matches the enum's underlying
 *              integer type).
 * @return Shifted enum value.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator>>(const T& value,
                              const typename std::underlying_type<T>::type& shift) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(value) >> shift);
}

/**
 * @brief Right shift assignment for enum classes.
 *
 * Performs `value = value >> shift` and returns a reference to the
 * modified `value`.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param value Enum value to modify.
 * @param shift Number of bits to shift (type matches the enum's underlying
 *              integer type).
 * @return Reference to the modified `value`.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator>>=(T& value, const typename std::underlying_type<T>::type& shift) {
  return value = value >> shift;
}

/**
 * @brief Left shift operator for enum classes.
 *
 * Shifts the underlying integer representation to the left by `shift`
 * and returns the resulting enum value.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param value Enum value to shift.
 * @param shift Number of bits to shift (type matches the enum's underlying
 *              integer type).
 * @return Shifted enum value.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator<<(const T& value,
                              const typename std::underlying_type<T>::type& shift) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(value) << shift);
}

/**
 * @brief Left shift assignment for enum classes.
 *
 * Performs `value = value << shift` and returns a reference to the
 * modified `value`.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param value Enum value to modify.
 * @param shift Number of bits to shift (type matches the enum's underlying
 *              integer type).
 * @return Reference to the modified `value`.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator<<=(T& value, const typename std::underlying_type<T>::type& shift) {
  return value = value << shift;
}

/**
 * @brief Check whether any bits in `probe` are set in `mask`.
 *
 * A convenience returning `true` if `(mask & probe) != 0` when interpreted
 * as booleans. Useful for testing flag-style enum classes.
 *
 * @tparam T Enum type to operate on.
 * @tparam Enable Unnamed SFINAE parameter used to enable this overload only
 *                for enum types.
 * @param mask The bitmask to test against.
 * @param probe Bits to probe for in `mask`.
 * @return `true` if any bits from `probe` are present in `mask`, otherwise
 *         `false`.
 */
template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr bool isSet(const T& mask, const T& probe) {
  return static_cast<bool>(mask & probe);
}