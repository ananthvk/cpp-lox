#include "map.hpp"
#include "list.hpp"
#include "value.hpp"
#include "vm.hpp"

auto native_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    ObjectMap *map = vm->get_allocator()->new_map();
    return {Value{map}, true};
}

// Handles both list & map append
auto native_append_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    if (arg_count < 2)
    {
        vm->report_error("too few arguments to call append(), expected either 2 or 3 arguments");
        return {Value{}, false};
    }
    Value first_val = values[1];

    // Handle list append
    if (first_val.is_list())
    {
        if (arg_count != 2)
        {
            vm->report_error(
                "too many arguments to call append(list), expected 2 arguments, got {} arguments",
                arg_count);
            return {Value{}, false};
        }
        Value element = values[2];
        ObjectList *list = static_cast<ObjectList *>(first_val.as_object());
        list->append(element);
        return {Value{}, true};
    }

    // Handle map append
    if (!first_val.is_map())
    {
        if (arg_count != 3)
        {
            vm->report_error(
                "too many arguments to call append(map), expected 3 arguments, got {} arguments",
                arg_count);
            return {Value{}, false};
        }
        vm->report_error("first argument to append() must be a list or map");
        return {Value{}, false};
    }

    Value key = values[2];
    Value value = values[3];

    // Check if key is hashable (not a list or map)
    if (key.hash_code() == -1)
    {
        vm->report_error("unhashable type: lists and maps cannot be used as map keys");
        return {Value{}, false};
    }

    ObjectMap *map = static_cast<ObjectMap *>(first_val.as_object());
    map->set(key, value);
    return {Value{}, true};
}

auto native_delete_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value first_val = values[1];

    // Handle list delete
    if (first_val.is_list())
    {
        Value index_val = values[2];

        if (!index_val.is_integer())
        {
            vm->report_error("second argument to delete() must be an integer ");
            return {Value{}, false};
        }

        ObjectList *list = static_cast<ObjectList *>(first_val.as_object());
        int64_t index = static_cast<int64_t>(index_val.as_integer());

        if (index >= list->size())
        {
            vm->report_error("error while calling delete(list,index) index out of bounds");
            return {Value{}, false};
        }

        list->erase(index);
        return {Value{}, true};
    }

    // Handle map delete
    if (!first_val.is_map())
    {
        vm->report_error("first argument to delete() must be a list or map");
        return {Value{}, false};
    }

    Value key = values[2];
    ObjectMap *map = static_cast<ObjectMap *>(first_val.as_object());
    bool removed = map->remove(key);
    return {Value{removed}, true};
}

auto native_keys_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value map_val = values[1];

    if (!map_val.is_map())
    {
        vm->report_error("argument to keys() must be a map");
        return {Value{}, false};
    }

    ObjectMap *map = static_cast<ObjectMap *>(map_val.as_object());
    ObjectList *keys_list = vm->get_allocator()->new_list(0, map->size());

    for (auto slot : map->get_table().get_slots())
    {
        if (slot.state == ValueValueTable::Slot::State::FILLED)
        {
            keys_list->append(slot.key);
        }
    }

    return {Value{keys_list}, true};
}

auto native_values_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value map_val = values[1];

    if (!map_val.is_map())
    {
        vm->report_error("argument to values() must be a map");
        return {Value{}, false};
    }

    ObjectMap *map = static_cast<ObjectMap *>(map_val.as_object());
    ObjectList *values_list = vm->get_allocator()->new_list(0, map->size());

    for (auto slot : map->get_table().get_slots())
    {
        if (slot.state == ValueValueTable::Slot::State::FILLED)
        {
            // Safe since no GC reallocation is triggered here in this implementation
            values_list->append(slot.value);
        }
    }

    return {Value{values_list}, true};
}

auto native_has_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value map_val = values[1];
    Value key = values[2];

    if (!map_val.is_map())
    {
        vm->report_error("first argument to has() must be a map");
        return {Value{}, false};
    }

    ObjectMap *map = static_cast<ObjectMap *>(map_val.as_object());
    bool has_key = map->has(key);
    return {Value{has_key}, true};
}

auto native_clear_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value val = values[1];

    if (val.is_map())
    {
        ObjectMap *map = static_cast<ObjectMap *>(val.as_object());
        map->clear();
        return {Value{}, true};
    }

    if (val.is_list())
    {
        ObjectList *list = static_cast<ObjectList *>(val.as_object());
        list->get_values().clear();
        return {Value{}, true};
    }

    vm->report_error("argument to clear() must be a map or list");
    return {Value{}, false};
}

auto native_get_map(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    Value map_val = values[1];
    Value key = values[2];
    Value default_val = Value{}; // nil

    if (arg_count == 3)
    {
        default_val = values[3];
    }

    if (!(arg_count == 2 || arg_count == 3))
    {
        vm->report_error("too many arguments to call get(map, key, default), expected either 2 or "
                         "3 arguments, got {}",
                         arg_count);
        return {Value{}, false};
    }

    if (!map_val.is_map())
    {
        vm->report_error("first argument to get() must be a map");
        return {Value{}, false};
    }

    ObjectMap *map = static_cast<ObjectMap *>(map_val.as_object());

    auto val = map->get(key);
    if (val)
    {
        return {val.value(), true};
    }
    else
    {
        return {default_val, true};
    }
}

auto register_map(VM *vm) -> void
{
    vm->define_native_function("map", 0, native_map);
    vm->define_native_function("append", -1, native_append_map);
    vm->define_native_function("delete", 2, native_delete_map);
    vm->define_native_function("keys", 1, native_keys_map);
    vm->define_native_function("values", 1, native_values_map);
    vm->define_native_function("has", 2, native_has_map);
    vm->define_native_function("clear", 1, native_clear_map);
    vm->define_native_function("get", -1, native_get_map);
}