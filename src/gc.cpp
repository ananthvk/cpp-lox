#include "gc.hpp"
#include "compiler.hpp"

GarbageCollector::GarbageCollector(VMOpts vm_opts) : vopts(vm_opts), log_indent_level(0) {}

auto GarbageCollector::set_allocator(Allocator *alloc) -> void { allocator = alloc; }

auto GarbageCollector::collect_garbage() -> void
{
    log(fmt::color::green, "{} [VM: {}, Compiler: {}]", "begin", (vm != nullptr) ? "live" : "none",
        Compiler::does_compiler_exist() ? "live" : "none");
    log_indent_level++;
    mark_roots();
    log_indent_level--;
    log(fmt::color::green, "{}", "end");
}

auto GarbageCollector::log_allocation(Object *obj) -> void
{
    log(fmt::color::cyan, "allocate {:<20p} {:16} [value: {}]", static_cast<void *>(obj),
        object_type_to_string(obj->get_type()), Value{obj}.to_string());
}

auto GarbageCollector::log_free(Object *obj) -> void
{
    log(fmt::color::red, "free {:<20p} {:16}", static_cast<void *>(obj),
        object_type_to_string(obj->get_type()));
}

auto GarbageCollector::mark_roots() -> void
{
    if (Compiler::does_compiler_exist())
    {
        log(fmt::color::yellow, "{}", "mark compiler roots");
        log_indent_level++;
        Compiler::mark_compiler_roots(*this);
        log_indent_level--;
        log(fmt::color::yellow, "{}", "end mark compiler roots");
    }
    if (vm != nullptr)
    {
        log(fmt::color::blue, "{}", "mark vm roots");
        log_indent_level++;
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
        log_indent_level--;
        log(fmt::color::blue, "{}", "end mark vm roots");
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
    log(fmt::color::white, "mark {:<20p} {:16} [value: {}]", static_cast<void *>(object),
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