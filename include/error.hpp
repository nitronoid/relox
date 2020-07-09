#pragma once
#if !defined(LOX_ERROR_H)
#define LOX_ERROR_H

#include <string>
#include <tl/expected.hpp>

namespace lox
{
struct Error final
{
  std::string message;
  std::size_t line;
};

auto report(Error const& error) -> void;

template <typename T>
using result = tl::expected<T, lox::Error>;

template <typename... Ts>
inline auto error(Ts&&... xs) noexcept
{
  return tl::make_unexpected(Error{std::forward<Ts>(xs)...});
}

inline auto ok() noexcept -> result<void>
{
  return {};
}
}
#endif // LOX_ERROR_H
