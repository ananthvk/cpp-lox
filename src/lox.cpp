#include "lox.hpp"
#include "allocator.hpp"
#include "compiler.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "vm.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>

auto Lox::execute(std::string_view src, ErrorReporter &reporter, VM &vm, Allocator &allocator,
                  Globals *globals) -> InterpretResult
{
    CompilerOpts copts;
    copts.debug_print_tokens = false;

    Compiler compiler(src, copts, allocator, reporter, globals);
    auto result = compiler.compile();
    if (result != InterpretResult::OK)
        return result;

    auto chunk = compiler.take_chunk();

    // disassemble_chunk(chunk, "program", globals);

    result = vm.run(&chunk, std::cout);

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
    VMOpts vopts;
    Allocator allocator;
    Globals globals;
    vopts.debug_trace_execution = false;
    vopts.debug_trace_value_stack = false;
    vopts.debug_step_mode_enabled = false;
    VM vm(vopts, reporter, allocator, &globals);

    execute(source, reporter, vm, allocator, &globals);
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
    VMOpts vopts;
    Allocator allocator;
    Globals globals;
    vopts.debug_trace_execution = false;
    vopts.debug_trace_value_stack = false;
    vopts.debug_step_mode_enabled = false;
    VM vm(vopts, reporter, allocator, &globals);

    while (true)
    {
        fmt::print(fmt::fg(fmt::color::blue), ">>> ");
        if (!std::getline(std::cin, line))
            return 0;
        if (line == "exit")
            break;
        if (line == "")
            continue;
        execute(line, reporter, vm, allocator, &globals);
        if (reporter.has_messages())
        {
            reporter.display(stderr);
            reporter.clear();
        }
    }
    return 0;
}

auto Lox::run_source(std::string_view src) -> int
{
    CompilerOpts copts;
    VMOpts vopts;
    Allocator allocator;
    ErrorReporter reporter;
    Globals globals;

    copts.debug_print_tokens = false;

    vopts.debug_trace_execution = false;
    vopts.debug_trace_value_stack = false;
    vopts.debug_step_mode_enabled = false;

    Compiler compiler(src, copts, allocator, reporter, &globals);
    auto result = compiler.compile();
    if (result != InterpretResult::OK)
    {
        std::cout << "ERROR" << std::endl;
        return 1;
    }

    auto chunk = compiler.take_chunk();

    VM vm(vopts, reporter, allocator, &globals);
    result = vm.run(&chunk, std::cout);

    if (reporter.has_error() || result != InterpretResult::OK)
    {
        std::cout << "ERROR" << std::endl;
        return 2;
    }
    return 0;
}