#pragma once
#include "object.hpp"

#include "fast_float.h"
#include <fmt/format.h>
#include <math.h>
#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <string>

const double EPSILON = 1e-9;

template <typename... T> struct always_false : std::false_type
{
};

struct Value
{
    enum ValueType : uint8_t
    {
        NIL,
        BOOLEAN,
        NUMBER_REAL,
        NUMBER_INT,
        OBJECT
    };

    bool uninitialized;

    ValueType type;

    union Data
    {
        bool b;
        int64_t i;
        double d;
        Object *o;
    } data;

    auto to_string() const -> std::string;

    auto operator==(Value other) const -> bool;

    template <typename T> Value(T value, bool uninitialized = false)
    {
        this->uninitialized = uninitialized;
        set_value(value);
    }

    template <typename T> auto set_value(T value) -> void
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            type = ValueType::BOOLEAN;
            data.b = value;
        }
        else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>)
        {
            type = ValueType::NUMBER_INT;
            data.i = value;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            type = ValueType::NUMBER_REAL;
            data.d = value;
        }
        else if constexpr (std::is_pointer_v<T> &&
                           std::is_base_of_v<Object, std::remove_pointer_t<T>>)
        {
            type = ValueType::OBJECT;
            data.o = value;
        }
        else
        {
            static_assert(always_false<T>(), "Unsupported type passed");
        }
    }

    auto as_string() const -> ObjectString * { return static_cast<ObjectString *>(data.o); }

    auto as_integer() const -> int64_t { return data.i; }

    auto as_real() const -> double { return data.d; }

    auto as_bool() const -> bool { return data.b; }

    auto as_object() const -> Object * { return data.o; }

    auto is_nil() const -> bool { return type == ValueType::NIL; }

    auto is_bool() const -> bool { return type == ValueType::BOOLEAN; }

    auto is_integer() const -> bool { return type == ValueType::NUMBER_INT; }

    auto is_real() const -> bool { return type == ValueType::NUMBER_REAL; }

    auto is_number() const -> bool { return is_integer() || is_real(); }

    auto is_object() const -> bool { return type == ValueType::OBJECT; }

    auto is_string() const -> bool
    {
        return type == ValueType::OBJECT && data.o->get_type() == ObjectType::STRING;
    }

    auto is_function() const -> bool
    {
        return type == ValueType::OBJECT && data.o->get_type() == ObjectType::FUNCTION;
    }

    auto is_closure() const -> bool
    {
        return type == ValueType::OBJECT && data.o->get_type() == ObjectType::CLOSURE;
    }

    auto is_class() const -> bool
    {
        return type == ValueType::OBJECT && data.o->get_type() == ObjectType::CLASS;
    }

    auto is_instance() const -> bool
    {
        return type == ValueType::OBJECT && data.o->get_type() == ObjectType::INSTANCE;
    }

    auto is_bound_method() const -> bool
    {
        return type == ValueType::OBJECT && data.o->get_type() == ObjectType::BOUND_METHOD;
    }

    auto is_native_function() const -> bool
    {
        return type == ValueType::OBJECT && data.o->get_type() == ObjectType::NATIVE_FUNCTION;
    }

    auto coerce_integer() const -> int64_t
    {
        if (is_integer())
            return data.i;
        if (is_real())
            return static_cast<int64_t>(data.d);
        throw std::logic_error("Invalid type coersion to int");
    }

    auto to_integer_try_parse() const -> std::pair<int64_t, bool>
    {
        if (is_integer())
            return {data.i, true};
        if (is_real())
            return {static_cast<int64_t>(data.d), true};
        if (is_string())
        {
            // Try converting the string to an integer
            auto as_string = to_string();

            int64_t value;
            auto answer = fast_float::from_chars(as_string.data(),
                                                 as_string.data() + as_string.size(), value);
            if (answer.ec != std::errc())
            {
                return {0, false};
            }
            return {value, true};
        }
        return {0, false};
    }

    auto coerce_real() const -> double
    {
        if (is_integer())
            return static_cast<double>(data.i);
        if (is_real())
            return data.d;
        throw std::logic_error("Invalid type coersion to double");
    }

    auto to_double_try_parse() const -> std::pair<double, bool>
    {
        if (is_integer())
            return {static_cast<double>(data.i), true};
        if (is_real())
            return {data.d, true};
        if (is_string())
        {
            // Try converting the string to an integer
            auto as_string = to_string();

            double value;
            auto answer = fast_float::from_chars(as_string.data(),
                                                 as_string.data() + as_string.size(), value);
            if (answer.ec != std::errc())
            {
                return {0, false};
            }
            return {value, true};
        }
        return {0, false};
    }

    auto is_falsey() const -> bool
    {
        if (is_bool())
            return !as_bool();
        if (is_nil())
            return true;
        return false;
    }

    auto is_uninitialized() const -> bool { return uninitialized; }

    auto set_uninitialized(bool new_state) -> void { uninitialized = new_state; }

    Value()
    {
        uninitialized = false;
        type = ValueType::NIL;
        memset(reinterpret_cast<void *>(&data), 0, sizeof(data));
    }
};
