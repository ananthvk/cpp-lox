#include "debug.hpp"

auto simple_instruction(OpCode op, int offset) -> int
{
    fmt::print(fmt::fg(fmt::color::cyan), "{}\n", opcode_to_string(op));
    return offset + 1;
}

auto constant_instruction(OpCode op, int offset, const Chunk &chunk) -> int
{
    uint8_t constant_index = chunk.get_code()[offset + 1];
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:4d} ", opcode_to_string(op), constant_index);
    if (auto value = chunk.get_value(constant_index))
    {
        fmt::print(fmt::fg(fmt::color::green), "'{}'\n", value.value().to_string());
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "NO_VALUE\n");
    }
    return offset + 2;
}

auto constant_instruction_long(OpCode op, int offset, const Chunk &chunk) -> int
{
    const auto &code = chunk.get_code();
    uint32_t constant_index = code[offset + 1];
    constant_index |= static_cast<uint32_t>(static_cast<uint32_t>(code[offset + 2]) << 8);
    constant_index |= static_cast<uint32_t>(static_cast<uint32_t>(code[offset + 3]) << 16);
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", opcode_to_string(op), constant_index);
    if (auto value = chunk.get_value(constant_index))
    {
        fmt::print(fmt::fg(fmt::color::green), "'{}'\n", value.value().to_string());
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "NO_VALUE\n");
    }
    return offset + 4;
}

auto disassemble_chunk(const Chunk &chunk, const std::string &name) -> void
{
    fmt::print(fmt::fg(fmt::color::white) | fmt::emphasis::bold, "== {} ({} bytes) ==\n", name,
               chunk.get_code().size());
    const auto &code = chunk.get_code();
    for (size_t offset = 0; offset < code.size();)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

auto disassemble_instruction(const Chunk &chunk, int offset) -> int
{
    fmt::print(fmt::fg(fmt::color::gray), "{:04} ", offset);

    if (offset > 0 && chunk.get_line_number(offset) == chunk.get_line_number(offset - 1))
    {
        fmt::print("    | ");
    }
    else
    {
        fmt::print("{:5} ", chunk.get_line_number(offset));
    }

    auto instruction = static_cast<OpCode>(chunk.get_code()[offset]);
    switch (instruction)
    {
    case OpCode::RETURN:
    case OpCode::NEGATE:
    case OpCode::ADD:
    case OpCode::SUBTRACT:
    case OpCode::MULTIPLY:
    case OpCode::DIVIDE:
        return simple_instruction(instruction, offset);
    case OpCode::LOAD_CONSTANT:
        return constant_instruction(instruction, offset, chunk);
    case OpCode::LOAD_CONSTANT_LONG:
        return constant_instruction_long(instruction, offset, chunk);
    default:
        fmt::print(fmt::fg(fmt::color::red), "{}", "UNKNOWN\n");
        return offset + 1;
    }
}
