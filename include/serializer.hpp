#pragma once
#include "classes.hpp"
#include "compiler_opts.hpp"
#include "context.hpp"
#include "function.hpp"
#include "object.hpp"
#include "serialized_bytecode.hpp"
#include <map>
#include <ostream>
#include <stddef.h>
#include <stdint.h>
#include <vector>

class Serializer
{
    std::vector<uint8_t> strings_block;

    // Since the language interns all strings, we can de-duplicate by using only the pointer
    std::map<ObjectString *, uint32_t> strings_offset;

    // Mapping of chunk id to bytes of the serialized function
    std::map<uint32_t, std::vector<uint8_t>> chunks;

    uint32_t chunk_counter = 0;

    uint32_t string_count = 0;

    CompilerOpts opts;

    auto write_chunk_header(uint8_t *buf, ObjectFunction *function, uint32_t chunk_id) -> void;
    auto write_constant_pool(uint8_t *buf, ObjectFunction *function) -> void;
    auto write_debug_information(uint8_t *buf, ObjectFunction *function) -> void;

    // Serializes the string into a length prefixed format, stores the string data internally, and
    // returns the offset of the string from the start of the strings block. To get the strings
    // block (containing all serialized strings, call get_strings()). It also de-duplicates strings,
    // if the function is called with the same string, the same offset is returned.
    auto serialize_string(ObjectString *string) -> uint32_t;

    // Serializes the function into bytes, returns a chunk id that can be used to get the bytes of
    // the function. This function recursively serializes functions present in the constant table of
    // the function, and will be stored internally. Call get_chunks() to retrieve all the chunks.
    // Note: This function should only be called once for each ObjectFunction* because it does not
    // de-duplicate based on ObjectFunction*, so duplicate chunks will be generated. In the current
    // design, the compiler creates each function and adds it to only a single constants table of
    // the surrounding scope.
    auto serialize_function(ObjectFunction *function) -> uint32_t;

    // This function is used to serialize the global variable table
    auto serialize_symbol_table(Context *context) -> std::vector<uint8_t>;

    auto write_string_block_header() -> void;

  public:
    Serializer(const CompilerOpts &opts) : opts(opts) {}

    auto serialize_program(ObjectFunction *function, Context *context) -> SerializedBytecode;

    auto display_serialized(std::ostream &os, const SerializedBytecode &bytecode) -> void;
};