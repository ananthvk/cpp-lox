#include "debug.hpp"
#include "function.hpp"
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
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", opcode_to_string(op), constant_index);
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

// If direction = 1, it means it's a foward offset, and if it's -1, it's a backward offset
auto jump_instruction(OpCode op, int offset, const Chunk &chunk, int direction) -> int
{
    const auto &code = chunk.get_code();
    uint16_t jmp = code[offset + 1];
    jmp |= static_cast<uint16_t>(static_cast<uint16_t>(code[offset + 2]) << 8);
    fmt::print(fmt::fg(fmt::color::blue), "{:<16} {:8d} ", opcode_to_string(op), jmp);
    int loc = (offset + 3 + direction * static_cast<int>(jmp));
    fmt::print(fmt::fg(fmt::color::gray), "{:04} ", loc);
    fmt::print(fmt::fg(fmt::color::green), "L{}\n", chunk.get_line_number(loc));
    return offset + 3;
}

/**
 * An instruction that has an unsigned 2 byte operand
 */
auto instruction_uint16_le(OpCode op, int offset, const Chunk &chunk, bool should_print_value)
    -> int
{
    const auto &code = chunk.get_code();
    uint16_t constant_index = code[offset + 1];
    constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(code[offset + 2]) << 8);
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", opcode_to_string(op), constant_index);
    if (should_print_value)
    {
        if (auto value = chunk.get_value(constant_index))
        {
            fmt::print(fmt::fg(fmt::color::green), "'{}'\n", value.value().to_string());
        }
        else
        {
            fmt::print(fmt::fg(fmt::color::red), "NO_VALUE\n");
        }
    }
    else
    {
        fmt::print("\n");
    }
    return offset + 3;
}

auto instruction_call(OpCode op, int offset, const Chunk &chunk) -> int
{
    const auto &code = chunk.get_code();
    uint8_t arguments = code[offset + 1];
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", opcode_to_string(op), +arguments);
    fmt::print(fmt::fg(fmt::color::green), "'{} arguments'\n", arguments);
    return offset + 2;
}

auto global_instruction(OpCode op, int offset, const Chunk &chunk, Context *context) -> int
{
    const auto &code = chunk.get_code();
    uint16_t constant_index = code[offset + 1];
    constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(code[offset + 2]) << 8);
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", opcode_to_string(op), constant_index);

    if (context->exists(constant_index))
    {
        fmt::print(fmt::fg(fmt::color::green), "'{}'\n", context->get_name(constant_index)->get());
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "NO_VALUE\n");
    }
    return offset + 3;
}

auto local_instruction(OpCode op, int offset, const Chunk &chunk, Context *context) -> int
{
    const auto &code = chunk.get_code();
    uint16_t slot = code[offset + 1];
    slot |= static_cast<uint16_t>(static_cast<uint16_t>(code[offset + 2]) << 8);
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} \n", opcode_to_string(op), slot);
    // fmt::print(fmt::fg(fmt::color::green), "'{}'\n", context->get_name(constant_index)->get());
    return offset + 3;
}

auto closure_instruction(OpCode op, int offset, const Chunk &chunk, Context *context) -> int
{
    const auto &code = chunk.get_code();
    uint16_t constant_index = code[offset + 1];
    constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(code[offset + 2]) << 8);
    fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", opcode_to_string(op), constant_index);
    offset = offset + 3;

    if (auto value = chunk.get_value(constant_index))
    {
        auto val = value.value();
        fmt::print(fmt::fg(fmt::color::green), "'{}'\n", val.to_string());

        // Also print the list of upvalues & locals this function captures
        auto function = static_cast<ObjectFunction *>(val.as_object());
        for (int i = 0; i < function->upvalue_count(); i++)
        {
            fmt::print(fmt::fg(fmt::color::gray), "{:04} ", offset);
            int is_local = code[offset];
            uint16_t index = code[offset + 1];
            index |= static_cast<uint16_t>(static_cast<uint16_t>(code[offset + 2]) << 8);
            offset = offset + 3;

            fmt::print("    | ");
            fmt::print(fmt::fg(fmt::color::purple), "{:<16} {:8d} ", "", index);
            if (is_local == 0)
                fmt::print(fmt::fg(fmt::color::pink), "upvalue\n");
            else
                fmt::print(fmt::fg(fmt::color::pink), "local\n");
        }
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "NO_VALUE\n");
    }
    return offset;
}

auto disassemble_chunk(const Chunk &chunk, const std::string &name, Context *context) -> void
{
    fmt::print(fmt::fg(fmt::color::white) | fmt::emphasis::bold, "== {} ({} bytes) ==\n", name,
               chunk.get_code().size());
    const auto &code = chunk.get_code();
    for (int offset = 0; offset < static_cast<int>(code.size());)
    {
        offset = disassemble_instruction(chunk, offset, context);
    }
}

auto disassemble_instruction(const Chunk &chunk, int offset, Context *context) -> int
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
    case OpCode::TRUE:
    case OpCode::FALSE:
    case OpCode::NIL:
    case OpCode::UNINITIALIZED:
    case OpCode::NOT:
    case OpCode::GREATER:
    case OpCode::LESS:
    case OpCode::EQUAL:
    case OpCode::PRINT:
    case OpCode::POP_TOP:
    case OpCode::DUP_TOP:
    case OpCode::CLOSE_UPVALUE:
        return simple_instruction(instruction, offset);
    case OpCode::CALL:
        return instruction_call(instruction, offset, chunk);
    case OpCode::LOAD_CONSTANT:
        return constant_instruction(instruction, offset, chunk);
    case OpCode::LOAD_CONSTANT_LONG:
    case OpCode::CLASS:
    case OpCode::METHOD:
    case OpCode::STORE_PROPERTY:
    case OpCode::LOAD_PROPERTY:
    case OpCode::LOAD_PROPERTY_SAFE:
        return instruction_uint16_le(instruction, offset, chunk, true);
    case OpCode::LOAD_UPVALUE:
    case OpCode::STORE_UPVALUE:
        return instruction_uint16_le(instruction, offset, chunk, false);
    case OpCode::CLOSURE:
        return closure_instruction(instruction, offset, chunk, context);
    case OpCode::STORE_GLOBAL:
    case OpCode::LOAD_GLOBAL:
    case OpCode::DEFINE_GLOBAL:
        return global_instruction(instruction, offset, chunk, context);
    case OpCode::LOAD_LOCAL:
    case OpCode::STORE_LOCAL:
        return local_instruction(instruction, offset, chunk, context);
    case OpCode::JUMP_IF_FALSE:
    case OpCode::POP_JUMP_IF_FALSE:
    case OpCode::JUMP_FORWARD:
    case OpCode::JUMP_IF_TRUE:
        return jump_instruction(instruction, offset, chunk, 1);
    case OpCode::JUMP_BACKWARD:
        return jump_instruction(instruction, offset, chunk, -1);

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