#include "value.hpp"
#include "vm.hpp"
#include <iostream>

auto native_getline(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    std::string line;
    // TODO: Check for failure
    std::getline(std::cin, line);
    auto allocator = vm->get_allocator();
    auto interned_string = allocator->intern_string(line);
    return {Value{interned_string}, true};
}

// Prints all the arguments passed to it, also flushes the output stream at the end
auto native_print(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    // TODO: Add error checking
    auto &os = vm->get_output_stream();
    for (int i = 1; i <= arg_count; i++)
    {
        if (i == 1)
        {
            os << values[i].to_string();
        }
        else
        {
            os << " " << values[i].to_string();
        }
    }
    return {Value{}, true};
}

// Prints all the arguments passed to it, also adds a newline and flushes the output stream at the
// end
auto native_println(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto &os = vm->get_output_stream();
    auto [value, ok] = native_print(vm, arg_count, values);
    if (!ok)
        return {Value{}, ok};
    os << std::endl;
    return {value, true};
}

auto register_stdio(VM *vm) -> void
{
    vm->define_native_function("input", 0, native_getline);
    vm->define_native_function("println", -1, native_println);
    vm->define_native_function("print", -1, native_print);
}