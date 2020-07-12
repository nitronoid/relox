#pragma once
#if !defined(LOX_AST_INTERPRETER_H)
#define LOX_AST_INTERPRETER_H

#include <cassert>

#include "lox/ast/expression.hpp"

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
			return std::get<std::string>(*lhs) + v;
		}
		auto operator()(float const& v) const -> Token::literal { return std::get<float>(*lhs) + v; }
		template <typename T>
		auto operator()(T const&) const -> Token::literal
		{
			return std::monostate{};
		}
		Token::literal const* lhs;
	};
	virtual auto visit(Ternary const& expr) -> void override
	{
		// Evaluate the condition
		expr.m_cond->accept(*this);
		// Conditionally evaluate one of the branches
		if (std::visit(Truth{}, result))
			expr.m_left->accept(*this);
		else
			expr.m_right->accept(*this);
	}
	virtual auto visit(Binary const& expr) -> void override
	{
		expr.m_left->accept(*this);
		// Cache the result
		auto const lhs = result;
		// Exec the rhs
		expr.m_right->accept(*this);

		// Apply the binary op to both operands
		result = [&]() -> Token::literal {
			switch (expr.m_op)
			{
			case TOKEN_TYPE::PLUS: return std::visit(Add{&lhs}, result);
			case TOKEN_TYPE::MINUS: return std::get<float>(lhs) - std::get<float>(result);
			case TOKEN_TYPE::STAR: return std::get<float>(lhs) * std::get<float>(result);
			case TOKEN_TYPE::SLASH: return std::get<float>(lhs) / std::get<float>(result);
			case TOKEN_TYPE::GREATER: return std::get<float>(lhs) > std::get<float>(result);
			case TOKEN_TYPE::GREATER_EQUAL: return std::get<float>(lhs) >= std::get<float>(result);
			case TOKEN_TYPE::LESS: return std::get<float>(lhs) < std::get<float>(result);
			case TOKEN_TYPE::LESS_EQUAL: return std::get<float>(lhs) <= std::get<float>(result);
			case TOKEN_TYPE::BANG_EQUAL: return lhs != result;
			case TOKEN_TYPE::EQUAL: return lhs == result;
			default: assert("Unhandled binary op. FIXME: Error handle this properly");
			}
			return std::monostate{};
		}();
	}
	virtual auto visit(Group const& expr) -> void override { expr.m_expression->accept(*this); }
	virtual auto visit(Literal const& expr) -> void override { result = expr.m_literal; }
	virtual auto visit(Unary const& expr) -> void override
	{
		expr.m_expression->accept(*this);
		result = [&]() -> Token::literal {
			switch (expr.m_op)
			{
			case TOKEN_TYPE::MINUS: return -std::get<float>(result);
			case TOKEN_TYPE::BANG:
			{
				return !std::visit(Truth{}, result);
			}
			default: assert("Unhandled unary op. FIXME: Error handle this properly");
			}
			return std::monostate{};
		}();
	}

	Token::literal result;
};
}  // namespace lox


#endif  // LOX_AST_INTERPRETER_H
