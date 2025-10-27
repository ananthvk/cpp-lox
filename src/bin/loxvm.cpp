#include "allocator.hpp"
#include "deserializer.hpp"
#include "file_header.hpp"
#include "gc.hpp"
#include "vmopts.hpp"
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

    VMOpts vm_opts;

    ErrorReporter reporter;
    GarbageCollector gc(vm_opts);
    Allocator allocator(vm_opts);
    allocator.set_gc(&gc);
    gc.set_allocator(&allocator);

    Context context;

    VM vm(vm_opts, reporter, allocator, &context);
    gc.set_vm(&vm);

    Deserializer deserialzer;
    auto obj = deserialzer.deserialize_program(bytecode, allocator, &context);

    allocator.disable_gc();
    vm.register_native_functions();
    allocator.enable_gc();

    auto result = vm.run(obj, std::cout);

    if (reporter.has_error() || result != InterpretResult::OK)
    {
        reporter.display(stderr);
        return 1;
    }
    return 0;
}