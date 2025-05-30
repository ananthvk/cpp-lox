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
    std::string_view source;
    CompilerOpts opts;
    ErrorReporter &reporter;

    Chunk chunk;
    Lexer lexer;
    Parser parser;

    auto emit_opcode(OpCode code) -> void { chunk.write_simple_op(code, parser.previous().line); }

    auto emit_opcode(OpCode code, uint8_t byte) -> void
    {
        emit_opcode(code);
        emit_byte(byte);
    }

    auto emit_byte(uint8_t byte) -> void { chunk.write_byte(byte, parser.previous().line); }

    auto emit_return() -> void { emit_opcode(OpCode::RETURN); }

    auto expression() -> void;

  public:
    Compiler(std::string_view source, const CompilerOpts &opts, ErrorReporter &reporter);

    auto compile() -> InterpretResult;

    auto take_chunk() -> Chunk &&;
};