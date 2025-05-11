#pragma once
#include "chunk.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <string>

auto disassemble_chunk(const Chunk &chunk, const std::string &name) -> void;

auto disassemble_instruction(const Chunk &chunk, size_t offset) -> size_t;