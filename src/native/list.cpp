#include "list.hpp"
#include "value.hpp"
#include "vm.hpp"

auto native_list(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    int64_t len = 0;
    int64_t cap = 0;
    Value default_val = Value{}; // nil

    if (arg_count >= 1)
    {
        if (!values[1].is_integer())
        {
            vm->report_error("first argument len, to list(len, default, cap) must be an integer");
            return {Value{}, false};
        }
        len = static_cast<int64_t>(values[1].as_integer());
    }
    if (arg_count >= 2)
    {
        default_val = values[2];
    }
    if (arg_count >= 3)
    {
        if (!values[3].is_integer())
        {
            vm->report_error("third argument cap, to list(len, default, cap) must be an integer");
            return {Value{}, false};
        }
        cap = static_cast<int64_t>(values[3].as_integer());
    }

    ObjectList *list = vm->get_allocator()->new_list(len, cap, default_val);
    return {Value{list}, true};
}

auto native_pop_list(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value list_val = values[1];

    if (!list_val.is_list())
    {
        vm->report_error("argument to pop() must be a list");
        return {Value{}, false};
    }

    ObjectList *list = static_cast<ObjectList *>(list_val.as_object());

    if (list->size() == 0)
    {
        vm->report_error("cannot pop from empty list");
        return {Value{}, false};
    }
    return {list->pop().value(), true};
}

auto native_cap_list(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value list_val = values[1];
    if (!list_val.is_list())
    {
        vm->report_error("argument to pop() must be a list");
        return {Value{}, false};
    }
    ObjectList *list = static_cast<ObjectList *>(list_val.as_object());
    return {list->capacity(), true};
}

auto register_list(VM *vm) -> void
{
    vm->define_native_function("list", -1, native_list);
    vm->define_native_function("pop", 1, native_pop_list);
    vm->define_native_function("cap", 1, native_cap_list);
}