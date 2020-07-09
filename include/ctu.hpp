#pragma once
#if !defined(COMPILE_TIME_UTILS_H)
#define COMPILE_TIME_UTILS_H

#include <array>
#include <string_view>

namespace ctu
{
template <std::string_view const&... Strs>
struct join
{
	// Helper to get a string literal from a std::array
	template <std::size_t N, std::array<char, N> const& S, typename>
	struct to_char_array;
	template <std::size_t N, std::array<char, N> const& S, std::size_t... I>
	struct to_char_array<N, S, std::index_sequence<I...>>
	{
		static constexpr const char value[]{S[I]..., 0};
	};
	// Join all strings into a single std::array of chars
	static constexpr auto impl() noexcept
	{
		constexpr std::size_t len = (Strs.size() + ... + 0);
		std::array<char, len> arr{};
		auto append = [i = 0, &arr](auto const& s) mutable {
			for (auto c : s) arr[i++] = c;
		};
		(append(Strs), ...);
		return arr;
	}
	// Give the joined string static storage
	static constexpr auto arr = impl();
	// Convert to a string literal, then view as a std::string_view
	using literal = to_char_array<arr.size(), arr, std::make_index_sequence<arr.size()>>;
	static constexpr std::string_view value = literal::value;
};
// Helper to get the value out
template <std::string_view const&... Strs>
static constexpr auto join_v = join<Strs...>::value;
// Helpers to get a string literal out
template <std::string_view const&... Strs>
using join_l = typename join<Strs...>::literal;
} // namespace ctu

#endif // COMPILE_TIME_UTILS_H
