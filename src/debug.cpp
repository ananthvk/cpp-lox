#include "debug.hpp"

auto disassemble_chunk(const Chunk &chunk, const std::string &name) -> void
{
    fmt::print(fmt::fg(fmt::color::white) | fmt::emphasis::bold, "== {} ==\n", name);
    for (size_t offset = 0; offset < chunk.bytes.size();)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

auto disassemble_instruction(const Chunk &chunk, size_t offset) -> size_t
{
    fmt::print(fmt::fg(fmt::color::gray), "{:04} ", offset);

    auto instruction = static_cast<OpCode>(chunk.bytes[offset]);
    switch (instruction)
    {
    case OpCode::RETURN:
        fmt::println("{}", opcode_to_string(instruction));
        return offset + 1;
    default:
        fmt::print(fmt::fg(fmt::color::red), "{:04}", offset);
        return offset + 1;
    }
}