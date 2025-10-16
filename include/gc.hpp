#pragma once
#include "allocator.hpp"
#include "vmopts.hpp"
#include <fmt/format.h>

class GarbageCollector
{
    // Non owning pointer to allocator
    Allocator *allocator;
    VMOpts vopts;

    template <typename... Args> inline auto log(const std::string &message, Args... args) -> void
    {
        if (vopts.debug_log_gc)
            fmt::println(fmt::runtime(std::string("[GC] ") + message), args...);
    }

  public:
    GarbageCollector(Allocator *allocator, VMOpts vm_opts);

    auto collect_garbage() -> void;

    auto log_allocation(Object *obj) -> void;

    auto log_free(Object *obj) -> void;
};