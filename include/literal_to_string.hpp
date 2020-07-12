#pragma once
#if !defined(LOX_LITERAL_TO_STRING_H)
#define LOX_LITERAL_TO_STRING_H

#include <fmt/format.h>

#include "lox/token.hpp"

namespace lox
{
struct LiteralToString
{
	auto operator()(std::string const& v) const -> std::string { return fmt::format("'{}'", v); }
	auto operator()(float const& v) const -> std::string { return std::to_string(v); }
	auto operator()(bool const& v) const -> std::string { return v ? "true" : "false"; }
	auto operator()(std::monostate const&) const -> std::string { return "nil"; }
};
}  // namespace lox
#endif  // LOX_LITERAL_TO_STRING_H
