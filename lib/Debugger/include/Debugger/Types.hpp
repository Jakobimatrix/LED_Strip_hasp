#pragma once

#include <Debugger/Constants.hpp>

#include <cstdint>
#include <concepts>
#include <type_traits>

namespace dbg {


enum class ArgType : uint8_t {
  Bool,

  Char,
  SignedChar,
  UnsignedChar,

  Int16,
  UInt16,

  Int32,
  UInt32,

  Int64,
  UInt64,

  Float32,
  Float64,

  CString,

  Level,
  Topic
};

template <typename T>
struct ArgTraits {
  static constexpr bool supported = false;
};

template <>
struct ArgTraits<bool> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Bool;
};

template <>
struct ArgTraits<char> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Char;
};

template <>
struct ArgTraits<signed char> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::SignedChar;
};

template <>
struct ArgTraits<unsigned char> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UnsignedChar;
};

template <>
struct ArgTraits<short> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Int16;
};

template <>
struct ArgTraits<unsigned short> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UInt16;
};

template <>
struct ArgTraits<int> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Int32;
};

template <>
struct ArgTraits<unsigned int> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UInt32;
};

template <>
struct ArgTraits<long> {
  static constexpr bool supported = true;
  static constexpr ArgType type = (sizeof(long) == 4) ? ArgType::Int32 : ArgType::Int64;
};

template <>
struct ArgTraits<unsigned long> {
  static constexpr bool supported = true;
  static constexpr ArgType type =
    (sizeof(unsigned long) == 4) ? ArgType::UInt32 : ArgType::UInt64;
};

template <>
struct ArgTraits<long long> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Int64;
};

template <>
struct ArgTraits<unsigned long long> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UInt64;
};

template <>
struct ArgTraits<float> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Float32;
};

template <>
struct ArgTraits<double> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Float64;
};

template <>
struct ArgTraits<const char*> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::CString;
};

template <>
struct ArgTraits<char*> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::CString;
};

template <>
struct ArgTraits<LEVEL> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Level;
};

template <>
struct ArgTraits<TOPIC> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Topic;
};

template <typename T>
concept SerializableArg = ArgTraits<std::decay_t<T>>::supported;

}  // namespace dbg
