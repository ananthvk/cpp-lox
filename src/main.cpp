#include "chunk.hpp"
#include "debug.hpp"
#include "logger.hpp"
#include <fmt/format.h>
#include <iostream>

int main()
{
    Chunk chunk;
    Value v;
    v.data.d = 3.1415;
    v.type = Value::NUMBER_DOUBLE;
    auto constant_index = chunk.add_constant(v);

    v.data.b = true;
    v.type = Value::BOOLEAN;
    auto constant_index_2 = chunk.add_constant(v);

    chunk.write_code(OpCode::LOAD_CONSTANT, 100);
    chunk.write_code(static_cast<uint8_t>(constant_index), 100);
    chunk.write_code(OpCode::LOAD_CONSTANT, 101);
    chunk.write_code(static_cast<uint8_t>(constant_index_2), 101);
    chunk.write_code(OpCode::RETURN, 101);

    disassemble_chunk(chunk, "program");
}