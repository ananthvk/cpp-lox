#pragma once
#include "allocator.hpp"
#include "error_reporter.hpp"
#include "result.hpp"
#include "vm.hpp"
#include <filesystem>

class Lox
{
    auto execute(std::string_view src, ErrorReporter &reporter, VM &vm, Allocator &allocator,
                 Globals *globals) -> InterpretResult;

  public:
    auto run_repl() -> int;
    auto run_file(const std::filesystem::path &path) -> int;

    /**
     * This method is used for running tests
     */
    auto run_source(std::string_view src) -> int;
};