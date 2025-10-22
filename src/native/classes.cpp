#include "value.hpp"
#include "vm.hpp"

// Checks if the first argument is an instance, and the second argument is a string
auto validate_arguments(VM *vm, Value *values, std::string_view method_name) -> bool
{
    auto instance = values[1];
    auto name = values[2];
    if (!instance.is_instance())
    {
        vm->report_error(
            "invalid argument type to call {}(object, name), object must be an instance",
            method_name);
        return false;
    }
    if (!name.is_string())
    {
        vm->report_error("invalid argument type to call {}(object, name), property name "
                         "must be a string",
                         method_name);
        return false;
    }
    return true;
}

auto native_has_property(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    if (!validate_arguments(vm, values, "has_property"))
        return {Value{}, false};

    auto property_name = values[2].as_string();
    return {Value{static_cast<ObjectInstance *>(values[1].as_object())
                      ->get_fields()
                      .contains(property_name)},
            true};
}

auto native_get_property(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    if (!validate_arguments(vm, values, "get_property"))
        return {Value{}, false};

    auto property_name = values[2].as_string();
    auto instance = static_cast<ObjectInstance *>(values[1].as_object());
    auto value = instance->get_fields().get(property_name);
    if (value)
        return {Value{value.value()}, true};
    else
    {
        vm->report_error("{} has not property '{}'", Value{instance}.to_string(),
                         property_name->get());
        return {Value{}, false};
    }
}

auto native_set_property(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    if (!validate_arguments(vm, values, "set_property"))
        return {Value{}, false};

    auto property_name = values[2].as_string();
    auto instance = static_cast<ObjectInstance *>(values[1].as_object());
    instance->get_fields().get_ref(property_name) = values[3];
    return {Value{}, true};
}

auto native_del_property(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    if (!validate_arguments(vm, values, "del_property"))
        return {Value{}, false};

    auto property_name = values[2].as_string();
    auto instance = static_cast<ObjectInstance *>(values[1].as_object());
    auto is_deleted = instance->get_fields().erase(property_name);
    return {Value{is_deleted}, true};
}

auto register_classes(VM *vm) -> void
{
    vm->define_native_function("has_property", 2, native_has_property);
    vm->define_native_function("get_property", 2, native_get_property);
    vm->define_native_function("set_property", 3, native_set_property);
    vm->define_native_function("del_property", 2, native_del_property);
}