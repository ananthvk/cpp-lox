#include "allocator.hpp"
#include "debug.hpp"
#include "deserializer.hpp"
#include "file_header.hpp"
#include "gc.hpp"
#include "vm.hpp"
#include <filesystem>
#include <fmt/color.h>
#include <fmt/format.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fmt::print(fmt::fg(fmt::color::red), "Usage: {} <compiled_lox_file>\n", argv[0]);
        return 1;
    }

    std::filesystem::path input_path(argv[1]);

    if (!std::filesystem::exists(input_path))
    {
        fmt::print(fmt::fg(fmt::color::red), "Error: File '{}' not found\n", input_path.string());
        return 1;
    }

    FileHeader header;
    if (!header.is_compiled_lox_program(input_path))
    {
        fmt::print(fmt::fg(fmt::color::red), "Error: '{}' is not a compiled Lox program\n",
                   input_path.string());
        return 1;
    }

    auto bytecode = header.read(input_path);

    // Setup VM components for deserialization
    VMOpts vm_opts;
    GarbageCollector gc(vm_opts);
    Allocator allocator(vm_opts);
    allocator.set_gc(&gc);
    gc.set_allocator(&allocator);
    allocator.disable_gc(); // Disable GC during deserialization

    Context context;

    try
    {
        Deserializer deserializer(true);
        fmt::print(fmt::fg(fmt::color::green), "Disassembly of '{}':\n\n", input_path.string());
        deserializer.deserialize_program(bytecode, allocator, &context);
    }
    catch (const std::exception &e)
    {
        fmt::print(fmt::fg(fmt::color::red), "Error during deserialization: {}\n", e.what());
        return 1;
    }

    return 0;
}