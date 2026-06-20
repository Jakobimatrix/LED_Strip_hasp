/**
 * @file Types.hpp
 * @brief Traits and small type utilities used by the debug logging helpers.
 *
 * This header declares a compact, constexpr-friendly description of
 * supported argument types (`ArgType`) and the `ArgTraits<T>` template
 * specializations used to map C++ types to the runtime `ArgType` enum.
 * It also provides the `SerializableArg` concept for constraining
 * logging call templates to supported argument types.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Debugger/Constants.hpp>

#include <cstdint>
#include <concepts>
#include <type_traits>

/**
 * @namespace dbg
 * @brief Debugging and logging utilities and constants.
 */
namespace dbg {


/**
 * @enum ArgType
 * @brief Compact runtime identifier for supported argument types.
 *
 * `ArgType` values are used internally by the logging/serialization
 * helpers to describe the runtime type of a provided argument in a
 * compact, portable form (stored as a `uint8_t`).
 */
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

/**
 * @brief `ArgTraits` template specialization for supported argument types.
 * @tparam T The type for which to provide traits.
 */
template <typename T>
struct ArgTraits {
  static constexpr bool supported = false;
};

/**
 * @brief `ArgTraits` specialization for `bool`.
 * @tparam (specialization) `bool`
 */
template <>
struct ArgTraits<bool> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Bool;
};

/**
 * @brief `ArgTraits` specialization for `char` (character type).
 * @tparam (specialization) `char`
 */
template <>
struct ArgTraits<char> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Char;
};

/**
 * @brief `ArgTraits` specialization for `signed char` (8-bit signed integer).
 * @tparam (specialization) `signed char`
 */
template <>
struct ArgTraits<signed char> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::SignedChar;
};

/**
 * @brief `ArgTraits` specialization for `unsigned char` (8-bit unsigned integer).
 * @tparam (specialization) `unsigned char`
 */
template <>
struct ArgTraits<unsigned char> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UnsignedChar;
};

/**
 * @brief `ArgTraits` specialization for `short` (16-bit signed integer).
 * @tparam (specialization) `short`
 */
template <>
struct ArgTraits<short> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Int16;
};

/**
 * @brief `ArgTraits` specialization for `unsigned short` (16-bit unsigned).
 * @tparam (specialization) `unsigned short`
 */
template <>
struct ArgTraits<unsigned short> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UInt16;
};

/**
 * @brief `ArgTraits` specialization for `int` (32-bit signed).
 * @tparam (specialization) `int`
 */
template <>
struct ArgTraits<int> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Int32;
};

/**
 * @brief `ArgTraits` specialization for `unsigned int` (32-bit unsigned).
 * @tparam (specialization) `unsigned int`
 */
template <>
struct ArgTraits<unsigned int> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UInt32;
};

/**
 * @brief `ArgTraits` specialization for `long`.
 *
 * The mapped `ArgType` depends on `sizeof(long)` on the target
 * platform; this keeps the trait portable across 32/64-bit targets.
 * @tparam (specialization) `long`
 */
template <>
struct ArgTraits<long> {
  static constexpr bool supported = true;
  static constexpr ArgType type = (sizeof(long) == 4) ? ArgType::Int32 : ArgType::Int64;
};

/**
 * @brief `ArgTraits` specialization for `unsigned long`.
 *
 * Platform-dependent mapping to 32/64-bit unsigned `ArgType`.
 * @tparam (specialization) `unsigned long`
 */
template <>
struct ArgTraits<unsigned long> {
  static constexpr bool supported = true;
  static constexpr ArgType type =
    (sizeof(unsigned long) == 4) ? ArgType::UInt32 : ArgType::UInt64;
};

/**
 * @brief `ArgTraits` specialization for `long long` (64-bit signed).
 * @tparam (specialization) `long long`
 */
template <>
struct ArgTraits<long long> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Int64;
};

/**
 * @brief `ArgTraits` specialization for `unsigned long long` (64-bit unsigned).
 * @tparam (specialization) `unsigned long long`
 */
template <>
struct ArgTraits<unsigned long long> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::UInt64;
};

/**
 * @brief `ArgTraits` specialization for `float` (32-bit floating point).
 * @tparam (specialization) `float`
 */
template <>
struct ArgTraits<float> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Float32;
};

/**
 * @brief `ArgTraits` specialization for `double` (64-bit floating point).
 * @tparam (specialization) `double`
 */
template <>
struct ArgTraits<double> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Float64;
};

/**
 * @brief `ArgTraits` specialization for C-style strings (`const char*`).
 * @tparam (specialization) `const char*`
 */
template <>
struct ArgTraits<const char*> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::CString;
};

/**
 * @brief `ArgTraits` specialization for mutable C-style strings (`char*`).
 * @tparam (specialization) `char*`
 */
template <>
struct ArgTraits<char*> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::CString;
};

/**
 * @brief `ArgTraits` specialization for the project's `LEVEL` enum.
 * @tparam (specialization) `LEVEL`
 */
template <>
struct ArgTraits<LEVEL> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Level;
};

/**
 * @brief `ArgTraits` specialization for the project's `TOPIC` enum.
 * @tparam (specialization) `TOPIC`
 */
template <>
struct ArgTraits<TOPIC> {
  static constexpr bool supported = true;
  static constexpr ArgType type   = ArgType::Topic;
};

/**
 * @brief Concept that constrains a template parameter to supported argument types.
 *
 * A type satisfies `SerializableArg` when a matching `ArgTraits`
 * specialization exists and its `supported` member is `true`.
 *
 * @tparam T Candidate type to test.
 */
template <typename T>
concept SerializableArg = ArgTraits<std::decay_t<T>>::supported;

}  // namespace dbg
