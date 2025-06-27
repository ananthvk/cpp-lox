#include "debug.hpp"
#include "lexer.hpp"
#include "utils.hpp"

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

/**
 * An instruction that has an unsigned 2 byte operand
 */
auto instruction_uint16_le(OpCode op, int offset, const Chunk &chunk) -> int
{
    const auto &code = chunk.get_code();
    uint16_t constant_index = code[offset + 1];
    constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(code[offset + 2]) << 8);
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", opcode_to_string(op), constant_index);
    if (auto value = chunk.get_value(constant_index))
    {
        fmt::print(fmt::fg(fmt::color::green), "'{}'\n", value.value().to_string());
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "NO_VALUE\n");
    }
    return offset + 3;
}

auto disassemble_chunk(const Chunk &chunk, const std::string &name) -> void
{
    fmt::print(fmt::fg(fmt::color::white) | fmt::emphasis::bold, "== {} ({} bytes) ==\n", name,
               chunk.get_code().size());
    const auto &code = chunk.get_code();
    for (int offset = 0; offset < static_cast<int>(code.size());)
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
    case OpCode::STORE_GLOBAL:
    case OpCode::LOAD_GLOBAL:
    case OpCode::DEFINE_GLOBAL:
        return instruction_uint16_le(instruction, offset, chunk);
    case OpCode::TRUE:
    case OpCode::FALSE:
    case OpCode::NIL:
    case OpCode::NOT:
    case OpCode::GREATER:
    case OpCode::LESS:
    case OpCode::EQUAL:
    case OpCode::PRINT:
    case OpCode::POP_TOP:
        return simple_instruction(instruction, offset);
    default:
        fmt::print(fmt::fg(fmt::color::red), "{}", "UNKNOWN\n");
        return offset + 1;
    }
}

auto print_tokens(const Lexer &lexer) -> void
{
    int previous_line = 0;
    Lexer::const_token_iterator it;

    for (it = lexer.begin(); it != lexer.end(); ++it)
    {
        auto token = *it;
        if (token.line != previous_line)
        {
            fmt::print(fmt::fg(fmt::color::cyan), "{:>5} ", token.line);
        }
        else
        {
            fmt::print("    | ");
        }
        fmt::print(fmt::fg(fmt::color::green), "{:<14} ", token_type_to_string(token.token_type));
        fmt::print(fmt::fg(fmt::color::blue), "'{}'", escape_string(token.lexeme));
        if (token.err != ErrorCode::NO_ERROR)
        {
            fmt::print(fmt::fg(fmt::color::red), "{:>30}\n", error_code_to_string(token.err));
        }
        else
        {
            fmt::println("");
        }
        previous_line = token.line;
    }

    fmt::print(fmt::fg(fmt::color::purple), "{:>5} {:<14} ''\n", previous_line + 1,
               token_type_to_string((*it).token_type));
}