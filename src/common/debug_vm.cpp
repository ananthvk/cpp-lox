#include "debug_vm.hpp"
#include "function.hpp"
#include "lexer.hpp"
#include "utils.hpp"
#include <fmt/color.h>
#include <fmt/format.h>

// Reads a byte value at the current location, and returns the byte
auto read_u8_le(const Chunk &chunk, int offset) -> uint8_t { return chunk.get_code()[offset]; }

// Reads an unsigned 16 bit value at the current location, and returns it
auto read_u16_le(const Chunk &chunk, int offset) -> uint16_t
{
    uint16_t value = chunk.get_code()[offset];
    value |= static_cast<uint16_t>(static_cast<uint16_t>(chunk.get_code()[offset + 1]) << 8);
    return value;
}

// Reads an index at the current location, and returns the value from the constant pool for that
// index
auto read_constant(const Chunk &chunk, int offset) -> std::optional<Value>
{
    auto index = read_u8_le(chunk, offset);
    return chunk.get_value(index);
}

// Reads an index at the current location, and returns the value from the constant pool for that
// index. Similar to read_constant, but uses a 2 byte unsigned operand instead
auto read_constant_long(const Chunk &chunk, int offset) -> std::optional<Value>
{
    auto index = read_u16_le(chunk, offset);
    return chunk.get_value(index);
}

// Reads a one byte opcode at offset
auto read_instruction(const Chunk &chunk, int offset) -> OpCode
{
    auto instruction = static_cast<OpCode>(chunk.get_code()[offset]);
    return instruction;
}

struct NameInfo
{
    bool is_global = true;
    std::string_view name;
};

struct DecodedInstruction
{
    // Offset to the start of the instruction (the opcode)
    int offset = 0;
    // The OpCode of this instruction
    OpCode opcode = OpCode::UNINITIALIZED;
    // Operands are represented as signed 64 bit integers, so actual size is lost
    std::vector<int64_t> operands;
    // Any constant values used by this instruction
    std::vector<std::optional<Value>> constants;
    // The line on which this instruction appears on
    int line = 0;
    // Offset to the next instruction, is equal to offset + 1 (for the opcode) + sizeof operands
    int next_offset = 0;
    // Names associated with each operand/constant index
    std::vector<NameInfo> names;
};

auto op_no_args(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    instr.next_offset = offset;
    return instr;
}

auto op_u8(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    instr.operands.push_back(read_u8_le(chunk, offset));
    offset += 1;
    instr.next_offset = offset;
    return instr;
}

auto op_u16(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    instr.operands.push_back(read_u16_le(chunk, offset));
    offset += 2;
    instr.next_offset = offset;
    return instr;
}

auto op_jump_u16(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    instr.operands.push_back(read_u16_le(chunk, offset));
    offset += 2;
    instr.next_offset = offset;
    return instr;
}

auto op_constant_u8(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    instr.operands.push_back(read_u8_le(chunk, offset));
    instr.constants.push_back(read_constant(chunk, offset));
    offset += 1;
    instr.next_offset = offset;
    return instr;
}

auto op_constant_u16(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    instr.operands.push_back(read_u16_le(chunk, offset));
    instr.constants.push_back(read_constant_long(chunk, offset));
    offset += 2;
    instr.next_offset = offset;
    return instr;
}

auto op_global_u16(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    auto index = read_u16_le(chunk, offset);
    instr.operands.push_back(index);
    offset += 2;
    if (context->exists(index))
        instr.names.push_back({true, context->get_name(index)->get()});
    else
        instr.names.push_back({true, ""});
    instr.next_offset = offset;
    return instr;
}

auto op_invoke(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    // Read the constant index for the method name
    instr.operands.push_back(read_u16_le(chunk, offset));
    instr.constants.push_back(read_constant_long(chunk, offset));
    offset += 2;
    // Next read the argument count
    instr.operands.push_back(read_u8_le(chunk, offset++));
    instr.next_offset = offset;
    return instr;
}

auto op_closure(Context *context, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction instr;
    instr.offset = offset;
    instr.line = chunk.get_line_number(offset);
    instr.opcode = read_instruction(chunk, offset++);
    // Read the constant index for the function name
    instr.operands.push_back(read_u16_le(chunk, offset));
    instr.constants.push_back(read_constant_long(chunk, offset));
    offset += 2;
    // Next read 3 bytes for each upvalue (is_local, index)

    if (instr.constants.back().has_value())
    {
        auto val = instr.constants.back().value();
        auto function = static_cast<ObjectFunction *>(val.as_object());
        for (int i = 0; i < function->upvalue_count(); i++)
        {
            auto is_local = read_u8_le(chunk, offset++);
            auto index = read_u16_le(chunk, offset);
            offset += 2;
            instr.operands.push_back(is_local);
            instr.operands.push_back(index);
        }
    }
    instr.next_offset = offset;
    return instr;
}

auto op_unknown(Context *, const Chunk &chunk, int offset) -> DecodedInstruction
{
    DecodedInstruction d{};
    d.offset = offset;
    d.line = chunk.get_line_number(offset);
    d.opcode = OpCode::INVALID;
    d.next_offset = offset + 1;
    return d;
}

using DecodeFn = DecodedInstruction (*)(Context *, const Chunk &, int);

static DecodeFn decode_table[256];

auto init_decode_table() -> void
{
    for (int i = 0; i < 256; i++)
        decode_table[i] = op_unknown;

    // OpCodes that don't have any operands
    decode_table[static_cast<int>(OpCode::RETURN)] = op_no_args;
    decode_table[static_cast<int>(OpCode::NEGATE)] = op_no_args;
    decode_table[static_cast<int>(OpCode::ADD)] = op_no_args;
    decode_table[static_cast<int>(OpCode::SUBTRACT)] = op_no_args;
    decode_table[static_cast<int>(OpCode::MULTIPLY)] = op_no_args;
    decode_table[static_cast<int>(OpCode::DIVIDE)] = op_no_args;
    decode_table[static_cast<int>(OpCode::TRUE)] = op_no_args;
    decode_table[static_cast<int>(OpCode::FALSE)] = op_no_args;
    decode_table[static_cast<int>(OpCode::NIL)] = op_no_args;
    decode_table[static_cast<int>(OpCode::UNINITIALIZED)] = op_no_args;
    decode_table[static_cast<int>(OpCode::NOT)] = op_no_args;
    decode_table[static_cast<int>(OpCode::GREATER)] = op_no_args;
    decode_table[static_cast<int>(OpCode::LESS)] = op_no_args;
    decode_table[static_cast<int>(OpCode::EQUAL)] = op_no_args;
    decode_table[static_cast<int>(OpCode::PRINT)] = op_no_args;
    decode_table[static_cast<int>(OpCode::POP_TOP)] = op_no_args;
    decode_table[static_cast<int>(OpCode::DUP_TOP)] = op_no_args;
    decode_table[static_cast<int>(OpCode::CLOSE_UPVALUE)] = op_no_args;
    decode_table[static_cast<int>(OpCode::INHERIT)] = op_no_args;
    decode_table[static_cast<int>(OpCode::ZERO)] = op_no_args;
    decode_table[static_cast<int>(OpCode::MINUS_ONE)] = op_no_args;
    decode_table[static_cast<int>(OpCode::ONE)] = op_no_args;
    decode_table[static_cast<int>(OpCode::LIST_APPEND)] = op_no_args;
    decode_table[static_cast<int>(OpCode::MAP_ADD)] = op_no_args;
    decode_table[static_cast<int>(OpCode::STORE_INDEX)] = op_no_args;
    decode_table[static_cast<int>(OpCode::LOAD_INDEX)] = op_no_args;

    // OpCodes that have a single uint8_t operand
    decode_table[static_cast<int>(OpCode::CALL)] = op_u8;
    decode_table[static_cast<int>(OpCode::LIST)] = op_u8;
    decode_table[static_cast<int>(OpCode::MAP)] = op_u8;

    // OpCodes that have a single uint8_t operand (that is also a constant index)
    decode_table[static_cast<int>(OpCode::LOAD_CONSTANT)] = op_constant_u8;

    // OpCodes that have a single uint16_t operand (that is also a constant index)
    decode_table[static_cast<int>(OpCode::LOAD_CONSTANT_LONG)] = op_constant_u16;
    decode_table[static_cast<int>(OpCode::CLASS)] = op_constant_u16;
    decode_table[static_cast<int>(OpCode::METHOD)] = op_constant_u16;
    decode_table[static_cast<int>(OpCode::STORE_PROPERTY)] = op_constant_u16;
    decode_table[static_cast<int>(OpCode::LOAD_PROPERTY)] = op_constant_u16;
    decode_table[static_cast<int>(OpCode::LOAD_PROPERTY_SAFE)] = op_constant_u16;
    decode_table[static_cast<int>(OpCode::LOAD_SUPER)] = op_constant_u16;

    // OpCodes that have a single uint16_t operand
    decode_table[static_cast<int>(OpCode::LOAD_UPVALUE)] = op_u16;
    decode_table[static_cast<int>(OpCode::STORE_UPVALUE)] = op_u16;
    decode_table[static_cast<int>(OpCode::LOAD_LOCAL)] = op_u16;
    decode_table[static_cast<int>(OpCode::STORE_LOCAL)] = op_u16;
    decode_table[static_cast<int>(OpCode::JUMP_IF_FALSE)] = op_u16;
    decode_table[static_cast<int>(OpCode::POP_JUMP_IF_FALSE)] = op_u16;
    decode_table[static_cast<int>(OpCode::JUMP_FORWARD)] = op_u16;
    decode_table[static_cast<int>(OpCode::JUMP_IF_TRUE)] = op_u16;
    decode_table[static_cast<int>(OpCode::JUMP_BACKWARD)] = op_u16;


    // OpCodes that have specific semantics
    decode_table[static_cast<int>(OpCode::DEFINE_GLOBAL)] = op_global_u16;
    decode_table[static_cast<int>(OpCode::STORE_GLOBAL)] = op_global_u16;
    decode_table[static_cast<int>(OpCode::LOAD_GLOBAL)] = op_global_u16;
    decode_table[static_cast<int>(OpCode::INVOKE)] = op_invoke;
    decode_table[static_cast<int>(OpCode::SUPER_INVOKE)] = op_invoke;
    decode_table[static_cast<int>(OpCode::CLOSURE)] = op_closure;
}

auto is_jump(OpCode opcode) -> bool
{
    switch (opcode)
    {
    case OpCode::JUMP_IF_FALSE:
    case OpCode::POP_JUMP_IF_FALSE:
    case OpCode::JUMP_FORWARD:
    case OpCode::JUMP_IF_TRUE:
    case OpCode::JUMP_BACKWARD:
        return true;
    default:
        return false;
    }
}

// Print line number (or space if same as previous)
auto print_line_number(int line, int previous_line, int offset) -> void
{
    if (offset > 0 && line == previous_line)
        fmt::print("      ");
    else
        fmt::print(fmt::fg(fmt::color::yellow), "{:5} ", line);
}

// Print offset in hex format
auto print_offset(int offset) -> void { fmt::print(fmt::fg(fmt::color::cyan), "{:05x}: ", offset); }

// Print instruction bytes (opcode + operands)
auto print_instruction_bytes(const std::vector<uint8_t> &code, const DecodedInstruction &instr)
    -> void
{
    int byte_count = instr.next_offset - instr.offset;
    const int max_bytes_shown = 12;

    if (byte_count <= max_bytes_shown)
    {
        for (int i = instr.offset; i < instr.next_offset; i++)
        {
            fmt::print(fmt::fg(fmt::color::green), "{:02x} ", code[i]);
        }
        // Pad to align opcodes
        for (int i = byte_count; i < max_bytes_shown; i++)
        {
            fmt::print("   ");
        }
    }
    else
    {
        // Show first few bytes then ...
        for (int i = instr.offset; i < instr.offset + max_bytes_shown - 1; i++)
        {
            fmt::print(fmt::fg(fmt::color::green), "{:02x} ", code[i]);
        }
        fmt::print(fmt::fg(fmt::color::green), "...");
    }
}

// Print closure operands with special formatting
auto print_closure_operands(const DecodedInstruction &instr) -> void
{
    if (instr.operands.empty())
        return;

    // Print first operand (function index)
    fmt::print(" ");
    fmt::print(fmt::fg(fmt::color::magenta), "{}", instr.operands[0]);

    // Print upvalue info in pairs (is_local, index)
    if (instr.operands.size() > 1)
    {
        fmt::print(" (");
        for (size_t i = 1; i < instr.operands.size(); i += 2)
        {
            if (i > 1)
                fmt::print(", ");

            if (i + 1 < instr.operands.size())
            {
                int is_local = static_cast<int>(instr.operands[i]);
                int index = static_cast<int>(instr.operands[i + 1]);
                if (is_local == 0)
                    fmt::print(fmt::fg(fmt::color::magenta), "u{}", index);
                else
                    fmt::print(fmt::fg(fmt::color::magenta), "l{}", index);
            }
        }
        fmt::print(")");
    }
}

// Print regular operands (comma separated)
auto print_operands(const DecodedInstruction &instr) -> void
{
    if (instr.operands.empty())
        return;

    fmt::print(" ");
    for (size_t i = 0; i < instr.operands.size(); i++)
    {
        if (i > 0)
            fmt::print(", ");
        fmt::print(fmt::fg(fmt::color::magenta), "{}", instr.operands[i]);
    }
}

// Print constants in brackets
auto print_constants(const DecodedInstruction &instr) -> void
{
    if (instr.constants.empty())
        return;

    fmt::print(fmt::fg(fmt::color::blue), " [");
    for (size_t i = 0; i < instr.constants.size(); i++)
    {
        if (i > 0)
            fmt::print(fmt::fg(fmt::color::blue), ", ");
        if (instr.constants[i].has_value())
        {
            fmt::print(fmt::fg(fmt::color::light_blue), "{}",
                       instr.constants[i].value().to_string());
        }
        else
        {
            fmt::print(fmt::fg(fmt::color::red), "NO_VALUE");
        }
    }
    fmt::print(fmt::fg(fmt::color::blue), "]");
}

// Print names if available
auto print_names(const DecodedInstruction &instr) -> void
{
    if (instr.names.empty())
        return;

    fmt::print(fmt::fg(fmt::color::orange), " # ");
    for (size_t i = 0; i < instr.names.size(); i++)
    {
        if (i > 0)
            fmt::print(fmt::fg(fmt::color::orange), ", ");
        if (!instr.names[i].name.empty())
        {
            fmt::print(fmt::fg(fmt::color::orange), "{}", instr.names[i].name);
        }
    }
}

// Print jump target if it's a jump instruction
auto print_jump_target(const Chunk &chunk, const DecodedInstruction &instr) -> void
{
    if (!is_jump(instr.opcode) || instr.operands.empty())
        return;

    int direction = (instr.opcode == OpCode::JUMP_BACKWARD) ? -1 : 1;
    int jmp = static_cast<int>(instr.operands[0]);
    int target_offset = instr.next_offset + direction * jmp;
    int target_line = chunk.get_line_number(target_offset);
    fmt::print(fmt::fg(fmt::color::yellow), " -> {:04x}:L{}", target_offset, target_line);
}

auto decode_chunk(const Chunk &chunk, Context *context)
{
    auto &code = chunk.get_code();
    DecodedInstruction previous{};
    previous.line = 0;

    for (int offset = 0; offset < static_cast<int>(code.size());)
    {
        auto instr = decode_table[code[offset]](context, chunk, offset);

        print_line_number(instr.line, previous.line, offset);
        print_offset(offset);
        print_instruction_bytes(code, instr);

        // Print opcode
        fmt::print(fmt::fg(fmt::color::white), "{}", opcode_to_string(instr.opcode));

        // Handle operands based on instruction type
        if (instr.opcode == OpCode::CLOSURE)
        {
            print_closure_operands(instr);
        }
        else
        {
            print_operands(instr);
        }

        print_constants(instr);
        print_names(instr);
        print_jump_target(chunk, instr);

        fmt::println("");
        offset = instr.next_offset;
        previous = instr;
    }
}

auto disassemble_chunk(const Chunk &chunk, const std::string &name, Context *context,
                       bool print_header) -> void
{
    if (print_header)
    {
        fmt::print(fmt::fg(fmt::color::white) | fmt::emphasis::bold, "== {} ({} bytes) ==\n", name,
                   chunk.get_code().size());
    }
    decode_chunk(chunk, context);
}