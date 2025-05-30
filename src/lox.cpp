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

auto Lox::execute(std::string_view src) -> InterpretResult
{
    CompilerOpts copts;
    VMOpts vopts;

    copts.debug_print_tokens = true;

    vopts.debug_trace_execution = true;
    vopts.debug_trace_value_stack = true;
    vopts.debug_step_mode_enabled = true;

    Compiler compiler(copts);
    auto [chunk, result] = compiler.compile(src);
    if (result != InterpretResult::OK)
    {
        // Handle error here
        fmt::print(fmt::fg(fmt::color::red), "Compilation error occured\n");
        return result;
    }
    VM vm(vopts);
    result = vm.run(&chunk);
    if (result != InterpretResult::OK)
    {
        fmt::print(fmt::fg(fmt::color::red), "Runtime error occured\n");
    }

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
    execute(source);
    return 0;
}

auto Lox::run_repl() -> int
{
    std::string line;
    while (true)
    {
        fmt::print(fmt::fg(fmt::color::blue), ">>> ");
        if (!std::getline(std::cin, line))
            return 0;
        if (line == "exit")
            break;
        if (line == "")
            continue;
        execute(line);
    }
    return 0;
}