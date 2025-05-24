#pragma once
#include "chunk.hpp"
#include "token.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <string>


auto print_tokens(std::string_view source) -> void;

auto disassemble_chunk(const Chunk &chunk, const std::string &name) -> void;

auto disassemble_instruction(const Chunk &chunk, int offset) -> int;