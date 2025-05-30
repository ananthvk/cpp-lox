#pragma once
#include "result.hpp"
#include <filesystem>

class Lox
{
    auto execute(std::string_view src) -> InterpretResult;

  public:
    auto run_repl() -> int;
    auto run_file(const std::filesystem::path &path) -> int;
};