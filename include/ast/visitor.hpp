#pragma once
#if !defined(LOX_AST_VISITOR_H)
#define LOX_AST_VISITOR_H

#include "lox/ast/expression_fwd.hpp"
#include "lox/error.hpp"

namespace lox
{
struct AstVisitor
{
	virtual ~AstVisitor() = default;

	virtual auto visit(Ternary const&) -> result<void> = 0;
	virtual auto visit(Binary const&) -> result<void> = 0;
	virtual auto visit(Group const&) -> result<void> = 0;
	virtual auto visit(Literal const&) -> result<void> = 0;
	virtual auto visit(Unary const&) -> result<void> = 0;
};
}  // namespace lox

#endif  // LOX_AST_VISITOR_H
