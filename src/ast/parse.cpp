#include "lox/ast/parse.hpp"

#include <fmt/format.h>

#include <magic_enum/magic_enum.hpp>

namespace lox
{
namespace
{
template <TOKEN_TYPE... Types>
constexpr auto match(gsl::span<Token> tokens) -> bool
{
  if (tokens.empty()) return false;
  return ((tokens[0].type == Types) || ...);
}

template <TOKEN_TYPE... Types, typename F>
auto parse_recursive_binary(gsl::span<Token> tokens, F&& rule) -> parse_result
{
  // Check that we have a lhs operand
  if (!match<TOKEN_TYPE::MINUS>(tokens) && match<Types...>(tokens))
  {
    return lox::error("Binary expression missing left operand.", tokens.empty() ? ~0u : tokens[0].line);
  }
  // Parse an initial comparison
  std::unique_ptr<Expression> expr;
  {
    auto parsed = std::invoke(std::forward<F>(rule), tokens);
    if (!parsed.has_value()) return parsed;
    std::tie(expr, tokens) = std::move(*parsed);
  }
  // Continue to parse binary equalities until we've exhausted the contiguous set
  while (match<Types...>(tokens))
  {
    std::unique_ptr<Expression> right;
    TOKEN_TYPE const operation = tokens[0].type;
    auto const parsed =
      std::invoke(std::forward<F>(rule), tokens.subspan(1))
        .map([&](auto&& parsed) { std::tie(right, tokens) = std::move(parsed); })
        .map([&] { expr = std::make_unique<Binary>(std::move(expr), std::move(right), operation); });
    if (!parsed) return lox::error(parsed.error());
  }
  // Return the expression tree head and the reduced token set
  return std::make_tuple(std::move(expr), tokens);
}

}  // namespace

auto parse(gsl::span<Token> tokens) -> parse_list_result
{
  std::vector<std::unique_ptr<Expression>> program;
  while (!tokens.empty() && tokens[0].type != TOKEN_TYPE::END)
  {
    program.emplace_back();
    auto stmt = parse_declaration(tokens);
    if (!stmt.has_value()) return lox::error(stmt.error());
    std::tie(program.back(), tokens) = std::move(*stmt);
  }
  return program;
}

auto parse_declaration(gsl::span<Token> tokens) -> parse_result
{
  if (!match<TOKEN_TYPE::VAR>(tokens)) return parse_statement(tokens);
  return parse_definition(tokens);
}

auto parse_definition(gsl::span<Token> tokens) -> parse_result
{
  if (!match<TOKEN_TYPE::VAR>(tokens))
  {
    return lox::error("Expected 'var' keyword.", tokens.data()[-1].line);
  }
  if (!match<TOKEN_TYPE::IDENTIFIER>(tokens.subspan(1)))
  {
    return lox::error("Expected an identifier.", tokens.data()[0].line);
  }
  auto const name = tokens[1];
  tokens = tokens.subspan(2);

  // Assignment of a value is optional, default to nil
  std::unique_ptr<Expression> value;
  if (match<TOKEN_TYPE::ASSIGN>(tokens))
  {
    tokens = tokens.subspan(1);
    auto expr = parse_expression(tokens);
    if (!expr.has_value()) return expr;
    std::tie(value, tokens) = std::move(*expr);
  }
  else
  {
    value = std::make_unique<Literal>(std::monostate{});
  }

  if (!match<TOKEN_TYPE::SEMICOLON>(tokens))
  {
    return lox::error("Expected ';' after expression.", tokens.data()[-1].line);
  }

  return std::make_tuple(std::make_unique<Definition>(std::move(name), std::move(value)),
                         tokens.subspan(1));
}

auto parse_statement(gsl::span<Token> tokens) -> parse_result
{
  std::unique_ptr<Expression> expr;
  auto const& token = tokens[0];
  auto parsed = [&]() -> parse_result {
    switch (token.type)
    {
    case TOKEN_TYPE::LEFT_BRACE: return parse_block(tokens);
    case TOKEN_TYPE::PRINT: return parse_print(tokens);
    default: return parse_expression(tokens);
    }
  }();
  if (!parsed.has_value()) return parsed;
  std::tie(expr, tokens) = std::move(*parsed);
  bool const is_block = token.type != TOKEN_TYPE::LEFT_BRACE;
  if (is_block && !match<TOKEN_TYPE::SEMICOLON>(tokens))
  {
    return lox::error("Expected ';' after expression.", tokens.data()[-1].line);
  }
  return std::make_tuple(std::make_unique<Statement>(std::move(expr)), tokens.subspan(is_block));
}

auto parse_block(gsl::span<Token> tokens) -> parse_result
{
  if (!match<TOKEN_TYPE::LEFT_BRACE>(tokens))
  {
    return lox::error("Expected '{' token", tokens.data()[-1].line);
  }
  tokens = tokens.subspan(1);

  std::vector<std::unique_ptr<Expression>> exprs;

  while (tokens.size() && !match<TOKEN_TYPE::RIGHT_BRACE>(tokens))
  {
    exprs.emplace_back();
    // Attempt to parse a declaration
    auto parsed = parse_declaration(tokens);
    // If we failed, try to parse an expression, but this must be the final part of our block
    if (!parsed.has_value())
    {
      parsed = parse_expression(tokens);
      if (!parsed.has_value()) return parsed;
      std::tie(exprs.back(), tokens) = std::move(*parsed);
      break;
    }
    std::tie(exprs.back(), tokens) = std::move(*parsed);
  }

  if (!match<TOKEN_TYPE::RIGHT_BRACE>(tokens))
  {
    return lox::error("Expected '}' token", tokens.data()[-1].line);
  }

  return std::make_pair(std::make_unique<Block>(std::move(exprs)), tokens.subspan(1));
}

auto parse_print(gsl::span<Token> tokens) -> parse_result
{
  if (!match<TOKEN_TYPE::PRINT>(tokens))
  {
    return lox::error("Expected 'print' token", tokens.data()[-1].line);
  }
  tokens = tokens.subspan(1);
  std::unique_ptr<Expression> expr;
  {
    auto parsed = parse_expression(tokens);
    if (!parsed.has_value()) return parsed;
    std::tie(expr, tokens) = std::move(*parsed);
  }
  return std::make_tuple(std::make_unique<Print>(std::move(expr)), tokens);
}

auto parse_expression(gsl::span<Token> tokens) -> parse_result { return parse_list(tokens); }

auto parse_list(gsl::span<Token> tokens) -> parse_result
{
  return parse_recursive_binary<TOKEN_TYPE::COMMA>(tokens, parse_assignment);
}

auto parse_assignment(gsl::span<Token> tokens) -> parse_result
{
  auto lval = parse_ternary(tokens);
  if (!lval.has_value())
  {
    lval = parse_block(tokens);
    if (!lval.has_value()) return lox::error(lval.error());
  }

  std::unique_ptr<Expression> expr;
  std::tie(expr, tokens) = std::move(*lval);

  if (match<TOKEN_TYPE::ASSIGN>(tokens))
  {
    return parse_assignment(tokens.subspan(1)).and_then([&](auto&& value_tok) -> parse_result {
      std::unique_ptr<Expression> value;
      std::tie(value, tokens) = std::move(value_tok);
      if (auto tok = expr->is_lvalue())
      {
        return std::make_tuple(std::make_unique<Assign>(*tok, std::move(value)), tokens);
      }
      return lox::error("Cannot assign to an rvalue.", tokens.data()[-1].line);
    });
  }

  return std::make_pair(std::move(expr), tokens);
}

auto parse_ternary(gsl::span<Token> tokens) -> parse_result
{
  // Parse an initial equality
  std::unique_ptr<Expression> expr;
  {
    auto parsed = parse_equality(tokens);
    if (!parsed.has_value()) return parsed;
    std::tie(expr, tokens) = std::move(*parsed);
  }
  // Parse two expressions, implicit recursion allows each to also be a ternary
  if (match<TOKEN_TYPE::QUESTION>(tokens))
  {
    tokens = tokens.subspan(1);
    std::unique_ptr<Expression> left, right;
    auto parsed = parse_ternary(tokens)
                    .and_then([&](auto&& lhs) -> parse_result {
                      std::tie(left, tokens) = std::move(lhs);
                      if (match<TOKEN_TYPE::COLON>(tokens))
                        return parse_ternary(tokens.subspan(1));
                      else
                        return lox::error("Expected ':' in ternary expression.", tokens.data()[-1].line);
                    })
                    .map([&](auto&& rhs) { std::tie(right, tokens) = std::move(rhs); });
    if (!parsed.has_value()) return lox::error(parsed.error());
    expr = std::make_unique<Ternary>(std::move(expr), std::move(left), std::move(right));
  }
  // Return the expression tree head and the reduced token set
  return std::make_tuple(std::move(expr), tokens);
}

auto parse_equality(gsl::span<Token> tokens) -> parse_result
{
  return parse_recursive_binary<TOKEN_TYPE::BANG_EQUAL, TOKEN_TYPE::EQUAL>(tokens, parse_comparison);
}

auto parse_comparison(gsl::span<Token> tokens) -> parse_result
{
  return parse_recursive_binary<TOKEN_TYPE::GREATER,
                                TOKEN_TYPE::GREATER_EQUAL,
                                TOKEN_TYPE::LESS,
                                TOKEN_TYPE::LESS_EQUAL>(tokens, parse_addition);
}

auto parse_addition(gsl::span<Token> tokens) -> parse_result
{
  return parse_recursive_binary<TOKEN_TYPE::MINUS, TOKEN_TYPE::PLUS>(tokens, parse_multiplication);
}

auto parse_multiplication(gsl::span<Token> tokens) -> parse_result
{
  return parse_recursive_binary<TOKEN_TYPE::SLASH, TOKEN_TYPE::STAR>(tokens, parse_unary);
}

auto parse_unary(gsl::span<Token> tokens) -> parse_result
{
  if (match<TOKEN_TYPE::BANG, TOKEN_TYPE::MINUS>(tokens))
  {
    TOKEN_TYPE const operation = tokens[0].type;
    auto parsed = parse_unary(tokens.subspan(1));
    if (!parsed.has_value()) return parsed;
    std::unique_ptr<Expression> right;
    std::tie(right, tokens) = std::move(*parsed);
    return std::make_tuple(std::make_unique<Unary>(std::move(right), operation), tokens);
  }
  return parse_primary(tokens);
}

auto parse_primary(gsl::span<Token> tokens) -> parse_result
{
  if (tokens.empty())
  {
    return lox::error("Failed to parse primary expression from empty token stream.", ~0u);
  }
  auto const& token = tokens[0];
  tokens = tokens.subspan(1);
  switch (token.type)
  {
  case TOKEN_TYPE::TRUE: return std::make_tuple(std::make_unique<Literal>(true), tokens);
  case TOKEN_TYPE::FALSE: return std::make_tuple(std::make_unique<Literal>(false), tokens);
  case TOKEN_TYPE::NIL: return std::make_tuple(std::make_unique<Literal>(std::monostate{}), tokens);
  case TOKEN_TYPE::IDENTIFIER: return std::make_tuple(std::make_unique<Read>(token), tokens);
  case TOKEN_TYPE::NUMBER: [[fallthrough]];
  case TOKEN_TYPE::STRING:
  {
    return std::make_tuple(std::make_unique<Literal>(token.literal_value), tokens);
  }
  case TOKEN_TYPE::LEFT_PAREN:
  {
    auto parsed = parse_expression(tokens);
    if (!parsed.has_value()) return parsed;
    std::unique_ptr<Expression> expr;
    std::tie(expr, tokens) = std::move(*parsed);
    if (tokens.size() && tokens[0].type == TOKEN_TYPE::RIGHT_PAREN)
    {
      return std::make_tuple(std::make_unique<Group>(std::move(expr)), tokens.subspan(1));
    }
    else
    {
      return lox::error("Expected a closing ')' to match '('.", token.line);
    }
  }
  default:
  {
    return lox::error(
      fmt::format("Token type {} does not match the primary rule.", magic_enum::enum_name(token.type)),
      token.line);
  }
  }
}
}  // namespace lox
