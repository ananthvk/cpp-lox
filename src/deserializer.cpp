#include "deserializer.hpp"
#include "datapacker.hpp"
#include "debug.hpp"
#include <fmt/format.h>

auto Deserializer::deserialize_strings(Allocator &allocator, uint8_t *buffer, uint32_t size) -> void
{
    // Read the strings header
    uint32_t string_count = 0;
    uint32_t string_block_size = 0;

    if (size < 8)
    {
        throw std::logic_error(
            "bytecode load error: too small strings block, should be >= 8 bytes");
    }
    if (datapacker::bytes::decode<datapacker::endian::little>(buffer, string_count,
                                                              string_block_size) != 8)
    {
        throw std::runtime_error("bytecode load error: strings block header could not be read");
    }
    if (size != string_block_size)
    {
        throw std::runtime_error("bytecode load error: invalid string block size, expected " +
                                 std::to_string(string_block_size) + " bytes, got " +
                                 std::to_string(size) + " bytes");
    }
    uint32_t offset = 8; // Header size

    // Read all the strings
    while (offset < size)
    {
        // Check if we have enough bytes to read the string length
        if (offset + 4 > size)
        {
            throw std::runtime_error(
                "bytecode load error: insufficient bytes to read string length");
        }
        // Read the length of the string
        uint32_t current_string_length = 0;
        uint32_t current_string_offset = offset;

        datapacker::bytes::decode<datapacker::endian::little>(buffer + offset,
                                                              current_string_length);
        offset += 4;

        // Check if we have enough bytes to read the string data
        if (offset + current_string_length > size)
        {
            throw std::runtime_error("bytecode load error: insufficient bytes to read string data");
        }
        // Read the string (with offset 4) for skipping the length
        std::string_view sv{reinterpret_cast<const char *>(buffer + offset), current_string_length};
        offset += current_string_length;

        ObjectString *obj = allocator.intern_string(sv);
        // Add it to the map
        strings[current_string_offset] = obj;
    }

    if (is_log_enabled)
    {
        fmt::println("=== strings block [size: {} bytes, count: {}] ===", size, string_count);
        // Display all strings
        for (auto rec : strings)
        {
            fmt::println("{:#010x} | '{}'", rec.first, rec.second->get());
        }
        fmt::println("");
    }
}

// It is assumed that context is empty when calling this function, since otherwise the index used in
// the program will not match with the contents stored in the values array within context. TODO: In
// the future, try offsetting this so that programs will load from a base global index
auto Deserializer::deserialize_global_table(Context *context, uint8_t *buffer, uint32_t size)
    -> void
{
    // Read the global table header
    uint16_t num_entries = 0;
    uint16_t reserved = 0;

    if (size < 4)
    {
        throw std::logic_error(
            "bytecode load error: too small global symbol table, should be >= 4 bytes");
    }
    if (datapacker::bytes::decode<datapacker::endian::little>(buffer, num_entries, reserved) != 4)
    {
        throw std::runtime_error(
            "bytecode load error: global symbol table header could not be read");
    }
    if (size != (num_entries * 8 + 4))
    {
        throw std::runtime_error(
            "bytecode load error: invalid global symbol table size, expected " +
            std::to_string(num_entries * 8 + 4) + " bytes, got " + std::to_string(size) + " bytes");
    }

    uint32_t offset = 4; // Header size
    if (is_log_enabled)
    {
        fmt::println("=== global table [size: {} bytes, entries: {}] ===", size, num_entries);
    }
    for (int i = 0; i < num_entries; i++)
    {
        uint32_t name_index = 0;
        uint8_t is_const = 0;

        uint16_t reserved0 = 0;
        uint8_t reserved1 = 0;

        if (datapacker::bytes::decode<datapacker::endian::little>(
                buffer + offset, name_index, is_const, reserved0, reserved1) != 8)
        {
            throw std::runtime_error(
                "bytecode load error: global symbol table entry could not be read");
        }

        auto iter = strings.find(name_index);
        if (iter == strings.end())
        {
            throw std::runtime_error(
                "bytecode load error: global symbol table references invalid string index " +
                std::to_string(name_index));
        }
        auto index = context->get_global(iter->second);
        if (index != i)
        {
            throw std::logic_error(
                "bytecode load error: context (global table) is not empty when loading bytecode");
        }
        context->get_internal_value(index).is_const = is_const;
        if (is_log_enabled)
        {
            fmt::println("{:010d} | [{}] '{}'", i, is_const ? "const" : "     ",
                         iter->second->get());
        }
        offset += 8;
    }
    if (is_log_enabled)
    {
        fmt::println("");
    }
}

auto Deserializer::deserialize_constant_pool(std::vector<Value> &constant_pool, uint8_t *buffer,
                                             uint32_t constant_pool_size) -> void
{
    constant_pool.resize(constant_pool_size);

    for (int i = 0; i < constant_pool_size; i++)
    {
        uint8_t type = 0;
        uint64_t value = 0;
        datapacker::bytes::decode<datapacker::endian::little>(buffer, type, value);
        switch (type)
        {
        case 'N':
            constant_pool[i].type = Value::ValueType::NIL;
            break;
        case 'B':
            constant_pool[i].type = Value::ValueType::BOOLEAN;
            constant_pool[i].data.b = (value == 0) ? false : true;
            break;
        case 'R':
        {
            double real_val = 0;
            datapacker::bytes::decode<datapacker::endian::little>(buffer + 1, real_val);
            constant_pool[i].type = Value::ValueType::NUMBER_REAL;
            constant_pool[i].data.d = real_val;
            break;
        }
        case 'I':
            constant_pool[i].type = Value::ValueType::NUMBER_INT;
            constant_pool[i].data.i = value;
            break;
        case 'S':
        {
            if (value >= UINT32_MAX)
            {
                throw std::logic_error(
                    "bytecode load error: constant pool references invalid string index");
            }
            auto iter = strings.find(static_cast<uint32_t>(value));
            if (iter == strings.end())
            {
                throw std::runtime_error(
                    "bytecode load error: constant pool references invalid string index " +
                    std::to_string(value));
            }
            constant_pool[i].type = Value::ValueType::OBJECT;
            constant_pool[i].data.o = iter->second;
            break;
        }
        case 'F':
        {
            if (value >= UINT32_MAX)
            {
                throw std::logic_error(
                    "bytecode load error: constant pool references invalid function index");
            }
            auto iter = functions.find(static_cast<uint32_t>(value));
            if (iter == functions.end())
            {
                // This should not happen in a correctly generated program because the chunks
                // are stored in reverse so all references should get resolved before they are
                // used
                throw std::runtime_error(
                    "bytecode load error: constant pool references invalid function index " +
                    std::to_string(value));
            }
            constant_pool[i].type = Value::ValueType::OBJECT;
            constant_pool[i].data.o = iter->second;
            break;
        }
        default:
            throw std::runtime_error("bytecode load error: invalid value type in constant pool");
            break;
        }
        buffer += 9;
    }
}

auto Deserializer::deserialize_debug_info(Chunk &chunk, uint8_t *buffer, uint32_t offset,
                                          uint32_t size) -> uint32_t
{
    uint32_t count = 0;
    uint32_t reserved = 0;

    if (offset + 8 > size)
    {
        throw std::runtime_error(
            "bytecode load error: insufficient bytes to read debug info header");
    }
    if (datapacker::bytes::decode<datapacker::endian::little>(buffer + offset, count, reserved) !=
        8)
    {
        throw std::runtime_error("bytecode load error: debug info header could not be read");
    }

    offset += 8;
    if (is_log_enabled)
    {
        fmt::println("debug info [count: {}, size: {} bytes]", count, count * 8 + 8);
    }

    // Check if minimum n * 8 bytes are available for the pairs
    if (offset + (count * 8) > size)
    {
        throw std::runtime_error(
            "bytecode load error: insufficient bytes to read debug info pairs");
    }

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t code_offset = 0;
        uint32_t line_number = 0;

        if (datapacker::bytes::decode<datapacker::endian::little>(buffer + offset, code_offset,
                                                                  line_number) != 8)
        {
            throw std::runtime_error("bytecode load error: debug info pair could not be read");
        }

        Chunk::LineInfo info;
        info.line_number = line_number;
        info.offset = code_offset;
        chunk.get_lines().push_back(info);
        offset += 8;
    }

    return offset;
}

auto Deserializer::deserialize_chunks(Allocator &allocator, Context *context, uint8_t *buffer,
                                      uint32_t size, uint32_t expected_chunk_count) -> void
{
    uint32_t offset = 0;
    while (offset < size)
    {
        // Check if the chunk header can be read
        if (offset + 24 > size)
        {
            throw std::runtime_error(
                "bytecode load error: insufficient bytes to read chunk header");
        }

        // Read the chunk header
        char type = 0;
        uint32_t function_name_index = 0;
        uint8_t arity = 0;
        uint16_t upvalue_count = 0;
        uint16_t constant_pool_size = 0;
        uint32_t code_length = 0;
        uint8_t debug_info_present = 0;
        uint32_t id = 0;

        uint32_t reserved0 = 0;
        uint8_t reserved1 = 0;

        if (datapacker::bytes::decode<datapacker::endian::little>(
                buffer + offset, type, function_name_index, arity, upvalue_count,
                constant_pool_size, code_length, debug_info_present, id, reserved0,
                reserved1) != 24)
        {
            throw std::runtime_error("bytecode load error: chunk header could not be read");
        }

        // Check if it's a function
        if (type != 'F')
        {
            throw std::runtime_error("bytecode load error: invalid chunk type, expected 'F'");
        }
        offset += 24;

        // Get the function name
        auto name_iter = strings.find(function_name_index);
        if (name_iter == strings.end())
        {
            throw std::logic_error(
                "bytecode load error: function references invalid string index " +
                std::to_string(function_name_index));
        }

        if (is_log_enabled)
        {
            fmt::println("=== chunk {} [name: '{}'] ===", id, name_iter->second->get());
            fmt::println("arity: {}", arity);
            fmt::println("upvalue count: {}", upvalue_count);
            fmt::println("constant pool size: {}", constant_pool_size);
            fmt::println("code length: {}", code_length);
            fmt::println("debug info present: {}", debug_info_present ? "yes" : "no");
        }

        // Read the constant pool
        auto chunk = std::make_unique<Chunk>();
        auto &constant_pool = chunk->get_constants();

        if (offset + (constant_pool_size * 9) > size)
        {
            throw std::runtime_error(
                "bytecode load error: insufficient bytes to read chunk constant pool values");
        }

        deserialize_constant_pool(constant_pool, buffer + offset, constant_pool_size);
        offset += (constant_pool_size * 9);

        if (is_log_enabled)
        {
            fmt::println("constant pool [size: {} bytes, entries: {}]", constant_pool_size * 9,
                         constant_pool_size);
            for (int i = 0; i < constant_pool_size; i++)
            {
                fmt::println("{:05d} | '{}'", i, constant_pool[i].to_string());
            }
        }

        // Read the code
        if (offset + code_length > size)
        {
            throw std::runtime_error("bytecode load error: insufficient bytes to read chunk code");
        }
        chunk->get_code().insert(chunk->get_code().end(), buffer + offset,
                                 buffer + offset + code_length);
        offset += code_length;

        // Read the optional debug info
        if (debug_info_present == 1)
        {
            offset = deserialize_debug_info(*chunk, buffer, offset, size);
        }

        if (is_log_enabled)
        {
            fmt::print("code [{} bytes]\n", chunk->get_code().size());
            disassemble_chunk(*chunk, std::string(name_iter->second->get()), context, false);
        }

        auto obj =
            allocator.new_function(arity, name_iter->second, upvalue_count, std::move(chunk));

        // Check for duplicate chunks
        if (functions.find(id) != functions.end())
        {
            throw std::runtime_error("bytecode load error: duplicate chunk id " +
                                     std::to_string(id));
        }
        functions[id] = obj;
        if (is_log_enabled)
        {
            fmt::println("");
        }
    }
}

// This function takes a program as serialized bytecode, and returns an ObjectFunction*. It
// creates all required ObjectFunction* and ObjectString*. The gc is disabled before
// deserialization so that objects don't get freed, and the gc is enabled before the
// function returns. In case an exception is thrown, it's the responsibility of the caller
// to re-enable the gc (if needed)
// TODO: Later make a zero copy version that directly takes three uint8_t* and sizes
auto Deserializer::deserialize_program(SerializedBytecode &bytecode, Allocator &allocator,
                                       Context *context) -> ObjectFunction *
{
    // Clear any existing state
    strings.clear();
    functions.clear();

    allocator.disable_gc();

    if (bytecode.strings.size() >= UINT32_MAX)
    {
        throw std::runtime_error("bytecode load error: constant strings block too large");
    }
    // First deserialize strings since they are used by both the global table as well as the
    // chunks
    deserialize_strings(allocator, bytecode.strings.data(),
                        static_cast<uint32_t>(bytecode.strings.size()));

    // Then deserialize the global names
    if (bytecode.globals.size() >= (8 * static_cast<size_t>(UINT16_MAX)))
    {
        throw std::runtime_error("bytecode load error: global symbol table is too large");
    }
    deserialize_global_table(context, bytecode.globals.data(),
                             static_cast<uint32_t>(bytecode.globals.size()));

    // Then deserialize all the chunks
    if (bytecode.bytecode.size() >= UINT32_MAX)
    {
        throw std::runtime_error("bytecode load error: bytecode block too large");
    }
    deserialize_chunks(allocator, context, bytecode.bytecode.data(),
                       static_cast<uint32_t>(bytecode.bytecode.size()), bytecode.chunk_count);

    // Check if the main function (id 0) exists
    auto main_iter = functions.find(0);
    if (main_iter == functions.end())
    {
        throw std::runtime_error("bytecode load error: main function with id 0 does not exist");
    }
    allocator.enable_gc();
    return main_iter->second;
}