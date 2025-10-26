#pragma once

#include "datapacker.hpp"
#include "serializer.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

class FileHeader
{
  public:
    static constexpr uint8_t MAGIC[4] = {0x00, 0x4C, 0x4F, 0x58};
    static constexpr uint8_t VERSION_MAJOR = 1;
    static constexpr uint8_t VERSION_MINOR = 0;
    static constexpr uint8_t VERSION_PATCH = 0;

    void write(const std::filesystem::path &filepath, const SerializedBytecode &bytecode)
    {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open output file for writing");
        }

        // Calculate offsets
        uint32_t chunk_start_offset = 32; // Header size
        uint32_t global_table_offset =
            chunk_start_offset + static_cast<uint32_t>(bytecode.bytecode.size());
        uint32_t strings_offset =
            global_table_offset + static_cast<uint32_t>(bytecode.globals.size());
        uint32_t file_size = strings_offset + static_cast<uint32_t>(bytecode.strings.size());

        // TODO: Implement CRC of file data later

        uint8_t header[32];
        auto count = datapacker::bytes::encode<datapacker::endian::little>(
            header, MAGIC[0], MAGIC[1], MAGIC[2], MAGIC[3], // Magic number (4 bytes)
            static_cast<uint32_t>(0),                       // CRC placeholder (4 bytes)
            VERSION_MAJOR,                                  // Version major (1 byte)
            VERSION_MINOR,                                  // Version minor (1 byte)
            VERSION_PATCH,                                  // Version patch (1 byte)
            static_cast<uint16_t>(bytecode.chunk_count),    // Number of chunks (2 bytes)
            chunk_start_offset,                             // Chunk start offset (4 bytes)
            global_table_offset,                            // Global table offset (4 bytes)
            strings_offset,                                 // Strings offset (4 bytes)
            file_size,                                      // File size (4 bytes)
            uint8_t(0), uint8_t(0), uint8_t(0)              // Reserved (3 bytes)
        );

        if (count != 32)
        {
            throw std::logic_error("Invalid number of bytes written for file header");
        }

        // Write header
        file.write(reinterpret_cast<const char *>(header), 32);
        if (!file.good())
        {
            throw std::runtime_error("Failed to write file header");
        }

        // Write bytecode (chunks)
        file.write(reinterpret_cast<const char *>(bytecode.bytecode.data()),
                   bytecode.bytecode.size());
        if (!file.good())
        {
            throw std::runtime_error("Failed to write bytecode");
        }

        // Write globals
        file.write(reinterpret_cast<const char *>(bytecode.globals.data()),
                   bytecode.globals.size());
        if (!file.good())
        {
            throw std::runtime_error("Failed to write globals");
        }

        // Write strings
        file.write(reinterpret_cast<const char *>(bytecode.strings.data()),
                   bytecode.strings.size());
        if (!file.good())
        {
            throw std::runtime_error("Failed to write strings");
        }
    }
};