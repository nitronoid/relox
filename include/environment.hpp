#pragma once
#if !defined(LOX_ENVIRONMENT_H)
#define LOX_ENVIRONMENT_H

#include <string_view>
#include <unordered_map>
#include <fmt/format.h>
#include "lox/error.hpp"
#include "lox/token.hpp"

namespace lox
{
struct Key
{
  std::string_view name;
  constexpr auto operator==(Key const& rhs) const noexcept -> bool
  {
    return name == rhs.name;
  }
};
}

namespace std
{
template<>
struct hash<lox::Key>
{
  auto operator()(lox::Key const& key) const -> std::size_t
  {
    return std::hash<std::string_view>{}(key.name);
  }
};
}


namespace lox
{
struct Environment
{
  struct Value
  {
    Token::literal value;
  };
  std::unordered_map<Key, Value> values;

  auto define(Key key, Value value) -> void
  {
    values[std::move(key)] = std::move(value);
  }

  auto assign(Key key, Value value) -> result<void>
  {
    if (!values.count(key))
    {
      return lox::error(fmt::format("Undefined variable '{}'.", key.name), ~0u);
    }
    define(std::move(key), std::move(value));
    return lox::ok();
  }

  auto lookup(Key const& key) -> result<Value*>
  {
    auto const it = values.find(key);
    if (it == values.end())
    {
      return lox::error(fmt::format("Undefined variable '{}'.", key.name), ~0u);
    }
    return &it->second;
  }
};
}

#endif // LOX_ENVIRONMENT_H
