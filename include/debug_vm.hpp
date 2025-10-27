#pragma once
#include "chunk.hpp"
#include "context.hpp"
#include <string>

auto disassemble_chunk(const Chunk &chunk, const std::string &name, Context *context,
                       bool print_header = true) -> void;

auto disassemble_instruction(const Chunk &chunk, int offset, Context *context) -> int;