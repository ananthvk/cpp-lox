#include "lox.hpp"
#include "compiler.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "vm.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>

auto Lox::execute(std::string_view src, ErrorReporter &reporter) -> InterpretResult
{
    CompilerOpts copts;
    VMOpts vopts;

    copts.debug_print_tokens = false;

    vopts.debug_trace_execution = false;
    vopts.debug_trace_value_stack = false;
    vopts.debug_step_mode_enabled = false;

    Compiler compiler(src, copts, reporter);
    auto result = compiler.compile();
    if (result != InterpretResult::OK)
        return result;

    auto chunk = compiler.take_chunk();

    disassemble_chunk(chunk, "program");

    VM vm(vopts, reporter);
    result = vm.run(&chunk);
    return result;
}

auto Lox::run_file(const std::filesystem::path &path) -> int
{
    std::ifstream file(path);
    if (!file)
    {
        fmt::print(fmt::fg(fmt::color::red), "Unable to read \"{}\" Error: {}\n", path.string(),
                   std::strerror(errno));
        return 1;
    }
    if (!std::filesystem::is_regular_file(path))
    {
        fmt::print(fmt::fg(fmt::color::red), "Unable to read \"{}\" Error: Not a file\n",
                   path.string());
        return 1;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    auto source = ss.str();
    ErrorReporter reporter;
    execute(source, reporter);
    if (reporter.has_messages())
    {
        reporter.display(stderr);
    }
    return 0;
}

auto Lox::run_repl() -> int
{
    std::string line;
    ErrorReporter reporter;
    while (true)
    {
        fmt::print(fmt::fg(fmt::color::blue), ">>> ");
        if (!std::getline(std::cin, line))
            return 0;
        if (line == "exit")
            break;
        if (line == "")
            continue;
        execute(line, reporter);
        if (reporter.has_messages())
        {
            reporter.display(stderr);
            reporter.clear();
        }
    }
    return 0;
}