#include "lox/error.hpp"
#include <fmt/format.h>

namespace lox
{
auto report(Error const& error) -> void
{
  fmt::print("[line {0}] Error {1}: {2}\n", error.line, "", error.message);
}
}
