#pragma once
#include "chunk.hpp"
#include "error_reporter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "result.hpp"
#include <string_view>

struct CompilerOpts
{
    bool debug_print_tokens = false;
};

class Compiler
{
    CompilerOpts opts;
    ErrorReporter &reporter;

  public:
    Compiler(const CompilerOpts &opts, ErrorReporter &reporter);
    auto compile(std::string_view source) const -> std::pair<Chunk, InterpretResult>;
};