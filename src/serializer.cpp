#include "serializer.hpp"
#include "context.hpp"
#include "datapacker.hpp"
#include <fmt/format.h>

auto Serializer::serialize_string(ObjectString *string) -> uint32_t
{
    auto iter = strings_offset.find(string);
    if (iter != strings_offset.end())
    {
        return iter->second;
    }
    std::string_view sv = string->get();

    // The string does not exist in the block, create it and return the offset
    if (strings_block.size() > static_cast<size_t>(UINT32_MAX))
    {
        throw std::logic_error("string too large");
    }

    uint32_t offset = static_cast<uint32_t>(strings_block.size());
    auto new_size = strings_block.size() + sizeof(uint32_t) + sv.size();
    strings_block.resize(new_size);
    uint8_t *buf = strings_block.data() + offset;

    uint32_t sz = static_cast<uint32_t>(sv.size());
    auto count =
        datapacker::bytes::encode_length_prefixed<datapacker::endian::little>(buf, sv.data(), sz);
    if (count != (sizeof(uint32_t) + sv.size()))
    {
        throw std::logic_error("string not serialized correctly");
    }
    strings_offset[string] = offset;
    string_count++;
    return offset;
}

auto Serializer::write_chunk_header(uint8_t *buf, ObjectFunction *function, uint32_t chunk_id)
    -> void
{
    Chunk *chunk = function->get();
    char type = 'F';
    uint32_t function_name_index = serialize_string(function->name());
    uint8_t arity = static_cast<uint8_t>(function->arity());
    uint16_t upvalue_count = static_cast<uint16_t>(function->upvalue_count());
    uint16_t constant_pool_size = static_cast<uint16_t>(chunk->get_constants().size());
    uint32_t code_length = static_cast<uint32_t>(chunk->get_code().size());
    uint8_t debug_present = 1;
    uint32_t chunk_id_ = chunk_id;

    uint32_t reserved0 = 0;
    uint8_t reserved1 = 0;

    auto count = datapacker::bytes::encode<datapacker::endian::little>(
        buf, type, function_name_index, arity, upvalue_count, constant_pool_size, code_length,
        debug_present, chunk_id_, reserved0, reserved1);
    if (count != 24)
    {
        throw std::logic_error("invalid number of bytes written for chunk header");
    }
}

auto Serializer::write_constant_pool(uint8_t *buf, ObjectFunction *function) -> void
{
    auto &constants = function->get()->get_constants();
    static_assert(sizeof(constants[0].data) == 8);
    for (auto value : constants)
    {
        switch (value.type)
        {
        case Value::ValueType::NIL:
            *buf++ = 'N';
            break;
        case Value::ValueType::BOOLEAN:
            *buf++ = 'B';
            datapacker::bytes::encode<datapacker::endian::little>(buf,
                                                                  static_cast<bool>(value.data.b));
            break;
        case Value::ValueType::NUMBER_REAL:
            *buf++ = 'R';
            datapacker::bytes::encode<datapacker::endian::little>(
                buf, static_cast<double>(value.data.d));
            break;
        case Value::ValueType::NUMBER_INT:
            *buf++ = 'I';
            datapacker::bytes::encode<datapacker::endian::little>(
                buf, static_cast<int64_t>(value.data.i));
            break;
        case Value::ValueType::OBJECT:
        {
            auto *obj = value.as_object();
            auto type = obj->get_type();
            if (type == ObjectType::STRING)
            {
                *buf++ = 'S';
                // Note: Even though the index is 4 bytes, while writing it as a value,
                // write it as 8 bytes
                uint64_t index = serialize_string(value.as_string());
                datapacker::bytes::encode<datapacker::endian::little>(buf, index);
            }
            else if (type == ObjectType::FUNCTION)
            {
                *buf++ = 'F';
                uint64_t index =
                    serialize_function(static_cast<ObjectFunction *>(value.as_object()));
                datapacker::bytes::encode<datapacker::endian::little>(buf, index);
            }
            else
            {
                throw std::logic_error("invalid object type for serialization");
            }
            break;
        }
        }
        buf += 8;
    }
}

auto Serializer::write_debug_information(uint8_t *buf, ObjectFunction *function) -> void
{
    auto &line_info = function->get()->get_lines();
    // Write the header of 8 bytes
    uint32_t num_pairs = 0;
    uint32_t reserved = 0;
    auto count = datapacker::bytes::encode<datapacker::endian::little>(buf, num_pairs, reserved);
    buf += count;
    if (count != 8)
    {
        throw std::logic_error("error while writing debug info");
    }
    for (auto line : line_info)
    {
        datapacker::bytes::encode<datapacker::endian::little>(
            buf, static_cast<uint32_t>(line.offset), static_cast<uint32_t>(line.line_number));
        buf += 8;
    }
}

auto Serializer::serialize_function(ObjectFunction *function) -> uint32_t
{
    if (chunk_counter == UINT16_MAX)
    {
        throw std::logic_error("too many chunks, can only serialize < 65535 chunks");
    }
    uint32_t current_chunk_id = chunk_counter;
    auto &buffer = chunks[current_chunk_id];
    chunk_counter++;

    Chunk *chunk = function->get();
    size_t current_offset = 0, current_segment_size = 0, new_size = 0;

    // Write the chunk header
    current_segment_size = 24;
    new_size = buffer.size() + current_segment_size;
    buffer.resize(new_size);
    write_chunk_header(buffer.data() + current_offset, function, current_chunk_id);
    current_offset += current_segment_size;

    // Serialize the constant pool
    // The size of the constant pool = 9 * number of constants bytes
    current_segment_size = 9 * chunk->get_constants().size();
    new_size = buffer.size() + current_segment_size;
    buffer.resize(new_size);
    write_constant_pool(buffer.data() + current_offset, function);
    current_offset += current_segment_size;

    // Serialize the bytecode of the function
    current_segment_size = chunk->get_code().size();
    new_size = buffer.size() + current_segment_size;
    buffer.insert(buffer.end(), chunk->get_code().begin(), chunk->get_code().end());
    current_offset += current_segment_size;

    // Serialize the debug information
    // The required size is 8 for the header, with 8 bytes per LineInfo struct
    current_segment_size = 8 + (8 * chunk->get_lines().size());
    new_size = buffer.size() + current_segment_size;
    buffer.resize(new_size);
    write_debug_information(buffer.data() + current_offset, function);
    current_offset += current_segment_size;

    return current_chunk_id;
}

auto Serializer::serialize_symbol_table(Context *context) -> std::vector<uint8_t>
{
    std::vector<uint8_t> buffer;

    // Write the symbol table header
    uint16_t size = static_cast<uint16_t>(context->names.size());
    uint16_t reserved = 0;

    buffer.resize(4);
    uint8_t *buf = buffer.data();
    auto count = datapacker::bytes::encode<datapacker::endian::little>(buf, size, reserved);
    if (count != 4)
    {
        throw std::logic_error("invalid number of bytes written for symbol table header");
    }

    // Write symbol name, index pairs
    auto offset = buffer.size();
    auto new_size = buffer.size() + 8 * context->names.size();
    buffer.resize(new_size);
    buf = buffer.data() + offset;

    for (size_t i = 0; i < context->names.size(); i++)
    {
        uint32_t index = serialize_string(context->names[i]);
        uint8_t is_const = context->values[i].is_const;
        uint16_t reserved0 = 0;
        uint8_t reserved1 = 0;
        buf += datapacker::bytes::encode<datapacker::endian::little>(buf, index, is_const,
                                                                     reserved0, reserved1);
    }
    return buffer;
}

auto Serializer::write_string_block_header() -> void
{
    uint32_t reserved = 0;
    // It's assumed that the size of the string block >= 8 bytes, and the first 8 bytes have been
    // left free for the string header
    if (datapacker::bytes::encode<datapacker::endian::little>(strings_block.data(), string_count,
                                                              reserved) != 8)
    {
        throw std::logic_error("coudl not write string header");
    }
}

auto Serializer::serialize_program(ObjectFunction *function, Context *context) -> SerializedBytecode
{
    // Reset all values before starting serialization
    strings_block.clear();
    // For storing the string header
    strings_block.resize(8);
    strings_offset.clear();
    chunks.clear();
    chunk_counter = 0;
    string_count = 0;

    // Serialize the program

    auto id = serialize_function(function);
    if (id != 0)
    {
        throw std::logic_error("id of main function (script) not 0");
    }

    // Serialize the global values
    auto globals = serialize_symbol_table(context);

    // Serialize the strings, emit the header
    write_string_block_header();

    // Combine all the bytes together

    SerializedBytecode result;
    // Iterate over chunks in reverse order, i.e. the main chunk will be emitted last
    // This is done to simplify the loader, since all references to functions will be resolved
    // before they are called
    std::vector<uint8_t> bytecode;
    for (auto iter = chunks.rbegin(); iter != chunks.rend(); ++iter)
    {
        bytecode.insert(bytecode.end(), iter->second.begin(), iter->second.end());
    }

    result.chunk_count = chunk_counter;
    result.bytecode = std::move(bytecode);
    result.globals = std::move(globals);
    result.strings = std::move(strings_block);
    strings_block.clear();
    return result;
}

auto Serializer::display_serialized(std::ostream &os, const SerializedBytecode &bytecode) -> void
{
    os << "=== Serialized Program ===\n";
    os << "Chunk count: " << bytecode.chunk_count << "\n\n";

    os << "Bytecode (" << bytecode.bytecode.size() << " bytes):\n";
    for (size_t i = 0; i < bytecode.bytecode.size(); ++i)
    {
        os << fmt::format("{:02x}", bytecode.bytecode[i]);
        if (i < bytecode.bytecode.size() - 1)
            os << " ";
    }
    os << "\n\n";

    os << "Strings (" << bytecode.strings.size() << " bytes):\n";
    for (size_t i = 0; i < bytecode.strings.size(); ++i)
    {
        os << fmt::format("{:02x}", bytecode.strings[i]);
        if (i < bytecode.strings.size() - 1)
            os << " ";
    }
    os << "\n\n";

    os << "Globals (" << bytecode.globals.size() << " bytes):\n";
    for (size_t i = 0; i < bytecode.globals.size(); ++i)
    {
        os << fmt::format("{:02x}", bytecode.globals[i]);
        if (i < bytecode.globals.size() - 1)
            os << " ";
    }
    os << "\n";
}