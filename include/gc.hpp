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
    int log_indent_level;

    template <typename... Args>
    inline auto log(decltype(fmt::color::red) color, const std::string &message, Args... args)
        -> void
    {
        if (vopts.debug_log_gc)
        {
            fmt::print("[GC] {}", fmt::format("{:\t>{}}", "", log_indent_level));
            fmt::print(fmt::fg(color), fmt::runtime(message), args...);
            fmt::println("");
        }
    }

    auto mark_roots() -> void;

    auto mark_global_variables(Context *context) -> void;

  public:
    GarbageCollector(VMOpts vm_opts);

    auto set_allocator(Allocator *alloc) -> void;

    auto collect_garbage() -> void;

    auto log_allocation(Object *obj) -> void;

    auto log_free(Object *obj) -> void;

    auto set_vm(VM *vm_ptr) -> void { vm = vm_ptr; }

    auto mark_object(Object *object) -> void;

    auto mark_value(Value value) -> void;
};