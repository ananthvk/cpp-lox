#pragma once
#include "hashmap.hpp"
#include "object.hpp"
#include "value.hpp"

struct StringValueTableHasher
{
    auto operator()(const ObjectString *str) const -> size_t { return str->hash(); }
};

using StringValueTable = HashMap<ObjectString *, Value, StringValueTableHasher>;

struct ValueValueTableHasher
{
    auto operator()(const Value &value) const -> size_t { return value.hash_code(); }
};

using ValueValueTable = HashMap<Value, Value, ValueValueTableHasher>;