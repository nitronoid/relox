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
using parse_list_result = result<std::vector<std::unique_ptr<Expression>>>;
using parse_result = result<std::tuple<std::unique_ptr<Expression>, gsl::span<Token>>>;

/// Parse a list of tokens to produce a list of instructions
auto parse(gsl::span<Token> tokens) -> parse_list_result;

/// declaration -> definition | statement
auto parse_declaration(gsl::span<Token> tokens) -> parse_result;

/// definition -> "var" IDENTIFIER ("=" expression)? ";"
auto parse_definition(gsl::span<Token> tokens) -> parse_result;

/// statement -> ((expression | print) ";") | block
auto parse_statement(gsl::span<Token> tokens) -> parse_result;

/// block -> "{" declaration* expression? "}"
auto parse_block(gsl::span<Token> tokens) -> parse_result;

/// print -> "print" expression ";"
auto parse_print(gsl::span<Token> tokens) -> parse_result;

/// expression -> list
auto parse_expression(gsl::span<Token> tokens) -> parse_result;

/// list -> assignment ("," assignment)*
auto parse_list(gsl::span<Token> tokens) -> parse_result;

/// assignment -> IDENTIFIER "=" assignment | ternary | block
auto parse_assignment(gsl::span<Token> tokens) -> parse_result;

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

/// primary -> NUMBER | STRING | "false" | "true" | "nil" | "(" expression ")" | IDENTIFIER
auto parse_primary(gsl::span<Token> tokens) -> parse_result;
}  // namespace lox

#endif  // LOX_AST_PARSE_H
