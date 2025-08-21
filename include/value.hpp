#pragma once
#include "object.hpp"

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

    auto to_string() const -> std::string
    {
        switch (type)
        {
        case Value::NIL:
            return "nil";
        case Value::NUMBER_REAL:
            return fmt::format("{:.6g}", data.d);
        case Value::NUMBER_INT:
            return std::to_string(data.i);
        case Value::OBJECT:
        {
            switch (data.o->get_type())
            {
            case ObjectType::STRING:
                return std::string(static_cast<ObjectString *>(data.o)->get());
            default:
                break;
            }
            break;
        }

        case Value::BOOLEAN:
            if (data.b)
                return "true";
            else
                return "false";
            break;
        }
        throw std::logic_error("Invalid value type passed to to_string");
    }

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

    auto coerce_integer() const -> int64_t
    {
        if (is_integer())
            return data.i;
        if (is_real())
            return static_cast<int64_t>(data.d);
        throw std::logic_error("Invalid type coersion to int");
    }

    auto coerce_real() const -> double
    {
        if (is_integer())
            return static_cast<double>(data.i);
        if (is_real())
            return data.d;
        throw std::logic_error("Invalid type coersion to double");
    }

    auto is_falsey() const -> bool
    {
        if (is_bool())
            return !as_bool();
        if (is_nil())
            return true;
        return is_nil() || (is_bool() && !as_bool());

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

    auto operator==(Value other) const -> bool
    {
        if (type != other.type)
            return false;
        switch (type)
        {
        case ValueType::NIL:
            return true;
        case ValueType::BOOLEAN:
            return as_bool() == other.as_bool();
        case ValueType::NUMBER_INT:
            return as_integer() == other.as_integer();
        case ValueType::OBJECT:
            switch (data.o->get_type())
            {
            case ObjectType::STRING:
                return *as_string() == *other.as_string();
            }
            return false;
        case Value::NUMBER_REAL:
            // Note: Comparing doubles like this is incorrect due to floating point precision errors
            // TODO: Fix this by either defining an epsilon, or do not allow == between doubles
            return fabs(as_real() - other.as_real()) < EPSILON;
        }
        return false;
    }
};
