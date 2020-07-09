#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
#include <gsl/span>

#include "lox/lex.hpp"

namespace lox
{
// Forward decl
struct Binary;
struct Group;
struct Literal;
struct Unary;

struct AstVisitor
{
  virtual ~AstVisitor() = default;

  virtual void visit(Binary const&) = 0;
  virtual void visit(Group const&) = 0;
  virtual void visit(Literal const&) = 0;
  virtual void visit(Unary const&) = 0;
};

struct Expression
{
  virtual ~Expression() = default;
  virtual void accept(AstVisitor& visitor) const = 0;
};

template <typename T>
struct ExpressionBase : public Expression
{
  virtual ~ExpressionBase() = default;

  virtual void accept(AstVisitor& visitor) const override
  {
    visitor.visit(static_cast<T const&>(*this));
  }
};

struct Binary final : public ExpressionBase<Binary>
{
  Binary(std::unique_ptr<Expression> left,
         std::unique_ptr<Expression> right,
         TOKEN_TYPE op)
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

struct AstPrinter : public AstVisitor
{
  virtual void visit(Binary const& expr) override
  {
    parenthesize(magic_enum::enum_name(expr.m_op), *expr.m_left, *expr.m_right);
  }
  virtual void visit(Group const& expr) override
  {
    parenthesize("group", *expr.m_expression);
  }
  virtual void visit(Literal const& expr) override
  {
    struct ToString
    {
      auto operator()(std::string const& v) const -> std::string
      {
        return v;
      }
      auto operator()(float const& v) const -> std::string
      {
        return std::to_string(v);
      }
      auto operator()(std::monostate const&) const -> std::string
      {
        return "nil";
      }
    };
    m_ast += std::visit(ToString{}, expr.m_literal);
  }
  virtual void visit(Unary const& expr) override
  {
    parenthesize(magic_enum::enum_name(expr.m_op), *expr.m_expression);
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

namespace parse
{
//std::unique_ptr<Expression> comparison() {}
//std::unique_ptr<Expression> equality(TokenStream& stream)
//{
//  std::unique_ptr<Expression> expr = comparison();
//  while ()
//  {
//    Token const op = previous();
//    std::unique_ptr<Expression> right = comparison();
//    expr = std::make_unique<Binary>(std::move(expr), std::move(right), op);
//  }
//  return expr;
//}
//std::unique_ptr<Expression> expression()
//{
//  return equality();
//}
} // namespace parse

} // namespace lox

auto run(std::string_view source) -> lox::result<void>
{
  return lox::lex(source).map([](std::vector<lox::Token>&& tokens) {
    for (auto const& token : tokens)
    { fmt::print("{}\n", magic_enum::enum_name(token.type)); }
  });
}

auto run_file(std::filesystem::path file_path) -> lox::result<void>
{
  std::ifstream file(file_path);
  if (!file.is_open())
  {
    using namespace std::string_literals;
    return lox::error("Failed to open file."s, 0ul);
  }
  using source_iter = std::istreambuf_iterator<char>;
  std::string const source{source_iter(file), source_iter{}};
  return run(source);
}

auto run_prompt() -> lox::result<void>
{
  std::string line;
  // Exit loop with CTRL + C
  while (true)
  {
    fmt::print(">> ");
    if (std::getline(std::cin, line) && !line.empty())
    { run(line).map_error(lox::report); }
  }
  return lox::ok();
}

auto main(int argc, char* argv[]) -> int
{
#if 0
  std::unique_ptr<lox::Expression> expression = std::make_unique<lox::Binary>(
    std::make_unique<lox::Unary>(
      std::make_unique<lox::Literal>(lox::TokenStream::literal{123.f}),
      lox::TOKEN_TYPE::MINUS),
    std::make_unique<lox::Group>(
      std::make_unique<lox::Literal>(lox::TokenStream::literal{45.67f})),
    lox::TOKEN_TYPE::STAR);

  lox::AstPrinter visitor;
  expression->accept(visitor);
  fmt::print("{}\n", visitor.m_ast);
#else

  if (argc > 2)
  {
    fmt::print("Usage: lox [script]\n");
    return 64;
  }
  else if (argc == 2)
  {
    fmt::print("Running lox file: {}\n", argv[1]);
    // Report the error and the end the process
    run_file(argv[1]).map_error(lox::report).map_error([](auto&&) {
      std::exit(65);
    });
  }
  else
  {
    fmt::print("Running lox repl\n");
    run_prompt();
  }

#endif
  return EXIT_SUCCESS;
}
