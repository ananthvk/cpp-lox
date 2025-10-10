#include "value.hpp"
#include "vm.hpp"

std::pair<Value, bool> native_stdlib_exit(VM *vm, int arg_count, Value *values)
{
    Value val = values[1];
    if (!val.is_integer())
    {
        vm->report_error("invalid argument type to call exit(), must be an integer");
        return {Value{}, false};
    }
    exit(static_cast<int>(val.as_integer()));
    return {Value{}, true};
}

void register_stdlib(VM *vm) { vm->define_native_function("exit", 1, native_stdlib_exit); }