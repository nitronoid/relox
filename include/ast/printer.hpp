#pragma once
#if !defined(LOX_PRINTER_H)
#define LOX_PRINTER_H

#include <magic_enum/magic_enum.hpp>

#include "lox/ast/expression.hpp"

namespace lox
{
struct AstPrinter : public AstVisitor
{
	virtual void visit(Binary const& expr) override
	{
		parenthesize(magic_enum::enum_name(expr.m_op), *expr.m_left, *expr.m_right);
	}
	virtual void visit(Group const& expr) override { parenthesize("group", *expr.m_expression); }
	virtual void visit(Literal const& expr) override
	{
		struct ToString
		{
			auto operator()(std::string const& v) const -> std::string { return v; }
			auto operator()(float const& v) const -> std::string { return std::to_string(v); }
			auto operator()(bool const& v) const -> std::string { return v ? "true" : "false"; }
			auto operator()(std::monostate const&) const -> std::string { return "nil"; }
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
}  // namespace lox

#endif  // LOX_PRINTER_H

