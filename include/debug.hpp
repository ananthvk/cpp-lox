#pragma once
#include "chunk.hpp"
#include "globals.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <string>


auto print_tokens(const Lexer &lexer) -> void;

auto disassemble_chunk(const Chunk &chunk, const std::string &name, Globals *globals) -> void;

auto disassemble_instruction(const Chunk &chunk, int offset, Globals *globals) -> int;