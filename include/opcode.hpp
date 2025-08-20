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
    X(DIVIDE)                                                                                      \
    X(NIL)                                                                                         \
    X(UNINITIALIZED)                                                                               \
    X(TRUE)                                                                                        \
    X(FALSE)                                                                                       \
    X(NOT)                                                                                         \
    X(EQUAL)                                                                                       \
    X(GREATER)                                                                                     \
    X(LESS)                                                                                        \
    X(PRINT)                                                                                       \
    X(POP_TOP)                                                                                     \
    X(DEFINE_GLOBAL)                                                                               \
    X(STORE_GLOBAL)                                                                                \
    X(LOAD_GLOBAL)                                                                                 \
    X(STORE_LOCAL)                                                                                 \
    X(LOAD_LOCAL)                                                                                  \
    X(JUMP_IF_FALSE)

enum class OpCode : uint8_t
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
