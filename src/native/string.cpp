#include "function.hpp"
#include "list.hpp"
#include "value.hpp"
#include "vm.hpp"


auto native_to_string(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value val = values[1];
    if (val.is_string())
    {
        // If it's already a string, do nothing
        return {val, true};
    }
    // TODO: optimiize to_string so that it does not create a std::string
    ObjectString *ptr = vm->get_allocator()->intern_string(val.to_string());
    return {Value{ptr}, true};
}

auto register_string(VM *vm) -> void
{
    vm->define_native_function("to_string", 1, native_to_string);
}