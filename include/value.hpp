#pragma once
#include <string.h>

#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <string>

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
    };

    ValueType type;

    union Data
    {
        bool b;
        int64_t i;
        double d;
    } data;

    auto to_string() const -> std::string
    {
        switch (type)
        {
        case Value::NIL:
            return "nil";
        case Value::NUMBER_REAL:
            return std::to_string(data.d);
        case Value::NUMBER_INT:
            return std::to_string(data.i);

        case Value::BOOLEAN:
            if (data.b)
                return "true";
            else
                return "false";
            break;
        default:
            throw std::logic_error("Invalid value type passed to to_string");
        }
    }

    template <typename T> Value(T value) { set_value(value); }

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
        else
        {
            static_assert(always_false<T>(), "Unsupported type passed");
        }
    }

    auto as_integer() const -> int64_t { return data.i; }

    auto as_real() const -> double { return data.d; }

    auto as_bool() const -> bool { return data.b; }

    auto is_nil() const -> bool { return type == ValueType::NIL; }

    auto is_bool() const -> bool { return type == ValueType::BOOLEAN; }

    auto is_integer() const -> bool { return type == ValueType::NUMBER_INT; }

    auto is_real() const -> bool { return type == ValueType::NUMBER_REAL; }

    auto is_number() const -> bool { return is_integer() || is_real(); }

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
        return true;
    }

    Value()
    {
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
        case Value::NUMBER_REAL:
            // Note: Comparing doubles like this is incorrect due to floating point precision errors
            // TODO: Fix this by either defining an epsilon, or do not allow == between doubles
            return as_real() == other.as_real();
        }
    }
};
