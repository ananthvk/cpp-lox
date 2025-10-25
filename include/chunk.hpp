#pragma once
#include <assert.h>
#include <optional>
#include <stdexcept>
#include <vector>

#include "opcode.hpp"
#include "value.hpp"

class Chunk
{
  public:
    /*
     * Marks the start of a line. Line `line_number` starts at the byte with offset `offset`
     */
    struct LineInfo
    {
        int offset;
        int line_number;
    };

  private:
    std::vector<uint8_t> code;
    std::vector<Value> value_array;
    std::vector<LineInfo> lines;

    auto add_line_number(int offset, int line) -> void;

  public:
    auto write_simple_op(OpCode op, int line) -> void;

    auto write_byte(uint8_t byte, int line) -> void;

    auto write_uint16_le(uint16_t bytes, int line) -> void;

    auto write_load_constant(int index, int line) -> void;

    auto add_constant(Value value) -> int
    {
        // Pushing & popping the temporary value is not necessary here
        // because the vector is not managed by the gc. If in the future push_back
        // causes a gc reallocation, it might be required here
        if (value_array.size() > 0xFFFF)
            throw std::logic_error("Too many constants in chunk");
        value_array.push_back(value);
        return static_cast<int>(value_array.size() - 1);
    }

    template <typename T> auto add_constant(T value) -> int { return add_constant(Value(value)); }

    auto get_code() const -> const std::vector<uint8_t> &;

    auto get_constants() -> std::vector<Value> & { return value_array; }

    auto get_lines() -> std::vector<LineInfo> & { return lines; }

    auto get_code() -> std::vector<uint8_t> &;

    auto get_value(int index) const -> std::optional<Value>;

    auto get_value_unchecked(int index) const -> Value;

    auto get_line_number(int offset) const -> int;
};
