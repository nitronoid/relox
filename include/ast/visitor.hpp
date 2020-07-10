#pragma once
#if !defined(LOX_VISITOR_H)
#define LOX_VISITOR_H

#include "lox/ast/expression_fwd.hpp"

namespace lox
{
struct AstVisitor
{
	virtual ~AstVisitor() = default;

	virtual void visit(Binary const&) = 0;
	virtual void visit(Group const&) = 0;
	virtual void visit(Literal const&) = 0;
	virtual void visit(Unary const&) = 0;
};
}  // namespace lox

#endif  // LOX_VISITOR_H
