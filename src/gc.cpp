#include "gc.hpp"

GarbageCollector::GarbageCollector(Allocator *allocator, VMOpts vm_opts)
    : allocator(allocator), vopts(vm_opts)
{
}

auto GarbageCollector::collect_garbage() -> void
{
    log("begin");
    log("end");
}

auto GarbageCollector::log_allocation(Object *obj) -> void
{
    log("{:10p} allocate {:16} [value: {}]", static_cast<void *>(obj),
        object_type_to_string(obj->get_type()), Value{obj}.to_string());
}

auto GarbageCollector::log_free(Object *obj) -> void
{
    log("{:12p} free {} [value: {}]", static_cast<void *>(obj),
        object_type_to_string(obj->get_type()), Value{obj}.to_string());
}