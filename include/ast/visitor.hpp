#pragma once
#if !defined(LOX_AST_VISITOR_H)
#define LOX_AST_VISITOR_H

#include "lox/ast/expression_fwd.hpp"

namespace lox
{
struct AstVisitor
{
	virtual ~AstVisitor() = default;

	virtual auto visit(Ternary const&) -> void = 0;
	virtual auto visit(Binary const&) -> void = 0;
	virtual auto visit(Group const&) -> void = 0;
	virtual auto visit(Literal const&) -> void = 0;
	virtual auto visit(Unary const&) -> void = 0;
};
}  // namespace lox

#endif  // LOX_AST_VISITOR_H
