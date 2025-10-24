#pragma once
#include "object.hpp"

// Implementation of list similar to CPython's list
// ObjectList is implemented as a dynamic array of Values
// This implementation uses std::vector to simplify development
class ObjectList : public Object
{
    std::vector<Value> values;

    // The constructor creates a list with the specified capacity and length (by default, unset
    // values are nil) len is the number of objects in the list, cap is the capacity of the list.
    // cap has to be >= len
    ObjectList(int64_t len, int64_t cap)
    {
        values.reserve(cap);
        values.resize(len);
    }

  public:
    auto get_type() const -> ObjectType override { return ObjectType::LIST; }

    auto append(Value value) -> void { values.push_back(value); }

    auto size() -> int64_t { return static_cast<int64_t>(values.size()); }

    auto capacity() -> int64_t { return static_cast<int64_t>(values.capacity()); }

    auto get_values() -> std::vector<Value> & { return values; }

    auto get(int64_t index) -> std::optional<Value>
    {
        if (index >= values.size())
            return std::nullopt;
        return values[index];
    }

    auto set(int64_t index, Value value) -> bool
    {
        if (index >= values.size())
        {
            return false;
        }
        values[index] = value;
        return true;
    }

    auto pop() -> std::optional<Value>
    {
        if (values.empty())
            return std::nullopt;
        auto value = values.back();
        values.pop_back();
        return value;
    }

    // TODO: Implement sort, reverse, contains, slice, erase, copy

    auto operator==(const ObjectList &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectList(const ObjectList &other) = delete;
    ObjectList &operator=(const ObjectList &other) = delete;

    // Move not allowed
    ObjectList(ObjectList &&other) noexcept = delete;
    ObjectList &operator=(ObjectList &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};