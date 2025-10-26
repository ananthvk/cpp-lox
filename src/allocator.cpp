#include "allocator.hpp"
#include "gc.hpp"

auto Allocator::set_gc(GarbageCollector *garbage_collector) -> void { gc = garbage_collector; }

auto Allocator::hash_string(const char *str, size_t length) const -> uint32_t
{
    // FNV-1a hash function from the book
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return hash;
}

auto Allocator::check_interned(const char *str, size_t length)
    -> std::pair<ObjectString *, uint32_t>
{
    // Check if the string is already interned
    uint32_t hash = hash_string(str, length);
    InternedString characters{str, length, hash};
    auto interned_objstring = interned_strings.get(characters);
    return std::make_pair(interned_objstring.value_or(nullptr), hash);
}

/**
 * If `storage_type` is `StorageType::DYNAMIC`, the allocator creates
 * another buffer to hold the copy of the string, and that buffer is managed by the allocator.
 *
 * If `storage_type` is `StorageType::TAKE_OWNERSHIP`, the allocator take ownership of the
 * passed buffer, and manages it. The buffer must be created only using new[], since the
 * allocator will call *delete[]* to destroy the buffer.
 */
auto Allocator::intern_string(const char *str, size_t length, StorageType storage_type)
    -> ObjectString *
{
    if (vopts.debug_stress_gc)
        collect_garbage();
    auto [interned_string, hash] = check_interned(str, length);

    if (interned_string)
    {
        if (storage_type == StorageType::DYNAMIC)
            return interned_string;
        else
        {
            // Delete the passed string since it already exists in memory
            delete[] str;
            return interned_string;
        }
    }
    create_object<ObjectString>();

    if (storage_type == StorageType::DYNAMIC)
    {
        // TODO: Use placement new (no flexible array members in C++) to create the ObjectString
        // & the buffer at the same location.
        char *buffer = new char[length + 1];
        memcpy(buffer, str, length);
        buffer[length] = '\0';
        auto obj = new ObjectString(buffer, length, hash);
        obj->is_marked = false;
        objs.push_back(obj);
        if (vopts.debug_log_gc)
            gc->log_allocation(obj);

        // Add the string to the string pool
        InternedString intern{buffer, length, hash};
        interned_strings.insert(intern, obj);

        return obj;
    }
    else if (storage_type == StorageType::TAKE_OWNERSHIP)
    {
        auto obj = new ObjectString(str, length, hash);
        obj->is_marked = false;
        objs.push_back(obj);
        if (vopts.debug_log_gc)
            gc->log_allocation(obj);

        // Add the string to the string pool
        InternedString intern{str, length, hash};
        interned_strings.insert(intern, obj);

        return obj;
    }
    throw std::logic_error("Invalid storage type");
}

auto Allocator::intern_string(std::string_view sv, StorageType storage_type) -> ObjectString *
{
    return intern_string(sv.data(), sv.size(), storage_type);
}

auto Allocator::new_class(ObjectString *name) -> ObjectClass *
{
    if (vopts.debug_stress_gc)
        collect_garbage();
    create_object<ObjectClass>();

    auto obj = new ObjectClass(name);
    obj->is_marked = false;
    objs.push_back(obj);

    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    return obj;
}

auto Allocator::new_list(int64_t length, int64_t capacity, Value default_) -> ObjectList *
{
    if (vopts.debug_stress_gc)
        collect_garbage();
    create_object<ObjectList>();

    auto obj = new ObjectList(length, capacity, default_);
    obj->is_marked = false;
    objs.push_back(obj);

    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    return obj;
}

auto Allocator::new_instance(ObjectClass *class_) -> ObjectInstance *
{
    if (vopts.debug_stress_gc)
        collect_garbage();
    create_object<ObjectInstance>();

    auto obj = new ObjectInstance(class_);
    obj->is_marked = false;
    objs.push_back(obj);

    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    return obj;
}

auto Allocator::new_bound_method(Value receiver, ObjectClosure *method) -> ObjectBoundMethod *
{
    if (vopts.debug_stress_gc)
        collect_garbage();
    create_object<ObjectBoundMethod>();

    auto obj = new ObjectBoundMethod(receiver, method);
    obj->is_marked = false;
    objs.push_back(obj);

    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    return obj;
}

auto Allocator::new_function(int arity, std::string_view name) -> ObjectFunction *
{
    if (vopts.debug_stress_gc)
        collect_garbage();
    auto chunk = std::make_unique<Chunk>();
    auto interned_name = intern_string(name);
    temp_stash.push_back(Value{interned_name});

    create_object<ObjectFunction>();
    auto obj = new ObjectFunction(arity, std::move(chunk), interned_name);
    obj->is_marked = false;
    objs.push_back(obj);
    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    temp_stash.pop_back();
    return obj;
}

auto Allocator::new_native_function(int arity, NativeFunction function) -> ObjectNativeFunction *
{
    if (vopts.debug_stress_gc)
        collect_garbage();
    create_object<ObjectNativeFunction>();
    auto obj = new ObjectNativeFunction(arity, function);
    obj->is_marked = false;
    objs.push_back(obj);
    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    return obj;
}

auto Allocator::new_closure(ObjectFunction *function) -> ObjectClosure *
{
    if (vopts.debug_stress_gc)
        collect_garbage();

    create_object<ObjectClosure>();
    auto obj = new ObjectClosure(function);

    obj->is_marked = false;
    objs.push_back(obj);
    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    return obj;
}

auto Allocator::new_upvalue(Value *slot) -> ObjectUpvalue *
{
    if (vopts.debug_stress_gc)
        collect_garbage();

    create_object<ObjectUpvalue>();
    auto obj = new ObjectUpvalue(slot);
    obj->is_marked = false;
    objs.push_back(obj);
    if (vopts.debug_log_gc)
        gc->log_allocation(obj);
    return obj;
}

auto Allocator::free_object(Object *obj) -> void
{
    if (obj == nullptr)
        return;
    if (vopts.debug_log_gc)
        gc->log_free(obj);
    delete_object(obj);
    delete obj;
}

auto Allocator::collect_garbage() -> void
{
    if (gc_disabled)
        return;
    gc->collect_garbage();
    gc_cycles++;
}

Allocator::~Allocator()
{
    for (Object *obj : objs)
    {
        free_object(obj);
    }
}

auto Allocator::remove_unused_strings() -> int
{
    auto &slots = interned_strings.get_slots();
    int count = 0;
    for (int i = 0; i < slots.size(); i++)
    {
        if (slots[i].state == decltype(interned_strings)::Slot::State::FILLED &&
            !(slots[i].value->is_marked))
        {
            interned_strings.erase(slots[i].key);
            count++;
        }
    }
    return count;
}