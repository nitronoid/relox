#include "lox/lex.hpp"

#include <fmt/format.h>

#include <charconv>
#include <ctre/ctre.hpp>
#include <magic_enum/magic_enum.hpp>

#include "lox/ctu.hpp"

namespace lox
{
namespace pattern
{
template <TOKEN_TYPE>
constexpr std::string_view match = "";
// clang-format off
template<> constexpr std::string_view match<TOKEN_TYPE::COMMENT> = R"((?://[^\n]*)|(?:/\*[^*]*\*+(?:[^/*][^*]*\*+)*/))";
template<> constexpr std::string_view match<TOKEN_TYPE::LEFT_PAREN> = R"(\()";
template<> constexpr std::string_view match<TOKEN_TYPE::RIGHT_PAREN> = R"(\))";
template<> constexpr std::string_view match<TOKEN_TYPE::LEFT_BRACE> = R"(\{)";
template<> constexpr std::string_view match<TOKEN_TYPE::RIGHT_BRACE> = R"(\})";
template<> constexpr std::string_view match<TOKEN_TYPE::LEFT_BRACKET> = R"(\[)";
template<> constexpr std::string_view match<TOKEN_TYPE::RIGHT_BRACKET> = R"(\])";
template<> constexpr std::string_view match<TOKEN_TYPE::COMMA> = R"(,)";
template<> constexpr std::string_view match<TOKEN_TYPE::DOT> = R"(\.)";
template<> constexpr std::string_view match<TOKEN_TYPE::MINUS> = R"(\-)";
template<> constexpr std::string_view match<TOKEN_TYPE::PLUS> = R"(\+)";
template<> constexpr std::string_view match<TOKEN_TYPE::SEMICOLON> = R"(;)";
template<> constexpr std::string_view match<TOKEN_TYPE::SLASH> = R"(/)";
template<> constexpr std::string_view match<TOKEN_TYPE::STAR> = R"(\*)";
template<> constexpr std::string_view match<TOKEN_TYPE::BANG_EQUAL> = R"(!=)";
template<> constexpr std::string_view match<TOKEN_TYPE::BANG> = R"(!)";
template<> constexpr std::string_view match<TOKEN_TYPE::EQUAL> = R"(==)";
template<> constexpr std::string_view match<TOKEN_TYPE::GREATER_EQUAL> = R"(>=)";
template<> constexpr std::string_view match<TOKEN_TYPE::LESS_EQUAL> = R"(<=)";
template<> constexpr std::string_view match<TOKEN_TYPE::GREATER> = R"(>)";
template<> constexpr std::string_view match<TOKEN_TYPE::LESS> = R"(<)";
template<> constexpr std::string_view match<TOKEN_TYPE::ASSIGN> = R"(=)";
template<> constexpr std::string_view match<TOKEN_TYPE::AND> = R"(and(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::STRUCT> = R"(struct(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::ELSE> = R"(else(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::FUN> = R"(fun(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::FOR> = R"(for(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::IF> = R"(if(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::NIL> = R"(nil(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::OR> = R"(or(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::PRINT> = R"(print(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::RETURN> = R"(return(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::SUPER> = R"(super(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::THIS> = R"(this(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::TRUE> = R"(true(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::FALSE> = R"(false(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::VAR> = R"(var(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::WHILE> = R"(while(?=\W|$))";
template<> constexpr std::string_view match<TOKEN_TYPE::IDENTIFIER> = R"([a-zA-Z_]+\w*)";
template<> constexpr std::string_view match<TOKEN_TYPE::STRING> = R"("[^"]*")";
template<> constexpr std::string_view match<TOKEN_TYPE::NUMBER> = R"([0-9]+(?:\.[0-9]+)?)";
template<> constexpr std::string_view match<TOKEN_TYPE::ERROR> = R"([^\s]+)";
// clang-format on

template <std::string_view const& S1, std::string_view const&... Strs>
struct PatternGenerator
{
	static constexpr std::string_view delim = "|";
	static constexpr std::string_view group_open = "(";
	static constexpr std::string_view group_close = ")";
	static constexpr std::string_view terminate = "\0";

	// Group the first string, and then join all other strings in their own
	// groups, separated by a delimiter and finally followed by a null terminator
	static constexpr ctll::fixed_string value =
	    ctu::join_l<group_open,
	                S1,
	                group_close,
	                ctu::join_v<delim, group_open, Strs, group_close>...,
	                terminate>::value;
};

template <typename>
struct make_pattern;
template <std::size_t... I>
struct make_pattern<std::index_sequence<I...>>
{
	static constexpr auto value =
	    PatternGenerator<match<magic_enum::enum_value<TOKEN_TYPE>(I)>...>::value;
};

static constexpr auto full =
    make_pattern<std::make_index_sequence<magic_enum::enum_count<TOKEN_TYPE>() - 1>>::value;
}  // namespace pattern

namespace impl
{
template <typename F, typename T, std::size_t... I>
constexpr decltype(auto) indexed_apply(F&& f, T&& t, std::index_sequence<I...>)
{
	return std::invoke(std::forward<F>(f),
	                   std::make_tuple(std::forward<T>(t).template get<I>(),
	                                   std::integral_constant<std::size_t, I>{})...);
}
}  // namespace impl

template <typename F, typename T>
constexpr decltype(auto) indexed_apply(F&& f, T&& t)
{
	return impl::indexed_apply(
	    std::forward<F>(f),
	    std::forward<T>(t),
	    std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});
}

template <TOKEN_TYPE>
struct ParseLiteral
{
	constexpr auto operator()(std::string_view) const -> std::monostate { return std::monostate{}; }
};
template <>
struct ParseLiteral<TOKEN_TYPE::NUMBER>
{
	auto operator()(std::string_view src) const -> float { return std::stof(std::string{src}); }
};
template <>
struct ParseLiteral<TOKEN_TYPE::STRING>
{
	auto operator()(std::string_view src) const -> std::string { return std::string{src}; }
};
template <>
struct ParseLiteral<TOKEN_TYPE::TRUE>
{
	auto operator()(std::string_view) const -> bool { return true; }
};
template <>
struct ParseLiteral<TOKEN_TYPE::FALSE>
{
	auto operator()(std::string_view) const -> bool { return false; }
};

auto lex_token(std::string_view src, std::size_t line) -> lox::result<Token>
{
	// Default to an EOF
	lox::result<Token> token = Token{TOKEN_TYPE::END, src, line, std::monostate{}};
	// Attempt to match our grammar, find a match if available
	auto const extract_match = [&](auto const& match_group, auto i) {
		// Index zero is a full match, which we are not interested in
		if (i.value == 0 || !match_group) return;
		// Build a new token from this match groups token type
		constexpr auto type = static_cast<TOKEN_TYPE>(i.value - 1);
		token = Token{type, match_group.to_view(), line, ParseLiteral<type>{}(match_group.to_view())};
	};
	// Helper to apply the matcher to each group, index pair with a fold expression
	auto const fwd = [&](auto&&... ms) { (std::apply(extract_match, ms), ...); };
	// Apply our matcher to each match group, with its group index
	indexed_apply(fwd, ctre::search<pattern::full>(src));
	return token;
}

auto lex(std::string_view source) -> lox::result<std::vector<Token>>
{
	// Build this token list
	std::vector<Token> tokens;
	// Keep track of the line we're processing
	std::size_t line = 0;
	// Consume until we're out of input characters
	while (source.size())
	{
		// Try to lex the next token
		if (auto lexed = lex_token(source, line); lexed.has_value())
		{
			// Advance past the source for this lexeme
			source = source.substr(&(*lexed).lexeme.back() + 1 - source.data());
			line = (*lexed).line;
			// Add the token to our stream
			tokens.emplace_back(std::move(*lexed));
		}
		else
		{
			return lox::error(lexed.error());
		}
	}
	return tokens;
}
}  // namespace lox
