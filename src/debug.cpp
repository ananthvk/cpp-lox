#include "debug.hpp"

auto simple_instruction(OpCode op, int offset) -> int
{
    fmt::println("{}", opcode_to_string(op));
    return offset + 1;
}

auto constant_instruction(OpCode op, int offset, const Chunk &chunk) -> int
{
    uint8_t constant_index = chunk.code[offset + 1];
    fmt::print("{:<16} {:4d} '", opcode_to_string(op), constant_index);
    auto value = chunk.value_array[constant_index];
    switch (value.type)
    {
    case Value::NUMBER_DOUBLE:
        fmt::print("{}'", value.data.d);
        break;
    case Value::NUMBER_INT:
        fmt::print("{}'", value.data.i);
        break;
    case Value::BOOLEAN:
        if (value.data.b)
            fmt::print("true'");
        else
            fmt::print("false'");
        break;
    }
    fmt::println("");
    return offset + 2;
}

auto constant_instruction_long(OpCode op, int offset, const Chunk &chunk) -> int
{
    uint32_t constant_index = chunk.code[offset + 1];
    constant_index |= static_cast<uint32_t>(static_cast<uint32_t>(chunk.code[offset + 2]) << 8);
    constant_index |= static_cast<uint32_t>(static_cast<uint32_t>(chunk.code[offset + 3]) << 16);

    fmt::print("{:<16} {:8d} '", opcode_to_string(op), constant_index);
    fmt::println("'");
    return offset + 4;
}

auto disassemble_chunk(const Chunk &chunk, const std::string &name) -> void
{
    fmt::print(fmt::fg(fmt::color::white) | fmt::emphasis::bold, "== {} ==\n", name);
    for (size_t offset = 0; offset < chunk.code.size();)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

auto disassemble_instruction(const Chunk &chunk, int offset) -> int
{
    fmt::print(fmt::fg(fmt::color::gray), "{:04} ", offset);

    if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1])
    {
        fmt::print("    | ");
    }
    else
    {
        fmt::print("{:5} ", chunk.lines[offset]);
    }

    auto instruction = static_cast<OpCode>(chunk.code[offset]);
    switch (instruction)
    {
    case OpCode::RETURN:
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
