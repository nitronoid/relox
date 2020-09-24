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
  constexpr bool operator==(Key const& rhs) const noexcept
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
  std::size_t operator()(lox::Key const& key) const
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

  void update(Key key, Value value)
  {
    values[std::move(key)] = std::move(value);
  }

  result<Value*> lookup(Key const& key)
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
