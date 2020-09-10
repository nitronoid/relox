#pragma once
#if !defined(LOX_AST_EXPRESSION_H)
#define LOX_AST_EXPRESSION_H

#include <memory>

#include "lox/ast/visitor.hpp"
#include "lox/token.hpp"

namespace lox
{
struct Expression
{
  virtual ~Expression() = default;
  virtual auto accept(AstVisitor& visitor) const -> result<void> = 0;
};

template <typename T>
struct ExpressionBase : public Expression
{
  virtual ~ExpressionBase() = default;

  virtual auto accept(AstVisitor& visitor) const -> result<void> override
  {
    return visitor.visit(static_cast<T const&>(*this));
  }
};

struct Ternary final : public ExpressionBase<Ternary>
{
  Ternary(std::unique_ptr<Expression> cond,
          std::unique_ptr<Expression> left,
          std::unique_ptr<Expression> right)
    : m_cond(std::move(cond)), m_left(std::move(left)), m_right(std::move(right))
  {
  }
  std::unique_ptr<Expression> m_cond;
  std::unique_ptr<Expression> m_left;
  std::unique_ptr<Expression> m_right;
};

struct Binary final : public ExpressionBase<Binary>
{
  Binary(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, TOKEN_TYPE op)
    : m_left(std::move(left)), m_right(std::move(right)), m_op(std::move(op))
  {
  }
  std::unique_ptr<Expression> m_left;
  std::unique_ptr<Expression> m_right;
  TOKEN_TYPE m_op;
};

struct Group final : public ExpressionBase<Group>
{
  Group(std::unique_ptr<Expression> expr) : m_expression(std::move(expr)) {}
  std::unique_ptr<Expression> m_expression;
};

struct Literal final : public ExpressionBase<Literal>
{
  template <typename T>
  Literal(T&& val) : m_literal(std::forward<T>(val))
  {
  }
  Token::literal m_literal;
};

struct Unary final : public ExpressionBase<Unary>
{
  Unary(std::unique_ptr<Expression> expr, TOKEN_TYPE op)
    : m_expression(std::move(expr)), m_op(std::move(op))
  {
  }
  std::unique_ptr<Expression> m_expression;
  TOKEN_TYPE m_op;
};
}  // namespace lox

#endif  // LOX_AST_EXPRESSION_H

