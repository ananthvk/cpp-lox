#include "gc.hpp"

GarbageCollector::GarbageCollector(VMOpts vm_opts) : vopts(vm_opts) {}

auto GarbageCollector::set_allocator(Allocator *alloc) -> void { allocator = alloc; }

auto GarbageCollector::collect_garbage() -> void
{
    log("{:<20} {}", "", "begin");
    mark_roots();
    log("{:<20} {}", "", "end");
}

auto GarbageCollector::log_allocation(Object *obj) -> void
{
    log("{:<20p} allocate {:16} [value: {}]", static_cast<void *>(obj),
        object_type_to_string(obj->get_type()), Value{obj}.to_string());
}

auto GarbageCollector::log_free(Object *obj) -> void
{
    log("{:<20p} free {:16}", static_cast<void *>(obj), object_type_to_string(obj->get_type()));
}

auto GarbageCollector::mark_roots() -> void
{
    if (vm != nullptr)
    {
        // Roots are the objects that are directly accesible by the VM, they include objects on the
        // stack, and the global table
        for (auto slot = vm->evalstack.data(); slot < vm->stack_top; ++slot)
        {
            mark_value(*slot);
        }

        // Mark closures on the call stack
        for (int i = 0; i < vm->frame_count; i++)
        {
            mark_object(vm->frames[i].closure);
        }

        // Mark all open upvalues
        auto current = vm->open_upvalues;
        while (current != nullptr)
        {
            mark_object(current);
            current = current->next;
        }

        mark_global_variables(vm->context);
    }
}

auto GarbageCollector::mark_value(Value value) -> void
{
    // Only mark objects, i.e. bool, nil & numbers are not allocated on the heap
    if (value.is_object())
        mark_object(value.as_object());
}

auto GarbageCollector::mark_object(Object *object) -> void
{
    if (object == nullptr)
        return;
    log("{:<20p} mark {:16} [value: {}]", static_cast<void *>(object),
        object_type_to_string(object->get_type()), Value{object}.to_string());
    object->is_marked = true;
}

auto GarbageCollector::mark_global_variables(Context *context) -> void
{
    // Mark both the keys and the values
    for (auto &slot : context->global_names.get_slots())
    {
        if (slot.state == decltype(context->global_names)::Slot::State::FILLED)
        {
            mark_object(slot.key);
        }
    }
    for (auto &value : context->values)
    {
        mark_value(value.value);
    }
}