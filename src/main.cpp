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
    chunk.add_constant(v);

    v.data.b = true;
    v.type = Value::BOOLEAN;
    chunk.add_constant(v);
    chunk.add_constant(v);
    chunk.add_constant(v);
    chunk.add_constant(v);

    chunk.write_load_constant(0, 123);
    chunk.write_load_constant(1, 124);
    chunk.write_load_constant(2, 124);
    chunk.write_load_constant(0, 124);
    chunk.write_load_constant(5112, 125);
    chunk.write_load_constant(256, 126);
    chunk.write_load_constant(16000, 127);
    chunk.write_load_constant(160000, 128);
    chunk.write_simple_op(OpCode::RETURN, 101);

    disassemble_chunk(chunk, "program");
}