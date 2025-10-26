#pragma once

struct CompilerOpts
{
    bool debug_print_tokens = false;

    bool dump_function_bytecode = false;

    // Used when writing debug information (bytecode offset - line mapping) to output file
    bool emit_debug_information = true;
};