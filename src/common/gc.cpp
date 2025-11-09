#include "gc.hpp"
#include "compiler_fwd.hpp"
#include "math.h"

GarbageCollector::GarbageCollector(VMOpts vm_opts) : vopts(vm_opts), log_indent_level(0) {}

auto GarbageCollector::set_allocator(Allocator *alloc) -> void { allocator = alloc; }

auto GarbageCollector::collect_garbage() -> void
{

    log(fmt::color::green, "{} [VM: {}, Compiler: {}]", "begin", (vm != nullptr) ? "live" : "none",
        Compiler::does_compiler_exist() ? "live" : "none");
    auto bytes_freed_initial = allocator->get_bytes_freed();
    log_indent_level++;
    mark_roots();
    trace_references();
    int count = allocator->remove_unused_strings();
    if (count > 0)
        log(fmt::color::green, "removed {} strings from string pool", count);
    sweep();
    auto bytes_freed = allocator->get_bytes_freed() - bytes_freed_initial;

    // Set the threshold for next garbage collection
    auto gc_current_mem = allocator->get_net_bytes();

    // Grow linearly, but have a minimum heap size before collection
    auto next_gc = std::max(static_cast<unsigned long>(vopts.gc_heap_grow_factor * gc_current_mem),
                            static_cast<unsigned long>(vopts.gc_next_collection));

    // Grow exponentially
    // auto next_gc = allocator->get_next_gc() * vopts.gc_heap_grow_factor;

    allocator->set_next_gc(next_gc);

    log_indent_level--;
    log(fmt::color::green, "{} [freed: {} bytes, next_gc: {}]", "end", bytes_freed, next_gc);
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
    for (auto value : allocator->get_temp_stash())
    {
        mark_value(value);
    }
    if (Compiler::does_compiler_exist())
    {
        log(fmt::color::yellow, "{}", "mark compiler roots");
        log_indent_level++;
        Compiler::mark_compiler_roots(*this, (vm != nullptr));
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
        mark_object(vm->constant_string_init);
        log_indent_level--;
        log(fmt::color::blue, "{}", "end mark vm roots");
    }
    for (auto obj : never_delete_objects)
    {
        mark_object(obj);
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
    if (object->is_marked)
        return;
    log(fmt::color::white, "mark    {:<20p} {:16} [value: {}]", static_cast<void *>(object),
        object_type_to_string(object->get_type()), Value{object}.to_string());
    object->is_marked = true;
    grey_objects.push_back(object);
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

auto GarbageCollector::mark_table(StringValueTable &table) -> void
{
    for (auto &slot : table.get_slots())
    {
        if (slot.state == StringValueTable::Slot::State::FILLED)
        {
            mark_object(slot.key);
            mark_value(slot.value);
        }
    }
}

auto GarbageCollector::trace_references() -> void
{
    log(fmt::color::purple, "{}", "start trace refs");
    log_indent_level++;
    while (!grey_objects.empty())
    {
        Object *last = grey_objects.back();
        grey_objects.pop_back();
        blacken_object(last);
    }
    log_indent_level--;
    log(fmt::color::purple, "{}", "end trace refs");
}

auto GarbageCollector::blacken_object(Object *object) -> void
{
    log(fmt::color::beige, "blacken {:<20p} {:16} [value: {}]", static_cast<void *>(object),
        object_type_to_string(object->get_type()), Value{object}.to_string());
    switch (object->get_type())
    {
    // strings & native functions do not reference any other object
    case ObjectType::STRING:
    case ObjectType::NATIVE_FUNCTION:
        break;
    // An upvalue can be in closed state (i.e. it holds a value)
    case ObjectType::UPVALUE:
        mark_value(static_cast<ObjectUpvalue *>(object)->closed);
        break;
    // A function holds reference to the name of the function, and an array of constants
    case ObjectType::FUNCTION:
    {
        auto func = static_cast<ObjectFunction *>(object);
        mark_object(func->name());
        for (auto value : func->get()->get_constants())
        {
            mark_value(value);
        }
        break;
    }
    // A class contains a reference to it's name, and a table of methods
    case ObjectType::CLASS:
    {
        auto class_ = static_cast<ObjectClass *>(object);
        mark_object(class_->name());
        mark_table(class_->methods());
        break;
    }

    // An instance contains a reference to it's class and it's field table
    case ObjectType::INSTANCE:
    {
        auto instance = static_cast<ObjectInstance *>(object);
        mark_object(instance->get_class());
        mark_table(instance->get_fields());
        break;
    }

    // A bound method references a value (receiver), and a closure (method)
    case ObjectType::BOUND_METHOD:
    {
        auto bound_method = static_cast<ObjectBoundMethod *>(object);
        mark_value(bound_method->receiver());
        mark_object(bound_method->method());
        break;
    }

    // A closure holds reference to a bare function, and an array of pointers to upvalues
    case ObjectType::CLOSURE:
    {
        auto closure = static_cast<ObjectClosure *>(object);
        // Mark the function held by the closure
        mark_object(closure->get());
        for (auto upvalue : closure->get_upvalues())
        {
            mark_object(upvalue);
        }
        break;
    }

    case ObjectType::LIST:
    {
        auto list = static_cast<ObjectList *>(object);
        // Mark all values stored in the list
        for (auto value : list->get_values())
            mark_value(value);
        break;
    }
    default:
        throw std::logic_error("invalid object type");
        break;
    }
}

auto GarbageCollector::sweep() -> void
{
    int destroyed_object_count = 0;
    log(fmt::color::orange, "{}", "start sweep");
    log_indent_level++;
    std::vector<Object *> &objects = allocator->get_objects();
    // All unmarked objects are considered garbage and are collected
    for (size_t i = 0; i < objects.size();)
    {
        Object *obj = objects[i];
        if (!obj)
        {
            // Remove null objects from the array
            std::swap(objects[i], objects.back());
            objects.pop_back();
            // Do not increment i since the current element also needs processing
        }
        else if (!obj->is_marked)
        {
            // Use swap-erase to remove element efficiently (since order does not matter in object
            // array)
            std::swap(objects[i], objects.back());
            objects.pop_back();

            // Free the object
            allocator->free_object(obj);
            destroyed_object_count++;

            // Do not increment i since the current element also needs processing
        }
        else
        {
            // This object is used, mark the object as white for next cycle
            obj->is_marked = false;
            i++;
        }
    }
    log_indent_level--;
    log(fmt::color::orange, "{} [destroyed {} objects]", "end sweep", destroyed_object_count);
}