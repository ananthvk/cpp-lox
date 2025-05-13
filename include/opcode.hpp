#pragma once

#include <stddef.h>
#include <stdint.h>

#define OPCODES_LIST                                                                               \
    X(RETURN)                                                                                      \
    X(LOAD_CONSTANT)                                                                               \
    X(LOAD_CONSTANT_LONG)                                                                          \
    X(NEGATE)                                                                                      \
    X(ADD)                                                                                         \
    X(SUBTRACT)                                                                                    \
    X(MULTIPLY)                                                                                    \
    X(DIVIDE)

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
