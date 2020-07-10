#include <fmt/format.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gsl/span>
#include <iostream>
#include <magic_enum/magic_enum.hpp>

#include "lox/ast/expression.hpp"
#include "lox/ast/parse.hpp"
#include "lox/ast/printer.hpp"
#include "lox/lex.hpp"

auto run(std::string_view source) -> lox::result<void>
{
	return lox::lex(source)
	    .map([](auto&& tokens) {
		    tokens.erase(
		        std::remove_if(tokens.begin(),
		                       tokens.end(),
		                       [](auto const& token) { return token.type == lox::TOKEN_TYPE::COMMENT; }),
		        tokens.end());
		    return std::move(tokens);
	    })
	    .and_then([](auto&& tokens) { return lox::parse(tokens); })
	    .map([](auto const& expr) {
		    lox::AstPrinter printer;
				std::get<0>(expr)->accept(printer);
		    fmt::print("{}\n", printer.m_ast);
	    });
}

auto run_file(std::filesystem::path file_path) -> lox::result<void>
{
	std::ifstream file(file_path);
	if (!file.is_open())
	{
		using namespace std::string_literals;
		return lox::error("Failed to open file."s, 0ul);
	}
	using source_iter = std::istreambuf_iterator<char>;
	std::string const source{source_iter(file), source_iter{}};
	return run(source);
}

auto run_prompt() -> lox::result<void>
{
	std::string line;
	// Exit loop with CTRL + C
	while (true)
	{
		fmt::print(">> ");
		if (std::getline(std::cin, line) && !line.empty())
		{
			run(line).map_error(lox::report);
		}
	}
	return lox::ok();
}

auto main(int argc, char* argv[]) -> int
{
#if 0
  std::unique_ptr<lox::Expression> expression = std::make_unique<lox::Binary>(
    std::make_unique<lox::Unary>(
      std::make_unique<lox::Literal>(lox::Token::literal{123.f}),
      lox::TOKEN_TYPE::MINUS),
    std::make_unique<lox::Group>(
      std::make_unique<lox::Literal>(lox::Token::literal{45.67f})),
    lox::TOKEN_TYPE::STAR);

  lox::AstPrinter visitor;
  expression->accept(visitor);
  fmt::print("{}\n", visitor.m_ast);
#else

	if (argc > 2)
	{
		fmt::print("Usage: lox [script]\n");
		return 64;
	}
	else if (argc == 2)
	{
		fmt::print("Running lox file: {}\n", argv[1]);
		// Report the error and the end the process
		run_file(argv[1]).map_error(lox::report).map_error([](auto&&) { std::exit(65); });
	}
	else
	{
		fmt::print("Running lox repl\n");
		run_prompt();
	}

#endif
	return EXIT_SUCCESS;
}
