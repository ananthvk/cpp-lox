#include "lox.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>

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
    print_tokens(source);
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
        print_tokens(line);
    }
    return 0;
}