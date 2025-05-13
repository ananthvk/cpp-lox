#pragma once

#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <string>

struct Value
{
    enum ValueType : uint8_t
    {
        BOOLEAN,
        NUMBER_DOUBLE,
        NUMBER_INT
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
        case Value::NUMBER_DOUBLE:
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
};
