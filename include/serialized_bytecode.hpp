#pragma once
#include <stddef.h>
#include <stdint.h>
#include <vector>

struct SerializedBytecode
{
    uint32_t chunk_count = 0;      // Number of chunks
    std::vector<uint8_t> bytecode; // Bytecode of all chunks concatenated together
    std::vector<uint8_t> strings;  // All strings compacted together (with the header)
    std::vector<uint8_t> globals;  // Global name table (with the header)
};
