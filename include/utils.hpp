#pragma once
#include "hashmap.hpp"
#include "object.hpp"
#include "value.hpp"
#include <fmt/format.h>
#include <string>

inline auto escape_string(const std::string_view &input) -> std::string
{
    std::string escaped;
    for (char c : input)
    {
        switch (c)
        {
        case '\n':
            escaped += "\\n";
            break;
        case '\t':
            escaped += "\\t";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        default:
            if (static_cast<unsigned char>(c) < 32 || static_cast<unsigned char>(c) > 126)
            {
                escaped += fmt::format("\\x{:02X}", static_cast<unsigned char>(c));
            }
            else
            {
                escaped += c;
            }
        }
    }
    return escaped;
}

struct VMStringValueTableHasher
{
    auto operator()(const ObjectString *str) const -> size_t { return str->hash(); }
};

using VMStringValueTable = HashMap<ObjectString *, Value, VMStringValueTableHasher>;
