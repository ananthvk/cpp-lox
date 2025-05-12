#pragma once
#include <stddef.h>
#include <stdint.h>
#include <vector>

#define OPCODES_LIST                                                                               \
    X(RETURN)                                                                                      \
    X(LOAD_CONSTANT)

enum class OpCode
{
#define X(name) name,
    OPCODES_LIST
#undef X
};

inline auto opcode_to_string(OpCode op) -> const char *
{
    switch (op)
    {
#define X(name)                                                                                    \
    case OpCode::name:                                                                             \
        return #name;
        OPCODES_LIST
#undef X
    default:
        return "OP_UNKNOWN";
    }
}

struct Value
{
    enum ValueType
    {
        BOOLEAN,
        NUMBER_DOUBLE,
        NUMBER_INT
    };

    union Data
    {
        bool b;
        int64_t i;
        double d;
    } data;

    ValueType type;
};

struct Chunk
{
    std::vector<uint8_t> code;
    std::vector<Value> value_array;
    std::vector<int> lines;

    auto write_code(OpCode op, int line) -> void
    {
        lines.push_back(line);
        code.push_back(static_cast<uint8_t>(op));
    }

    auto write_code(uint8_t byte, int line) -> void
    {
        lines.push_back(line);
        code.push_back(byte);
    }

    auto add_constant(Value v) -> int
    {
        value_array.push_back(v);
        return value_array.size() - 1;
    }
};
