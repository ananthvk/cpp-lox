#pragma once
#include "allocator.hpp"
#include "vm.hpp"
#include "vmopts.hpp"
#include <fmt/format.h>

class GarbageCollector
{
    // Non owning pointer to allocator
    Allocator *allocator;
    VM *vm;
    VMOpts vopts;

    template <typename... Args> inline auto log(const std::string &message, Args... args) -> void
    {
        if (vopts.debug_log_gc)
            fmt::println(fmt::runtime(std::string("[GC] ") + message), args...);
    }

    auto mark_roots() -> void;

    auto mark_value(Value value) -> void;

    auto mark_object(Object *object) -> void;

    auto mark_global_variables(Context *context) -> void;

  public:
    GarbageCollector(VMOpts vm_opts);

    auto set_allocator(Allocator *alloc) -> void;

    auto collect_garbage() -> void;

    auto log_allocation(Object *obj) -> void;

    auto log_free(Object *obj) -> void;

    auto set_vm(VM *vm_ptr) -> void { vm = vm_ptr; }
};