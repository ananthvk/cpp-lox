#pragma once

#include "crc32.hpp"
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

    auto write(const std::filesystem::path &filepath, const SerializedBytecode &bytecode) -> void
    {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            throw new std::system_error(errno, std::generic_category(),
                                        "failed to open bytecode file for writing: " +
                                            std::string(filepath.filename().string()));
        }

        // Calculate offsets
        uint32_t chunk_start_offset = 32; // Header size
        uint32_t global_table_offset =
            chunk_start_offset + static_cast<uint32_t>(bytecode.bytecode.size());
        uint32_t strings_offset =
            global_table_offset + static_cast<uint32_t>(bytecode.globals.size());
        uint32_t file_size = strings_offset + static_cast<uint32_t>(bytecode.strings.size());

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

        // Compute the CRC32 of the rest of the header (excluding the magic and crc placeholder)
        uint32_t crc = Crc32_ComputeBuf(0, header + 8, sizeof(header) - 8);

        // Compute the CRC32 of the chunks
        crc = Crc32_ComputeBuf(crc, bytecode.bytecode.data(), bytecode.bytecode.size());

        // Compute the CRC32 of the globals
        crc = Crc32_ComputeBuf(crc, bytecode.globals.data(), bytecode.globals.size());

        // Compute the CRC32 of the strings
        crc = Crc32_ComputeBuf(crc, bytecode.strings.data(), bytecode.strings.size());

        // Store the checksum in little endian format
        if (datapacker::bytes::encode<datapacker::endian::little>(header +
                                                                      4, // To skip the magic bytes
                                                                  crc)   // CRC-32 checksum
            != 4)
        {
            throw std::logic_error("Invalid number of bytes written for checksum");
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

    // This only checks for the presence of magic bytes
    auto is_compiled_lox_program(const std::filesystem::path &filepath) -> bool
    {
        std::ifstream infile(filepath, std::ios::binary);
        if (!infile.is_open())
        {
            return false;
        }

        std::vector<uint8_t> buffer(32);
        infile.read(reinterpret_cast<char *>(buffer.data()), 32);

        if (infile.gcount() < 32)
        {
            return false;
        }

        if (buffer[0] != MAGIC[0] || buffer[1] != MAGIC[1] || buffer[2] != MAGIC[2] ||
            buffer[3] != MAGIC[3])
        {
            return false;
        }
        return true;
    }

    auto read(const std::filesystem::path &filepath) -> SerializedBytecode
    {
        SerializedBytecode bytecode;
        std::ifstream infile(filepath, std::ios::binary | std::ios::ate);
        if (!infile.is_open())
        {
            throw new std::system_error(errno, std::generic_category(),
                                        "failed to open bytecode file for reading: " +
                                            std::string(filepath.filename().string()));
        }
        auto size = infile.tellg();
        infile.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(size);
        if (!infile.read(reinterpret_cast<char *>(buffer.data()), size))
        {
            throw std::logic_error("failed to read from bytecode file");
        }

        if (buffer.size() < 32)
        {
            throw std::runtime_error("bytecode file is corrupt, < 32 bytes");
        }

        uint8_t magic[4] = {0};
        uint32_t crc = 0;
        uint8_t version_major = 0, version_minor = 0, version_patch = 0;

        uint16_t chunk_count = 0;
        uint32_t chunk_start_offset = 0;
        uint32_t global_table_offset = 0;
        uint32_t strings_offset = 0;
        uint32_t file_size = 0;

        uint8_t reserved0 = 0, reserved1 = 0, reserved2 = 0;

        auto count = datapacker::bytes::decode<datapacker::endian::little>(
            buffer.data(), magic[0], magic[1], magic[2], magic[3], // Magic number (4 bytes)
            crc,                                                   // CRC placeholder (4 bytes)
            version_major,                                         // Version major (1 byte)
            version_minor,                                         // Version minor (1 byte)
            version_patch,                                         // Version patch (1 byte)
            chunk_count,                                           // Number of chunks (2 bytes)
            chunk_start_offset,                                    // Chunk start offset (4 bytes)
            global_table_offset,                                   // Global table offset (4 bytes)
            strings_offset,                                        // Strings offset (4 bytes)
            file_size,                                             // File size (4 bytes)
            reserved0, reserved1, reserved2                        // Reserved (3 bytes)
        );
        if (count != 32)
        {
            throw std::logic_error("Could not load file header");
        }
        // Check magic bytes
        if (magic[0] != MAGIC[0] || magic[1] != MAGIC[1] || magic[2] != MAGIC[2] ||
            magic[3] != MAGIC[3])
        {
            throw std::runtime_error("Not a valid Lox compiled program");
        }

        // Compute the CRC of the file, excluding the magic bytes and the CRC value itself
        uint32_t file_crc = Crc32_ComputeBuf(0, buffer.data() + 8, buffer.size() - 8);

        if (file_crc != crc)
        {
            throw std::runtime_error("Bytecode file corrupted: CRC32 checksum mismatch");
        }

        // Check for compatibility only for major version. Minor versions and patch should remain
        // backward compatible. Reject all forward versions of the bytecode file
        if (version_major != VERSION_MAJOR)
        {
            throw std::runtime_error(
                "Incompatible bytecode version: " + std::to_string(version_major) + "." +
                std::to_string(version_minor) + "." + std::to_string(version_patch) +
                ", expected major version " + std::to_string(VERSION_MAJOR));
        }
        if (version_minor > VERSION_MINOR ||
            (version_minor == VERSION_MINOR && version_patch > VERSION_PATCH))
        {
            throw std::runtime_error(
                "Forward bytecode version not supported: " + std::to_string(version_major) + "." +
                std::to_string(version_minor) + "." + std::to_string(version_patch) +
                ", maximum supported version " + std::to_string(VERSION_MAJOR) + "." +
                std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_PATCH));
        }

        // Check file size consistency
        if (file_size != buffer.size())
        {
            throw std::runtime_error("File size mismatch: header indicates " +
                                     std::to_string(file_size) + " bytes, but file contains " +
                                     std::to_string(buffer.size()) + " bytes");
        }

        SerializedBytecode result;
        result.chunk_count = chunk_count;

        // Extract bytecode section
        size_t bytecode_size = global_table_offset - chunk_start_offset;
        result.bytecode.resize(bytecode_size);
        std::copy(buffer.begin() + chunk_start_offset, buffer.begin() + global_table_offset,
                  result.bytecode.begin());

        // Extract globals section
        size_t globals_size = strings_offset - global_table_offset;
        result.globals.resize(globals_size);
        std::copy(buffer.begin() + global_table_offset, buffer.begin() + strings_offset,
                  result.globals.begin());

        // Extract strings section
        size_t strings_size = buffer.size() - strings_offset;
        result.strings.resize(strings_size);
        std::copy(buffer.begin() + strings_offset, buffer.end(), result.strings.begin());

        return result;
    }
};