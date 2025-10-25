#pragma once
#include "classes.hpp"
#include "function.hpp"
#include "object.hpp"
#include <map>
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

    auto write_chunk_header(uint8_t *buf, ObjectFunction *function, uint32_t chunk_id) -> void;
    auto write_constant_pool(uint8_t *buf, ObjectFunction *function) -> void;
    auto write_debug_information(uint8_t *buf, ObjectFunction *function) -> void;


  public:
    // Serializes the string into a length prefixed format, stores the string data internally, and
    // returns the offset of the string from the start of the strings block. To get the strings
    // block (containing all serialized strings, call get_strings()). It also de-duplicates strings,
    // if the function is called with the same string, the same offset is returned.
    auto serialize_string(ObjectString *string) -> uint32_t;

    // Serializes the function into bytes, returns a chunk id that can be used to get the bytes of
    // the function. This function recursively serializes functions present in the constant table of
    // the function, and will be stored internally. Call get_chunks() to retrieve all the chunks
    auto serialize_function(ObjectFunction *function) -> uint32_t;

    // Returns a reference to the internal strings block containing serialized string data
    // Used for accessing or modifying the byte array that stores string literals
    auto get_strings() -> std::vector<uint8_t> & { return strings_block; }

    auto get_chunks() -> std::map<uint32_t, std::vector<uint8_t>> & { return chunks; }
};