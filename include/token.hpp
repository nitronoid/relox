#pragma once
#if !defined(LOX_TOKEN_H)
#define LOX_TOKEN_H

#include <vector>
#include <variant>
#include <cstdint>
#include <string_view>
#include "lox/error.hpp"

namespace lox
{
// clang-format off
enum class TOKEN_TYPE : uint8_t
{
  // Should be ignored.---------------------------------------------------------
  COMMENT,
  // Single-character tokens.---------------------------------------------------
  LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, LEFT_BRACKET, RIGHT_BRACKET,
  COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
  // One or two character tokens.-----------------------------------------------
  BANG_EQUAL, BANG, EQUAL, GREATER_EQUAL, LESS_EQUAL, GREATER, LESS, ASSIGN,
  // Keywords.
  AND, STRUCT, ELSE, FUN, FOR, IF, NIL, OR, PRINT, RETURN, SUPER, THIS,
  TRUE, FALSE, VAR, WHILE,
  // Literals.-----------------------------------------------------------------
  IDENTIFIER, STRING, NUMBER,
  // Represents a lexical error.------------------------------------------------
  ERROR,
  // END OF FILE.---------------------------------------------------------------
  END,
};
// clang-format on

struct Token
{
  using literal = std::variant<std::monostate, std::string, float, bool>;
  TOKEN_TYPE type;
  std::string_view lexeme;
  std::size_t line;
  literal literal_value{std::monostate{}};
};
} // namespace lox
#endif // LOX_TOKEN_H
