#pragma once

#include <cstdint>

namespace dbg {

enum class TokenType : uint8_t { Text, Arg };

struct Token {
  TokenType type;
  uint16_t offset;
  uint16_t length;
};


template <StaticString fmt>
struct Format {
  static constexpr std::size_t max_tokens = 16;

  std::array<Token, max_tokens> tokens{};
  std::size_t count = 0;

  constexpr Format() {
    std::size_t i          = 0;
    std::size_t text_start = 0;

    while (i < fmt.size() && i < max_tokens - 1) {
      if (fmt.data()[i] == '%') {

        if (i > text_start) {
          tokens[count++] = Token{TokenType::Text,
                                  static_cast<uint16_t>(text_start),
                                  static_cast<uint16_t>(i - text_start)};
        }

        ++i;  // skip '%'

        tokens[count++] = Token{TokenType::Arg, 0, 0};

        text_start = i + 1;
        ++i;
      } else {
        ++i;
      }
    }

    if (text_start < fmt.size()) {
      tokens[count++] = Token{TokenType::Text,
                              static_cast<uint16_t>(text_start),
                              static_cast<uint16_t>(fmt.size() - text_start)};
    }
  }
};

}  // namespace dbg
