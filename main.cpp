#include <fmt/format.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gsl/span>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <structopt/app.hpp>

#include "lox/ast/expression.hpp"
#include "lox/ast/interpreter.hpp"
#include "lox/ast/parse.hpp"
#include "lox/ast/printer.hpp"
#include "lox/lex.hpp"


struct Options
{
  // Optional name of file to execute
  std::optional<std::string> script;

  // Display the program ast as a tree
  std::optional<bool> ast_dump = false;

  // Display the list of tokens which comprise the program
  std::optional<bool> token_dump = false;

  // Display the list of tokens which comprise the program
  std::optional<bool> immediate_result_dump = false;
};
STRUCTOPT(Options, script, ast_dump, token_dump, immediate_result_dump);


struct DisplaySettings
{
  bool ast_dump = false;
  bool token_dump = false;
  bool immediate_result = false;
};

auto run(std::string_view source, lox::Interpreter* interpreter, DisplaySettings const& display)
  -> lox::result<void>
{
  return lox::lex(source)
    .map([=](auto&& tokens) {
      if (display.token_dump)
      {
        for (auto const& token : tokens) fmt::print("{}\n", magic_enum::enum_name(token.type));
      }
      tokens.erase(
        std::remove_if(tokens.begin(),
                       tokens.end(),
                       [](auto const& token) { return token.type == lox::TOKEN_TYPE::COMMENT; }),
        tokens.end());
      return std::move(tokens);
    })
    .and_then([](auto&& tokens) { return lox::parse(tokens); })
    .map([=](auto const& parsed) {
      // Print the expression tree
      for (auto const& expr : parsed)
      {
        if (display.ast_dump)
        {
          lox::AstPrinter printer;
          expr->accept(printer).map([&] { fmt::print("{}\n", printer.m_ast); }).map_error(lox::report);
        }
        // Evaluate the expression tree
        expr->accept(*interpreter)
          .map([&] {
            if (display.immediate_result) fmt::print("{}\n", interpreter->result);
          })
          .map_error(lox::report);
      }
    });
}

auto run_file(std::filesystem::path file_path, DisplaySettings const& display) -> lox::result<void>
{
  std::ifstream file(file_path);
  if (!file.is_open())
  {
    using namespace std::string_literals;
    return lox::error("Failed to open file."s, 0ul);
  }
  using source_iter = std::istreambuf_iterator<char>;
  std::string const source{source_iter(file), source_iter{}};
  lox::Interpreter interpreter;
  return run(source, &interpreter, display);
}

auto run_prompt(DisplaySettings const& display) -> lox::result<void>
{
  std::string line;
  // Outside the loop for persistent variables
  lox::Interpreter interpreter;
  // Exit loop with CTRL + C
  while (true)
  {
    fmt::print(">> ");
    if (std::getline(std::cin, line) && !line.empty())
    {
      run(line, &interpreter, display).map_error(lox::report);
    }
  }
  return lox::ok();
}

auto main(int argc, char* argv[]) -> int
{
  try
  {
    auto opts = structopt::app("lox").parse<Options>(argc, argv);
    DisplaySettings const display{opts.ast_dump.value_or(false),
                                  opts.token_dump.value_or(false),
                                  opts.immediate_result_dump.value_or(false)};
    if (opts.script)
    {
      fmt::print("Running lox file: {}\n", *opts.script);
      // Report the error and the end the process
      run_file(*opts.script, display).map_error(lox::report).map_error([](auto&&) { std::exit(65); });
    }
    else
    {
      fmt::print("Running lox repl\n");
      run_prompt(display);
    }
  }
  catch (structopt::exception& e)
  {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }

  return EXIT_SUCCESS;
}
