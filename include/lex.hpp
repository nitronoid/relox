#pragma once
#if !defined(LOX_LEX_H)
#define LOX_LEX_H

#include "lox/error.hpp"
#include "lox/token.hpp"
#include <string_view>

namespace lox
{
auto lex(std::string_view source) -> lox::result<std::vector<Token>>;
}
#endif // LOX_LEX_H
