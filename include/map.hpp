#pragma once
#include "object.hpp"
#include "table.hpp"
#include "value.hpp"
#include <optional>
#include <vector>

// Implementation of map similar to ObjectList
// ObjectMap is implemented using ValueValueTable (HashMap)
class ObjectMap : public Object
{
    ValueValueTable table;

    // Private constructor for friend classes
    ObjectMap() = default;

  public:
    auto get_type() const -> ObjectType override { return ObjectType::MAP; }

    auto size() -> int64_t { return static_cast<int64_t>(table.size()); }

    // Maps are not hashable
    auto hash_code() const -> int64_t override { return -1; }

    auto get(const Value &key) -> std::optional<Value> { return table.get(key); }

    auto get(const Value &key, const Value &default_value) -> Value
    {
        auto val = table.get(key);
        if (!val)
            return default_value;
        return val.value();
    }

    auto set(const Value &key, const Value &value) -> bool
    {
        // Check if the key is hashable
        if (key.hash_code() == -1)
        {
            // Key is not hashable (like lists or maps)
            return false;
        }
        table[key] = value;
        return true;
    }

    auto has(const Value &key) -> bool { return table.contains(key); }

    auto remove(const Value &key) -> bool { return table.erase(key); }

    auto clear() -> void { table.clear(); }

    auto keys() -> std::vector<Value>
    {
        // TODO: Later implement some sort of iterator access so that these vectors need not be
        // created
        std::vector<Value> result;
        result.reserve(table.size());
        auto &slots = table.get_slots();
        for (auto slot : slots)
        {
            if (slot.state == ValueValueTable::Slot::State::FILLED)
            {
                result.push_back(slot.key);
            }
        }
        return result;
    }

    auto values() -> std::vector<Value>
    {
        // TODO: Later implement some sort of iterator access so that these vectors need not be
        // created
        std::vector<Value> result;
        result.reserve(table.size());
        auto &slots = table.get_slots();
        for (auto slot : slots)
        {
            if (slot.state == ValueValueTable::Slot::State::FILLED)
            {
                result.push_back(slot.value);
            }
        }
        return result;
    }

    auto get_table() -> ValueValueTable & { return table; }

    auto operator==(const ObjectMap &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectMap(const ObjectMap &other) = delete;
    ObjectMap &operator=(const ObjectMap &other) = delete;

    // Move not allowed
    ObjectMap(ObjectMap &&other) noexcept = delete;
    ObjectMap &operator=(ObjectMap &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};
