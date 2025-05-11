#pragma once
#include <stddef.h>
#include <stdint.h>
#include <vector>

#define OPCODES_LIST                                                                               \
    X(RETURN)                                                                                      \
    X(PRINT)

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

struct Chunk
{
    std::vector<uint8_t> bytes;

    auto write_code(OpCode code) -> void { bytes.push_back(static_cast<uint8_t>(code)); }
};
