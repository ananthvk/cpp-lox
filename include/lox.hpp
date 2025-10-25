#pragma once
#include "allocator.hpp"
#include "compiler.hpp"
#include "error_reporter.hpp"
#include "result.hpp"
#include "vm.hpp"
#include <filesystem>

struct LoxOpts
{
    bool dump_bytecode = false;
    bool compile_only = false;
};

class Lox
{
    auto compile_and_execute(std::string_view src, ErrorReporter &reporter, VM &vm,
                             Allocator &allocator, Context *context) -> InterpretResult;

    CompilerOpts compiler_opts;
    VMOpts vm_opts;
    LoxOpts lox_opts;
    
    bool native_functions_registered = false;

  public:
    auto run_repl() -> int;
    auto run_file(const std::filesystem::path &path) -> int;

    /**
     * This method is used for running tests
     */
    auto run_source(std::string_view src) -> int;

    Lox(const CompilerOpts &compiler_opts, const VMOpts &vm_opts, const LoxOpts &lox_opts);
};