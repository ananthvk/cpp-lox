#include "vm.hpp"

auto native_get_bytes_allocated(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto allocator = vm->get_allocator();
    auto bytes = static_cast<int64_t>(allocator->get_bytes_allocated());
    return {Value{bytes}, true};
}

auto native_get_bytes_freed(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto allocator = vm->get_allocator();
    auto bytes = static_cast<int64_t>(allocator->get_bytes_freed());
    return {Value{bytes}, true};
}

auto native_get_next_gc(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto allocator = vm->get_allocator();
    auto threshold = static_cast<int64_t>(allocator->get_next_gc());
    return {Value{threshold}, true};
}

auto native_get_objects_created(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto allocator = vm->get_allocator();
    auto count = static_cast<int64_t>(allocator->get_objects_created());
    return {Value{count}, true};
}

auto native_get_objects_freed(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto allocator = vm->get_allocator();
    auto count = static_cast<int64_t>(allocator->get_objects_freed());
    return {Value{count}, true};
}

auto native_get_live_objects(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto allocator = vm->get_allocator();
    auto count = static_cast<int64_t>(allocator->get_live_objects());
    return {Value{count}, true};
}

auto native_get_net_bytes(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto allocator = vm->get_allocator();
    auto bytes = static_cast<int64_t>(allocator->get_net_bytes());
    return {Value{bytes}, true};
}

auto native_display_gc_stats(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto &os = vm->get_output_stream();
    auto allocator = vm->get_allocator();

    os << "GC Statistics:" << std::endl;
    os << "  Bytes allocated: " << allocator->get_bytes_allocated() << std::endl;
    os << "  Bytes freed: " << allocator->get_bytes_freed() << std::endl;
    os << "  Net bytes: " << allocator->get_net_bytes() << std::endl;
    os << "  Next GC threshold: " << allocator->get_next_gc() << std::endl;
    os << "  Objects created: " << allocator->get_objects_created() << std::endl;
    os << "  Objects freed: " << allocator->get_objects_freed() << std::endl;
    os << "  Live objects: " << allocator->get_live_objects() << std::endl;
    os << "  Cycles: " << allocator->get_gc_cycles() << std::endl;

    return {Value{}, true};
}

auto register_memory(VM *vm) -> void
{
    vm->define_native_function("sys__mem_get_bytes_allocated", 0, native_get_bytes_allocated);
    vm->define_native_function("sys__mem_get_bytes_freed", 0, native_get_bytes_freed);
    vm->define_native_function("sys__mem_get_next_gc", 0, native_get_next_gc);
    vm->define_native_function("sys__mem_get_objects_created", 0, native_get_objects_created);
    vm->define_native_function("sys__mem_get_objects_freed", 0, native_get_objects_freed);
    vm->define_native_function("sys__mem_get_live_objects", 0, native_get_live_objects);
    vm->define_native_function("sys__mem_get_net_bytes", 0, native_get_net_bytes);
    vm->define_native_function("sys__mem_display_gc_stats", 0, native_display_gc_stats);
}
