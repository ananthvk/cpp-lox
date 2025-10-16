#pragma once
#include "hashmap.hpp"
#include "object.hpp"
#include "value.hpp"

class Context
{
  public:
    struct GlobalValue
    {
        Value value;
        bool defined;
        bool initialized;
        bool is_const;

        GlobalValue() : value(), defined(false), initialized(false), is_const(false) {}
    };

  private:
    struct StringIndexTableHasher
    {
        auto operator()(const ObjectString *str) const -> size_t { return str->hash(); }
    };

    using StringIndexTable = HashMap<ObjectString *, int, StringIndexTableHasher>;

    StringIndexTable global_names;
    std::vector<GlobalValue> values;
    std::vector<ObjectString *> names;

  public:
    auto get_global(ObjectString *name) -> int
    {
        auto val = global_names.get(name);
        /*
         * The global name exists in the table, return its index
         */
        if (val)
        {
            return val.value();
        }

        // Otherwise add the global to the table
        values.push_back(GlobalValue{});
        global_names.insert(name, static_cast<int>(values.size() - 1));
        names.push_back(name);
        return static_cast<int>(values.size() - 1);
    }

    auto exists(ObjectString *name) -> bool { return global_names.contains(name); }

    auto exists(int index) -> bool { return index < values.size() && index >= 0; }

    auto get_value(int index) -> Value & { return values[index].value; }

    auto get_internal_value(int index) -> GlobalValue & { return values[index]; }

    auto is_initialized(int index) const -> bool { return values[index].initialized; }

    auto is_defined(int index) const -> bool { return values[index].defined; }

    auto set_initialized(int index, bool new_state) -> void
    {
        values[index].initialized = new_state;
    }

    auto set_defined(int index, bool new_state) -> void { values[index].defined = new_state; }

    /**
     * Only useful when debugging / or when you want to get the variable name corresponding to an
     * index
     */
    auto get_name(int index) const -> ObjectString * { return names[index]; }

    friend class GarbageCollector;
};