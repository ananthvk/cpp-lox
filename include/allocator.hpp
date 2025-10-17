#pragma once
#include "function.hpp"
#include "hashmap.hpp"
#include "object.hpp"
#include "utils.hpp"
#include "vmopts.hpp"
#include <stdexcept>
#include <vector>

class GarbageCollector;

/**
 * For now, this allocator class just creates the object and returns it's address
 * Later, it can be improved to an arena allocator / pool allocator
 */
class Allocator
{
  private:
    std::vector<Object *> objs;
    // Non-owning pointer to a garbage collector
    GarbageCollector *gc;
    VMOpts vopts;

    // This is basically an ObjectString, but with a different name. Idk why I have this here
    struct InternedString
    {
        const char *str;
        size_t length;
        uint32_t hash;
    };

    struct InternedStringHasher
    {
        auto operator()(InternedString str) const -> size_t { return str.hash; }
    };

    struct InternedStringCmp
    {
        auto operator()(InternedString first, InternedString second) const -> size_t
        {
            if (first.length != second.length)
                return false;
            if (first.hash != second.hash)
                return false;
            return strncmp(first.str, second.str, first.length) == 0;
        }
    };

    HashMap<InternedString, ObjectString *, InternedStringHasher, InternedStringCmp>
        interned_strings;


  public:
    /**
     * Storage type defines if the allocator needs to allocate memory to hold a copy of the data or
     * not.
     */
    enum class StorageType
    {
        DYNAMIC,
        TAKE_OWNERSHIP
    };

    Allocator(VMOpts vm_opts) : gc(nullptr), vopts(vm_opts) {}

    auto set_gc(GarbageCollector *garbage_collector) -> void;

    auto hash_string(const char *str, size_t length) const -> uint32_t;

    auto check_interned(const char *str, size_t length) -> std::pair<ObjectString *, uint32_t>;

    auto intern_string(const char *str, size_t length,
                       StorageType storage_type = StorageType::DYNAMIC) -> ObjectString *;

    auto intern_string(std::string_view sv, StorageType storage_type = StorageType::DYNAMIC)
        -> ObjectString *;

    auto new_function(int arity, std::string_view name) -> ObjectFunction *;

    auto new_native_function(int arity, NativeFunction function) -> ObjectNativeFunction *;

    auto new_closure(ObjectFunction *function) -> ObjectClosure *;

    auto new_upvalue(Value *slot) -> ObjectUpvalue *;

    auto collect_garbage() -> void;

    auto free_object(Object *object) -> void;

    // Get a vector of all objects allocated by the allocator
    auto get_objects() -> std::vector<Object *> & { return objs; }
    
    // Removes all ObjectString* from the table that has is_marked set to false, i.e. they are marked for getting sweeped
    // It returns the number of strings removed
    auto remove_unused_strings() -> int;

    ~Allocator();
};