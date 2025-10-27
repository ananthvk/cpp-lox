#pragma once
#include "chunk.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <string>


auto print_tokens(const Lexer &lexer) -> void;

auto disassemble_chunk(const Chunk &chunk, const std::string &name, Context *context, bool print_header = true) -> void;

auto disassemble_instruction(const Chunk &chunk, int offset, Context *context) -> int;