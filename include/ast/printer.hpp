#pragma once
#if !defined(LOX_AST_PRINTER_H)
#define LOX_AST_PRINTER_H

#include <magic_enum/magic_enum.hpp>

#include "lox/ast/expression.hpp"
#include "lox/literal_to_string.hpp"

namespace lox
{
struct AstPrinter final : public AstVisitor
{
  virtual auto visit(Definition const& expr) -> result<void> override
  {
    parenthesize(fmt::format("Definition {}", expr.m_name.lexeme), *expr.m_value);
    return lox::ok();
  }
  virtual auto visit(Read const& expr) -> result<void> override
  {
    parenthesize(fmt::format("Read {}", expr.m_name.lexeme));
    return lox::ok();
  }
  virtual auto visit(Statement const& expr) -> result<void> override
  {
    parenthesize("Statement", *expr.m_expression);
    return lox::ok();
  }
  virtual auto visit(Block const& expr) -> result<void> override
  {
    m_ast += "(Block";
    for (auto const& e : expr.m_expressions)
    {
        m_ast += " ";
        e->accept(*this);
    }
    m_ast += ")";
    return lox::ok();
  }
  virtual auto visit(Print const& expr) -> result<void> override
  {
    parenthesize("Print", *expr.m_value);
    return lox::ok();
  }
  virtual auto visit(Assign const& expr) -> result<void> override
  {
    parenthesize(fmt::format("Assign {}", expr.m_name.lexeme), *expr.m_value);
    return lox::ok();
  }
  virtual auto visit(Ternary const& expr) -> result<void> override
  {
    parenthesize("TERNARY", *expr.m_cond, *expr.m_left, *expr.m_right);
    return lox::ok();
  }
  virtual auto visit(Binary const& expr) -> result<void> override
  {
    parenthesize(magic_enum::enum_name(expr.m_op), *expr.m_left, *expr.m_right);
    return lox::ok();
  }
  virtual auto visit(Group const& expr) -> result<void> override
  {
    parenthesize("group", *expr.m_expression);
    return lox::ok();
  }
  virtual auto visit(Literal const& expr) -> result<void> override
  {
    m_ast += std::visit(LiteralToString{}, expr.m_literal);
    return lox::ok();
  }
  virtual auto visit(Unary const& expr) -> result<void> override
  {
    parenthesize(magic_enum::enum_name(expr.m_op), *expr.m_expression);
    return lox::ok();
  }

  template <typename... Ts>
  auto parenthesize(std::string_view name, Ts&&... exprs) -> void
  {
    m_ast += "(";
    m_ast += name;
    // Concatenate all expressions, space delimited
    (
      [this](auto&& e) {
        m_ast += " ";
        std::forward<Ts>(e).accept(*this);
      }(std::forward<Ts>(exprs)),
      ...);
    m_ast += ")";
  }
  std::string m_ast;
};
}  // namespace lox

#endif  // LOX_AST_PRINTER_H

