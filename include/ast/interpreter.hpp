#pragma once
#if !defined(LOX_AST_INTERPRETER_H)
#define LOX_AST_INTERPRETER_H

#include <fmt/format.h>

#include <cassert>
#include <magic_enum/magic_enum.hpp>

#include "lox/ast/expression.hpp"
#include "lox/environment.hpp"
#include "lox/literal_to_string.hpp"

namespace lox
{
struct Interpreter final : public AstVisitor
{
  struct Truth
  {
    auto operator()(std::string const&) const -> bool { return true; }
    auto operator()(float const&) const -> bool { return true; }
    auto operator()(bool const& v) const -> bool { return v; }
    auto operator()(std::monostate const&) const -> bool { return false; }
  };

  struct Add
  {
    auto operator()(std::string const& v) const -> Token::literal
    {
      return std::visit(LiteralToString{}, *lhs) + v;
    }
    auto operator()(float const& v) const -> Token::literal
    {
      if (std::holds_alternative<std::string>(*lhs))
      {
        return std::get<std::string>(*lhs) + std::to_string(v);
      }
      return std::get<float>(*lhs) + v;
    }
    template <typename T>
    auto operator()(T const&) const -> Token::literal
    {
      throw std::bad_variant_access{};
    }
    Token::literal const* lhs;
  };

  virtual auto visit(Definition const& expr) -> result<void> override
  {
    if (auto value = expr.m_value->accept(*this); !value.has_value()) return value;

    environment.define(Key{std::string{expr.m_name.lexeme}}, Environment::Value{result});
    return lox::ok();
  }

  virtual auto visit(Read const& expr) -> result<void> override
  {
    auto value = environment.lookup(Key{std::string{expr.m_name.lexeme}});
    if (!value.has_value()) return lox::error(value.error());
    result = (*value)->value;
    return lox::ok();
  }

  virtual auto visit(Statement const& stmt) -> result<void> override
  {
    // Evaluate the condition
    if (auto res = stmt.m_expression->accept(*this); !res.has_value())
    {
      return lox::error(res.error());
    }
    result = std::monostate{};
    return lox::ok();
  }

  virtual auto visit(Print const& stmt) -> result<void> override
  {
    // Evaluate the condition
    if (auto res = stmt.m_value->accept(*this); !res.has_value())
    {
      return lox::error(res.error());
    }
    fmt::print("{}\n", result);
    result = std::monostate{};
    return lox::ok();
  }

  virtual auto visit(Assign const& expr) -> result<void> override
  {
    if (auto value = expr.m_value->accept(*this); !value.has_value()) return value;
    return environment.assign(Key{std::string{expr.m_name.lexeme}}, Environment::Value{result});
  }

  virtual auto visit(Ternary const& expr) -> result<void> override
  {
    // Evaluate the condition
    return expr.m_cond->accept(*this).and_then([&] {
      // Conditionally evaluate one of the branches
      if (std::visit(Truth{}, result))
        return expr.m_left->accept(*this);
      else
        return expr.m_right->accept(*this);
    });
  }

  virtual auto visit(Binary const& expr) -> result<void> override
  {
    auto const mismatched_type_error = [&] {
      return lox::error(
        fmt::format("Mismatched types for {} expression.", magic_enum::enum_name(expr.m_op)), ~0u);
    };
    auto const matched_binary =
      [&](auto const& lhs, auto const& rhs, auto op) -> lox::result<Token::literal> {
      if (lhs.index() == rhs.index())
      {
        return std::invoke(op, lhs, rhs);
      }
      return mismatched_type_error();
    };
    auto const float_binary =
      [&](auto const& lhs, auto const& rhs, auto op) -> lox::result<Token::literal> {
      if (std::holds_alternative<float>(lhs) && std::holds_alternative<float>(rhs))
      {
        return std::invoke(op, std::get<float>(lhs), std::get<float>(rhs));
      }
      return lox::error(
        fmt::format("Expected number operands for {} expression.", magic_enum::enum_name(expr.m_op)),
        ~0u);
    };
    auto const compute_rhs = [&] {
      // Cache the result
      auto const lhs = result;
      // Exec the rhs
      return expr.m_right->accept(*this).map([&] { return lhs; });
    };
    auto const compute_result = [&](auto&& lhs) -> lox::result<Token::literal> {
      // Apply the binary op to both operands
      switch (expr.m_op)
      {
      case TOKEN_TYPE::PLUS:
        try
        {
          return std::visit(Add{&lhs}, result);
        }
        catch (std::bad_variant_access const&)
        {
          return mismatched_type_error();
        }
      case TOKEN_TYPE::MINUS: return float_binary(lhs, result, std::minus<>{});
      case TOKEN_TYPE::STAR: return float_binary(lhs, result, std::multiplies<>{});
      case TOKEN_TYPE::SLASH:
      {
        if (std::holds_alternative<float>(result) && std::get<float>(result) == 0.f)
        {
          return lox::error("Division by zero is prohibited.", ~0u);
        }
        return float_binary(lhs, result, std::divides<>{});
      }
      case TOKEN_TYPE::GREATER: return matched_binary(lhs, result, std::greater<>{});
      case TOKEN_TYPE::GREATER_EQUAL: return matched_binary(lhs, result, std::greater_equal<>{});
      case TOKEN_TYPE::LESS: return matched_binary(lhs, result, std::less<>{});
      case TOKEN_TYPE::LESS_EQUAL: return matched_binary(lhs, result, std::less_equal<>{});
      case TOKEN_TYPE::BANG_EQUAL: return lhs != result;
      case TOKEN_TYPE::EQUAL: return lhs == result;
      case TOKEN_TYPE::COMMA: return result;  // Discard the left hand side
      default: return lox::error("Unhandled binary op. FIXME: Error handle this properly", ~0u);
      }
    };
    auto const evaluated = expr.m_left->accept(*this).and_then(compute_rhs).and_then(compute_result);
    if (evaluated)
    {
      result = *evaluated;
      return lox::ok();
    }
    return lox::error(evaluated.error());
  }

  virtual auto visit(Group const& expr) -> result<void> override
  {
    return expr.m_expression->accept(*this);
  }

  virtual auto visit(Literal const& expr) -> result<void> override
  {
    result = expr.m_literal;
    return lox::ok();
  }

  virtual auto visit(Unary const& expr) -> result<void> override
  {
    auto const compute_result = [&]() -> lox::result<Token::literal> {
      switch (expr.m_op)
      {
      case TOKEN_TYPE::MINUS:
      {
        if (std::holds_alternative<float>(result))
        {
          return -std::get<float>(result);
        }
        return lox::error(
          fmt::format("Expected number as operand to {}.", magic_enum::enum_name(expr.m_op)), ~0u);
      }
      case TOKEN_TYPE::BANG: return !std::visit(Truth{}, result);
      default:
        return lox::error(fmt::format("Unhandled unary op {}.", magic_enum::enum_name(expr.m_op)), ~0u);
      }
    };
    auto const evaluated = expr.m_expression->accept(*this).and_then(compute_result);
    if (evaluated)
    {
      result = *evaluated;
      return lox::ok();
    }
    return lox::error(evaluated.error());
  }

  Environment environment;
  Token::literal result;
};
}  // namespace lox


#endif  // LOX_AST_INTERPRETER_H
