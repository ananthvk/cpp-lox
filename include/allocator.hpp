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

    size_t bytes_allocated;
    size_t bytes_freed;
    size_t next_gc;
    size_t objects_created;
    size_t objects_freed;

    // Tempory stack to stash values so that they don't get garbage collected
    std::vector<Value> temp_stash;

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

    // These two functions exist to capture statistics about garbage collection, and to trigger
    // garbage collection when it exceeds threshold
    template <typename T> auto create_object(T *ptr)
    {
        auto object_size = sizeof(std::remove_pointer_t<T>);
        bytes_allocated += object_size;
        objects_created++;

        // TODO: Use this previous if statement
        // and spam the folowing statement in repl to trigger a crash
        // check why it crashes
        // sys__mem_display_gc_stats();
        // if (bytes_allocated > next_gc)
        if (get_net_bytes() > next_gc)
            collect_garbage();
    }

    auto delete_object(Object *obj)
    {
        size_t object_size = 0;
        switch (obj->get_type())
        {
        case ObjectType::STRING:
            object_size = sizeof(ObjectString);
            break;
        case ObjectType::FUNCTION:
            object_size = sizeof(ObjectFunction);
            break;
        case ObjectType::CLOSURE:
            object_size = sizeof(ObjectClosure);
            break;
        case ObjectType::UPVALUE:
            object_size = sizeof(ObjectUpvalue);
            break;
        case ObjectType::NATIVE_FUNCTION:
            object_size = sizeof(ObjectNativeFunction);
            break;
        default:
            throw std::logic_error("invalid object type");
            break;
        }
        bytes_freed += object_size;
        objects_freed++;
    }


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

    Allocator(VMOpts vm_opts)
        : gc(nullptr), vopts(vm_opts), bytes_allocated(0), bytes_freed(0),
          next_gc(DEFAULT_GC_NEXT_COLLECTION), objects_created(0), objects_freed(0)
    {
    }

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

    // Removes all ObjectString* from the table that has is_marked set to false, i.e. they are
    // marked for getting sweeped It returns the number of strings removed
    auto remove_unused_strings() -> int;

    auto set_next_gc(size_t next) { next_gc = next; }

    auto get_bytes_allocated() const -> size_t { return bytes_allocated; }

    auto get_bytes_freed() const -> size_t { return bytes_freed; }

    auto get_next_gc() const -> size_t { return next_gc; }

    auto get_objects_created() const -> size_t { return objects_created; }

    auto get_objects_freed() const -> size_t { return objects_freed; }

    auto get_live_objects() const -> size_t { return objects_created - objects_freed; }

    auto get_net_bytes() const -> size_t { return bytes_allocated - bytes_freed; }

    auto get_temp_stash() -> std::vector<Value> & { return temp_stash; }

    ~Allocator();
};