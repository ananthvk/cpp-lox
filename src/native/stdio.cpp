#include "value.hpp"
#include "vm.hpp"
#include <iostream>

std::pair<Value, bool> native_stdio_getline(VM *vm, int arg_count, Value *values)
{
    std::string line;
    // TODO: Check for failure
    std::getline(std::cin, line);
    auto allocator = vm->get_allocator();
    auto interned_string = allocator->intern_string(line);
    return {Value{interned_string}, true};
}

// Prints all the arguments passed to it, also flushes the output stream at the end
std::pair<Value, bool> native_stdio_print(VM *vm, int arg_count, Value *values)
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
std::pair<Value, bool> native_stdio_println(VM *vm, int arg_count, Value *values)
{
    auto &os = vm->get_output_stream();
    auto [value, ok] = native_stdio_print(vm, arg_count, values);
    if (!ok)
        return {Value{}, ok};
    os << std::endl;
    return {value, true};
}

void register_stdio(VM *vm)
{
    vm->define_native_function("input", 0, native_stdio_getline);
    vm->define_native_function("println", -1, native_stdio_println);
    vm->define_native_function("print", -1, native_stdio_print);
}