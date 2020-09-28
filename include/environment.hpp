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
  std::string name;
  auto operator==(Key const& rhs) const -> bool
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
    return std::hash<std::string>{}(key.name);
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

  auto current_scope() -> std::unordered_map<Key, Value>&
  {
    return scopes.back();
  }

  auto lookup(Key const& key) -> result<Value*>
  {
    // Reverse search through the scopes until the key is found
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      // If the scope has our key, return the associated value
      if (auto val = it->find(key); val != it->end()) return &val->second;
    }
    return lox::error(fmt::format("Undefined variable '{}'.", key.name), ~0u);
  }

  auto define(Key const& key, Value const& value) -> void
  {
    current_scope()[key] = value;
  }

  auto assign(Key const& key, Value const& value) -> result<void>
  {
    auto val = lookup(key);
    if (!val) return lox::error(val.error());
    **val = value;
    return lox::ok();
  }

  // Default construct with a single scope
  std::vector<std::unordered_map<Key, Value>> scopes{{}};
};
}

#endif // LOX_ENVIRONMENT_H
