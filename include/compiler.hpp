#pragma once
#include "chunk.hpp"
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

  public:
    Compiler(const CompilerOpts &opts);
    auto compile(std::string_view source) const -> std::pair<Chunk, InterpretResult>;
};