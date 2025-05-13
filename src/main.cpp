#include "debug.hpp"
#include "logger.hpp"
#include "vm.hpp"
#include <fmt/format.h>
#include <iostream>

int main()
{
    Chunk chunk;
    for (int i = 0; i < 254; i++)
    {
        chunk.add_constant(i);
    }

    chunk.write_load_constant(chunk.add_constant(1.2), 1);
    chunk.write_load_constant(chunk.add_constant(3.4), 1);
    chunk.write_simple_op(OpCode::ADD, 1);

    chunk.write_load_constant(chunk.add_constant(5.6), 2);
    chunk.write_simple_op(OpCode::DIVIDE, 2);
    chunk.write_simple_op(OpCode::NEGATE, 3);
    chunk.write_simple_op(OpCode::RETURN, 3);

    disassemble_chunk(chunk, "program");
    fmt::println("\n== Begin execution ===");

    VMOpts opts;
    opts.debug_trace_execution = true;
    opts.debug_trace_value_stack = true;
    VM vm(opts);
    vm.run(&chunk);
}