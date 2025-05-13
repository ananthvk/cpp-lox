#pragma once
#include <assert.h>
#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#define OPCODES_LIST                                                                               \
    X(RETURN)                                                                                      \
    X(LOAD_CONSTANT)                                                                               \
    X(LOAD_CONSTANT_LONG)

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
    enum ValueType : uint8_t
    {
        BOOLEAN,
        NUMBER_DOUBLE,
        NUMBER_INT
    };

    ValueType type;

    union Data
    {
        bool b;
        int64_t i;
        double d;
    } data;
};

struct Chunk
{
    std::vector<uint8_t> code;
    std::vector<Value> value_array;
    std::vector<int> lines;

    auto write_simple_op(OpCode op, int line) -> void
    {
        lines.push_back(line);
        code.push_back(static_cast<uint8_t>(op));
    }

    auto write_byte(uint8_t byte, int line) -> void
    {
        lines.push_back(line);
        code.push_back(byte);
    }

    auto write_load_constant(int index, int line) -> void
    {
        if (index < 0)
        {
            throw std::logic_error("Invalid index for write_load_constant. Underflow: < 0");
        }
        if (index <= 0xFF)
        {
            write_simple_op(OpCode::LOAD_CONSTANT, line);
            write_byte(static_cast<uint8_t>(index), line);
        }
        else if (index <= 0xFFFFFF)
        {
            write_simple_op(OpCode::LOAD_CONSTANT_LONG, line);
            /*
             * All values are stored in little endian format
             */
            write_byte(static_cast<uint8_t>(index & 0xFF), line);
            write_byte(static_cast<uint8_t>((index >> 8) & 0xFF), line);
            write_byte(static_cast<uint8_t>((index >> 16) & 0xFF), line);
        }
        else
        {
            throw std::logic_error("Invalid index for write_load_constant. Overflow: > 0xFFFFFF");
        }
    }

    template <typename T> std::enable_if_t<std::is_same_v<T, Value>, int> add_constant(T value)
    {
        value_array.push_back(value);
        return value_array.size() - 1;
    }

    template <typename T> std::enable_if_t<std::is_same_v<T, bool>, int> add_constant(T value)
    {
        Value v;
        v.data.b = value;
        v.type = Value::BOOLEAN;
        return add_constant(v);
    }

    template <typename T>
    std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> add_constant(T value)
    {
        Value v;
        v.data.i = value;
        v.type = Value::NUMBER_INT;
        return add_constant(v);
    }

    template <typename T> std::enable_if_t<std::is_same_v<T, double>, int> add_constant(T value)
    {
        Value v;
        v.data.d = value;
        v.type = Value::NUMBER_DOUBLE;
        return add_constant(v);
    }
};
