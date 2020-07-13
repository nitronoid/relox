#pragma once
#if !defined(LOX_AST_INTERPRETER_H)
#define LOX_AST_INTERPRETER_H

#include <fmt/format.h>

#include <cassert>
#include <magic_enum/magic_enum.hpp>

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
			throw std::bad_variant_access{};
		}
		Token::literal const* lhs;
	};

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
		auto const evaluated =
		    expr.m_left->accept(*this)
		        .and_then([&] {
			        // Cache the result
			        auto const lhs = result;
			        // Exec the rhs
			        return expr.m_right->accept(*this).map([&] { return lhs; });
		        })
		        .and_then([&](auto&& lhs) -> lox::result<Token::literal> {
			        try
			        {
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
						        return lox::error(fmt::format("Mismatched types for {} expression.",
						                                      magic_enum::enum_name(expr.m_op)),
						                          ~0u);
					        }
				        case TOKEN_TYPE::MINUS: return std::get<float>(lhs) - std::get<float>(result);
				        case TOKEN_TYPE::STAR: return std::get<float>(lhs) * std::get<float>(result);
				        case TOKEN_TYPE::SLASH: return std::get<float>(lhs) / std::get<float>(result);
				        case TOKEN_TYPE::GREATER: return std::get<float>(lhs) > std::get<float>(result);
				        case TOKEN_TYPE::GREATER_EQUAL:
					        return std::get<float>(lhs) >= std::get<float>(result);
				        case TOKEN_TYPE::LESS: return std::get<float>(lhs) < std::get<float>(result);
				        case TOKEN_TYPE::LESS_EQUAL:
					        return std::get<float>(lhs) <= std::get<float>(result);
				        case TOKEN_TYPE::BANG_EQUAL: return lhs != result;
				        case TOKEN_TYPE::EQUAL: return lhs == result;
				        default:
					        return lox::error("Unhandled binary op. FIXME: Error handle this properly",
					                          ~0u);
				        }
			        }
			        catch (std::bad_variant_access const&)
			        {
				        return lox::error(fmt::format("Expected number as operand to {}.",
				                                      magic_enum::enum_name(expr.m_op)),
				                          ~0u);
			        }
		        });
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
		auto const evaluated =
		    expr.m_expression->accept(*this).and_then([&]() -> lox::result<Token::literal> {
			    try
			    {
				    switch (expr.m_op)
				    {
				    case TOKEN_TYPE::MINUS: return -std::get<float>(result);
				    case TOKEN_TYPE::BANG: return !std::visit(Truth{}, result);
				    default:
					    return lox::error(
					        fmt::format("Unhandled unary op {}.", magic_enum::enum_name(expr.m_op)),
					        ~0u);
				    }
			    }
			    catch (std::bad_variant_access const&)
			    {
				    return lox::error(fmt::format("Expected number as operand to {}.",
				                                  magic_enum::enum_name(expr.m_op)),
				                      ~0u);
			    }
		    });
		if (evaluated)
		{
			result = *evaluated;
			return lox::ok();
		}
		return lox::error(evaluated.error());
	}

	Token::literal result;
};
}  // namespace lox


#endif  // LOX_AST_INTERPRETER_H
