#include "value.hpp"
#include "vm.hpp"

auto native_assert(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto value = values[1];
    auto message = values[2];
    // If the value evaluates to true, do nothing
    if (!value.is_falsey())
    {
        return {Value{}, true};
    }
    vm->report_error("assertion failed: {}", message.to_string());
    return {Value{}, false};
}

auto register_test(VM *vm) -> void { vm->define_native_function("assert", 2, native_assert); }