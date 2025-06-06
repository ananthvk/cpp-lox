#include "chunk.hpp"

auto Chunk::write_simple_op(OpCode op, int line) -> void
{
    add_line_number(code.size(), line);
    code.push_back(static_cast<uint8_t>(op));
}

auto Chunk::write_byte(uint8_t byte, int line) -> void
{
    add_line_number(code.size(), line);
    code.push_back(byte);
}

auto Chunk::write_load_constant(int index, int line) -> void
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

auto Chunk::add_line_number(int offset, int line) -> void
{
    if (lines.size() == 0 || lines.back().line_number != line)
    {
        LineInfo info = {offset, line};
        lines.push_back(info);
    }
}

auto Chunk::get_line_number(int offset) const -> int
{
    // Not efficient: Since it traverses the entire line info array to find the line number
    // TODO: Improve it
    if (lines.size() == 0)
    {
        return -1;
    }
    if (lines.size() == 1)
    {
        return lines[0].line_number;
    }
    for (int i = 1; i < static_cast<int>(lines.size()); i++)
    {
        if (lines[i].offset > offset)
        {
            return lines[i - 1].line_number;
        }
        if (lines[i].offset == offset)
        {
            return lines[i].line_number;
        }
    }
    return lines.back().line_number;
}

auto Chunk::get_code() const -> const std::vector<uint8_t> & { return code; }

auto Chunk::get_code() -> std::vector<uint8_t> & { return code; }

auto Chunk::get_value(int index) const -> std::optional<Value>
{
    if (index < 0 || index >= static_cast<int>(value_array.size()))
    {
        return std::nullopt;
    }
    return value_array[index];
}

auto Chunk::get_value_unchecked(int index) const -> Value { return value_array[index]; }
