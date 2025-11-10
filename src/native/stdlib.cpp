#include "list.hpp"
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
    else if (val.is_map())
    {
        return {vm->get_allocator()->intern_string("map"), true};
    }
    else
    {
        vm->report_error("internal error: invalid type");
        throw std::logic_error("unhandled type");
        return {Value{}, false};
    }
}

auto native_len(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value val = values[1];
    if (val.is_string())
    {
        auto size = val.as_string()->get().size();
        return {Value{size}, true};
    }
    if (val.is_list())
    {
        auto size = static_cast<ObjectList *>(val.as_object())->size();
        return {Value{size}, true};
    }
    if (val.is_map())
    {
        auto size = static_cast<ObjectMap *>(val.as_object())->size();
        return {Value{size}, true};
    }
    vm->report_error("invalid argument type to call len(), must be a string, list or map");
    return {Value{0}, false};
}

auto native_hash(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value val = values[1];
    auto hash = val.hash_code();
    if (hash == -1)
    {
        vm->report_error("unhashable type");
        return {Value{}, false};
    }
    return {Value{hash}, true};
}

auto native_is_hashable(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value val = values[1];
    auto hash = val.hash_code();
    if (hash == -1)
    {
        return {Value{false}, false};
    }
    return {Value{true}, true};
}

auto register_stdlib(VM *vm) -> void
{
    vm->define_native_function("len", 1, native_len);
    vm->define_native_function("exit", 1, native_exit);
    vm->define_native_function("type", 1, native_type);
    vm->define_native_function("hash", 1, native_hash);
    vm->define_native_function("is_hashable", 1, native_is_hashable);
}