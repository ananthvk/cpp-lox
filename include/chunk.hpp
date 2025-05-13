#pragma once
#include <assert.h>
#include <optional>
#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
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

    auto to_string() const -> std::string
    {
        switch (type)
        {
        case Value::NUMBER_DOUBLE:
            return std::to_string(data.d);
        case Value::NUMBER_INT:
            return std::to_string(data.i);

        case Value::BOOLEAN:
            if (data.b)
                return "true";
            else
                return "false";
            break;
        }
    }
};

class Chunk
{
  private:
    /*
     * Marks the start of a line. Line `line_number` starts at the byte with offset `offset`
     */
    struct LineInfo
    {
        int offset;
        int line_number;
    };

    std::vector<uint8_t> code;
    std::vector<Value> value_array;
    std::vector<LineInfo> lines;

    auto add_line_number(int offset, int line) -> void;

  public:
    auto write_simple_op(OpCode op, int line) -> void;

    auto write_byte(uint8_t byte, int line) -> void;

    auto write_load_constant(int index, int line) -> void;

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

    auto get_code() const -> const std::vector<uint8_t> & { return code; }

    auto get_code() -> std::vector<uint8_t> & { return code; }

    auto get_value(int index) const -> std::optional<Value>
    {
        if (index < 0 || index >= static_cast<int>(value_array.size()))
        {
            return std::nullopt;
        }
        return value_array[index];
    }

    auto get_line_number(int offset) const -> int;
};
