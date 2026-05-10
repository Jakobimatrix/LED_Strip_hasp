#pragma once

#include <type_traits>

// The following templatefunctions allow enum classes to be treated as integers
template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator~(const T& value) {
  return static_cast<T>(~static_cast<typename std::underlying_type<T>::type>(value));
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator|(const T& left, const T& right) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(left) |
                        static_cast<typename std::underlying_type<T>::type>(right));
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator|=(T& left, const T& right) {
  return left = left | right;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator&(const T& left, const T& right) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(left) &
                        static_cast<typename std::underlying_type<T>::type>(right));
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator&=(T& left, const T& right) {
  return left = left & right;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator>>(const T& value,
                              const typename std::underlying_type<T>::type& shift) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(value) >> shift);
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator>>=(T& value, const typename std::underlying_type<T>::type& shift) {
  return value = value >> shift;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator<<(const T& value,
                              const typename std::underlying_type<T>::type& shift) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(value) << shift);
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T& operator<<=(T& value, const typename std::underlying_type<T>::type& shift) {
  return value = value << shift;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr bool isSet(const T& mask, const T& probe) {
  return static_cast<bool>(mask & probe);
}