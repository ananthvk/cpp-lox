#pragma once
#include "allocator.hpp"
#include "classes.hpp"
#include "context.hpp"
#include "function.hpp"
#include "serialized_bytecode.hpp"
#include <map>

// TODO: Later make it a zero copy version
// i.e. for strings, and if possible bytecode use the same vector instead of allocating new objects

class Deserializer
{
    std::map<uint32_t, ObjectString *> strings;
    std::map<uint32_t, ObjectFunction *> functions;

    bool is_log_enabled;

    auto deserialize_strings(Allocator &allocator, uint8_t *buffer, uint32_t size) -> void;
    auto deserialize_global_table(Context *context, uint8_t *buffer, uint32_t size) -> void;
    auto deserialize_chunks(Allocator &allocator, uint8_t *buffer, uint32_t size,
                            uint32_t expected_chunk_count) -> void;
    auto deserialize_constant_pool(std::vector<Value> &constant_pool, uint8_t *buffer,
                                   uint32_t constant_pool_size) -> void;

    // Reads the debug info (line-bytecode) mapping, and returns the new offset, i.e. the location
    // of the first byte after the debug info
    auto deserialize_debug_info(Chunk &chunk, uint8_t *buffer, uint32_t offset, uint32_t size)
        -> uint32_t;

  public:
    Deserializer(bool is_log_enabled = false) : is_log_enabled(is_log_enabled) {}

    auto deserialize_program(SerializedBytecode &bytecode, Allocator &allocator, Context *context)
        -> ObjectFunction *;
};