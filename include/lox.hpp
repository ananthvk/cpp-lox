#pragma once
#include <filesystem>

class Lox
{
  public:
    auto run_repl() -> int;
    auto run_file(const std::filesystem::path &path) -> int;
};