#include "value.hpp"
#include "vm.hpp"

auto native_exit(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
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

auto native_type(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value val = values[1];
    if (val.is_integer())
    {
        return {vm->get_allocator()->intern_string("int"), true};
    }
    else if (val.is_real())
    {
        return {vm->get_allocator()->intern_string("double"), true};
    }
    else if (val.is_bool())
    {
        return {vm->get_allocator()->intern_string("bool"), true};
    }
    else if (val.is_nil())
    {
        return {vm->get_allocator()->intern_string("nil"), true};
    }
    else if (val.is_string())
    {
        return {vm->get_allocator()->intern_string("string"), true};
    }
    else if (val.is_function())
    {
        return {vm->get_allocator()->intern_string("function"), true};
    }
    else if (val.is_native_function())
    {
        return {vm->get_allocator()->intern_string("native_function"), true};
    }
    else if (val.is_class())
    {
        return {vm->get_allocator()->intern_string("class"), true};
    }
    else if (val.is_instance())
    {
        return {vm->get_allocator()->intern_string("instance"), true};
    }
    else if (val.is_list())
    {
        return {vm->get_allocator()->intern_string("list"), true};
    }
    else
    {
        vm->report_error("internal error: invalid type");
        throw std::logic_error("unhandled type");
        return {Value{}, false};
    }
}

auto register_stdlib(VM *vm) -> void
{
    vm->define_native_function("exit", 1, native_exit);
    vm->define_native_function("type", 1, native_type);
}