#pragma once
#if !defined(LOX_AST_PARSE_H)
#define LOX_AST_PARSE_H

#include <gsl/span>
#include <tuple>

#include "lox/ast/expression.hpp"
#include "lox/error.hpp"
#include "lox/token.hpp"

namespace lox
{
using parse_result = result<std::tuple<std::unique_ptr<Expression>, gsl::span<Token>>>;
auto parse(gsl::span<Token> tokens) -> parse_result;
/// expression -> block
auto parse_expression(gsl::span<Token> tokens) -> parse_result;
/// block -> ternary ("," ternary)*
auto parse_block(gsl::span<Token> tokens) -> parse_result;
/// ternary -> equality ("?" ternary ":" ternary)*
auto parse_ternary(gsl::span<Token> tokens) -> parse_result;
/// equality -> comparison (("!=" | "==") comparison)*
auto parse_equality(gsl::span<Token> tokens) -> parse_result;
/// comparison -> addition ((">" | ">=" | "<" | "<=") addition)*
auto parse_comparison(gsl::span<Token> tokens) -> parse_result;
/// addition -> multiplication (("-" | "+") multiplication)*
auto parse_addition(gsl::span<Token> tokens) -> parse_result;
/// multiplication -> unary (("/" | "*") unary)*
auto parse_multiplication(gsl::span<Token> tokens) -> parse_result;
/// unary -> ("!" | "-") unary | primary
auto parse_unary(gsl::span<Token> tokens) -> parse_result;
/// primary -> NUMBER | STRING | "false" | "true" | "nil" | "(" expression ")"
auto parse_primary(gsl::span<Token> tokens) -> parse_result;
}  // namespace lox

#endif  // LOX_AST_PARSE_H
