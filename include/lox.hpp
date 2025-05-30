#pragma once
#include "result.hpp"
#include "error_reporter.hpp"
#include <filesystem>

class Lox
{
    auto execute(std::string_view src, ErrorReporter &reporter) -> InterpretResult;

  public:
    auto run_repl() -> int;
    auto run_file(const std::filesystem::path &path) -> int;
};