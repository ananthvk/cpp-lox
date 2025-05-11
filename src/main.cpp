#include "chunk.hpp"
#include "debug.hpp"
#include "logger.hpp"
#include <fmt/format.h>
#include <iostream>

int main()
{
    Chunk chunk;
    chunk.write_code(OpCode::RETURN);
    disassemble_chunk(chunk, "program");
}