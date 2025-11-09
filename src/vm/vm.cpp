#include "vm.hpp"
#include "debug.hpp"

// TODO: When both operands are integers, division converts them to double before performing the
// operation. Provide an integer division operator (like //)

#define BINARY_OP(operator)                                                                        \
    do                                                                                             \
    {                                                                                              \
        if (!peek(0).is_number() || !peek(1).is_number())                                          \
        {                                                                                          \
            report_error("Expected operands of '{}' to be numbers", #operator);                    \
            return InterpretResult::RUNTIME_ERROR;                                                 \
        }                                                                                          \
        auto b = pop();                                                                            \
        auto a = pop();                                                                            \
        Value v;                                                                                   \
        if (b.is_integer() && a.is_integer())                                                      \
        {                                                                                          \
            if constexpr ((#operator)[0] == '/')                                                   \
            {                                                                                      \
                auto aval = a.coerce_real();                                                       \
                auto bval = b.coerce_real();                                                       \
                v.set_value(aval operator bval);                                                   \
            }                                                                                      \
            else                                                                                   \
            {                                                                                      \
                auto aval = a.as_integer();                                                        \
                auto bval = b.as_integer();                                                        \
                v.set_value(aval operator bval);                                                   \
            }                                                                                      \
        }                                                                                          \
        else if (b.is_number() && a.is_number())                                                   \
        {                                                                                          \
            auto aval = a.coerce_real();                                                           \
            auto bval = b.coerce_real();                                                           \
            v.set_value(aval operator bval);                                                       \
        }                                                                                          \
        push(v);                                                                                   \
    } while (false)

auto concatenate(Allocator &allocator, ObjectString *a, ObjectString *b) -> ObjectString *
{
    size_t len = a->size() + b->size();
    char *buffer = new char[len + 1];
    memcpy(buffer, a->get().data(), a->size());
    memcpy(buffer + a->size(), b->get().data(), b->size());
    buffer[len] = '\0';

    return allocator.intern_string(buffer, len, Allocator::StorageType::TAKE_OWNERSHIP);
}

auto VM::execute(std::ostream &os) -> InterpretResult
{
    while (1)
    {
        if (opts.debug_trace_value_stack)
        {
            fmt::print("[ ");
            for (auto ptr = evalstack.data(); ptr < stack_top; ++ptr)
            {
                fmt::print(fmt::fg(fmt::color::beige), "{}, ", ptr->to_string());
            }
            fmt::println(" ]");
        }
        // TODO: Fix this
        // if (opts.debug_trace_execution)
        //{
        //     auto current_chunk = current_frame->closure->get()->get();
        //     disassemble_instruction(
        //         *current_chunk,
        //         static_cast<int>(current_frame->ip - current_chunk->get_code().data()), context);
        // }
        if (opts.debug_step_mode_enabled)
        {
            getchar();
        }
        uint8_t instruction = read_byte();
        auto op = static_cast<OpCode>(instruction);
        switch (op)
        {
        case OpCode::RETURN:
        {
            auto result = pop();
            close_upvalues(current_frame->slots);
            --frame_count;
            if (frame_count == 0)
            {
                // The main script has finished executing, pop the script from the stack
                pop();
                return InterpretResult::OK;
            }

            // Otherwise, discard all stack values that were used by the function, and continue
            // executing the previous function
            // current_frame->slots points to the first value used by this frame
            stack_top = current_frame->slots;

            push(result);
            current_frame = &frames[frame_count - 1];
            break;
        }
        case OpCode::LOAD_CONSTANT:
            push(read_constant());
            break;
        case OpCode::LOAD_CONSTANT_LONG:
            push(read_constant_long());
            break;
        case OpCode::NEGATE:
        {
            if (!peek(0).is_number())
            {
                report_error("{}", "Runtime Error: Cannot negate value, it must be a number");
                return InterpretResult::RUNTIME_ERROR;
            }
            /**
             * pop(), then push()
             * BM_RunExpressionNegation       3.02 us         2.99 us       231099
             *
             * modify in place
             * BM_RunExpressionNegation       1.68 us         1.68 us       408390
             * So there is a performance gain when negating in place
             */
            auto loc = (stack_top - 1);
            if (loc->is_real())
                loc->data.d = -loc->as_real();
            else if (loc->is_integer())
                loc->data.i = -loc->as_integer();
            // push(v);
            break;
        }
        case OpCode::ADD:
            if (peek(0).is_list() && peek(1).is_list())
            {
                auto b = static_cast<ObjectList *>(peek(0).as_object());
                auto a = static_cast<ObjectList *>(peek(1).as_object());

                std::vector<Value> concatenated;
                concatenated.reserve(a->size() + b->size());
                concatenated.insert(concatenated.end(), a->get_values().begin(),
                                    a->get_values().end());
                concatenated.insert(concatenated.end(), b->get_values().begin(),
                                    b->get_values().end());

                auto new_list = allocator.new_list(0, 0);
                new_list->set_values(std::move(concatenated));
                pop();
                pop();
                push(new_list);
            }
            else if (peek(0).is_string() && peek(1).is_string())
            {
                auto b = peek(0).as_string();
                auto a = peek(1).as_string();

                auto new_string = concatenate(allocator, a, b);
                pop();
                pop();
                push(new_string);
            }
            else
                BINARY_OP(+);
            break;
        case OpCode::SUBTRACT:
            BINARY_OP(-);
            break;
        case OpCode::DIVIDE:
            BINARY_OP(/);
            break;
        case OpCode::MULTIPLY:
            BINARY_OP(*);
            break;
        case OpCode::FALSE:
            push(Value(false));
            break;
        case OpCode::TRUE:
            push(Value(true));
            break;
        case OpCode::NIL:
            push(Value());
            break;
        case OpCode::UNINITIALIZED:
        {
            auto val = Value{};
            val.set_uninitialized(true);
            push(val);
            break;
        }
        case OpCode::NOT:
            push(Value(pop().is_falsey()));
            break;
        case OpCode::EQUAL:
        {
            auto b = pop();
            auto a = pop();
            if (a.is_number() && b.is_number() && (a.is_real() || b.is_real()))
                // TODO: -Wfloat-equal
                push(a.coerce_real() == b.coerce_real());
            else
            {
                push(a == b);
            }
            break;
        }
        case OpCode::GREATER:
            BINARY_OP(>);
            break;
        case OpCode::LESS:
            BINARY_OP(<);
            break;
        case OpCode::POP_TOP:
            pop();
            break;
        case OpCode::PRINT:
            os << pop().to_string() << "\n";
            break;
        case OpCode::JUMP_IF_FALSE:
        {
            uint16_t offset = read_uint16_le();
            if (peek(0).is_falsey())
                current_frame->ip += offset;
            break;
        }
        case OpCode::JUMP_IF_TRUE:
        {
            uint16_t offset = read_uint16_le();
            if (!(peek(0).is_falsey()))
                current_frame->ip += offset;
            break;
        }
        case OpCode::POP_JUMP_IF_FALSE:
        {
            uint16_t offset = read_uint16_le();
            if (pop().is_falsey())
                current_frame->ip += offset;
            break;
        }
        case OpCode::JUMP_FORWARD:
        {
            uint16_t offset = read_uint16_le();
            current_frame->ip += offset;
            break;
        }
        case OpCode::JUMP_BACKWARD:
        {
            uint16_t offset = read_uint16_le();
            current_frame->ip -= offset;
            break;
        }
        case OpCode::DUP_TOP:
        {
            push(peek(0));
            break;
        }
        case OpCode::DEFINE_GLOBAL:
        {
            auto index = read_uint16_le();
            // We first insert the value, then pop from the stack because if a garbage collection is
            // triggered in the middle of adding it to the table, the VM will not be able to find
            // the value
            auto &global_val = context->get_internal_value(index);

            if (global_val.defined && global_val.is_const)
            {
                report_error("Runtime Error: Cannot redeclare const variable '{}'",
                             context->get_name(index)->get());
                return InterpretResult::RUNTIME_ERROR;
            }

            global_val.defined = true;
            global_val.value = peek(0);
            if (!peek(0).is_uninitialized())
            {
                global_val.initialized = true;
            }
            pop();
            break;
        }
        case OpCode::STORE_GLOBAL:
        {
            // Another difference between STORE_GLOBAL and DEFINE_GLOBAL is that this OPCODE does
            // not consume the value on top of the stack since an assignment expression's value can
            // be used
            auto index = read_uint16_le();
            if (!context->exists(index) || !context->is_defined(index))
            {
                report_error("Runtime Error: Undefined global variable '{}'",
                             context->get_name(index)->get());
                return InterpretResult::RUNTIME_ERROR;
            }
            else if (context->get_internal_value(index).is_const)
            {
                report_error("Runtime Error: Assignment to const variable '{}'",
                             context->get_name(index)->get());
                return InterpretResult::RUNTIME_ERROR;
            }
            else
            {
                context->get_value(index) = peek(0);
                context->set_initialized(index, true);
            }
            break;
        }
        case OpCode::LOAD_GLOBAL:
        {
            auto index = read_uint16_le();
            if (!context->exists(index) || !context->is_defined(index))
            {
                report_error("Runtime Error: Undefined global variable '{}'",
                             context->get_name(index)->get());
                return InterpretResult::RUNTIME_ERROR;
            }
            if (!context->is_initialized(index))
            {
                report_error("Runtime Error: Uninitialized access of global variable '{}'",
                             context->get_name(index)->get());
                return InterpretResult::RUNTIME_ERROR;
            }
            push(context->get_value(index));
            break;
        }
        case OpCode::LOAD_LOCAL:
        {
            auto slot = read_uint16_le();
            if ((evalstack.data() + slot) >= stack_top)
            {
                report_error("Runtime Error: Local variable not declared");
                return InterpretResult::RUNTIME_ERROR;
            }
            if (current_frame->slots[slot].is_uninitialized())
            {
                // TODO: Also report variable name
                report_error("Runtime Error: Uninitialized access of local variable");
                return InterpretResult::RUNTIME_ERROR;
            }
            push(current_frame->slots[slot]);
            break;
        }
        case OpCode::STORE_LOCAL:
        {
            auto slot = read_uint16_le();
            current_frame->slots[slot] = peek(0);
            // Do not pop the value here since assignment is an expression
            break;
        }
        case OpCode::CALL:
        {
            // We need the argument count so that we can peek to get the function being executed
            auto arg_count = read_byte();
            if (!call_value(peek(arg_count), arg_count))
            {
                return InterpretResult::RUNTIME_ERROR;
            }
            // After the call, a new frame is created for the function
            // move the frame pointer to the new function
            current_frame = &frames[frame_count - 1];
            break;
        }
        case OpCode::CLOSURE:
        {
            ObjectFunction *func = static_cast<ObjectFunction *>(read_constant_long().as_object());
            ObjectClosure *closure = allocator.new_closure(func);
            push(Value{closure});
            for (int i = 0; i < closure->upvalue_count(); i++)
            {
                int is_local = read_byte();
                uint16_t index = read_uint16_le();
                if (is_local)
                {
                    closure->get_upvalues()[i] = capture_upvalue(current_frame->slots + index);
                }
                else
                {
                    // The closure references an upvalue of the current function
                    closure->get_upvalues()[i] = current_frame->closure->get_upvalues()[index];
                }
            }
            break;
        }
        case OpCode::LOAD_UPVALUE:
        {
            uint16_t slot = read_uint16_le();
            push(*(current_frame->closure->get_upvalues()[slot]->get()));
            break;
        }
        case OpCode::STORE_UPVALUE:
        {
            uint16_t slot = read_uint16_le();
            auto loc = current_frame->closure->get_upvalues()[slot]->get();
            *loc = peek(0);
            break;
        }
        case OpCode::CLOSE_UPVALUE:
        {
            auto last_value = stack_top - 1;
            close_upvalues(last_value);
            pop();
            break;
        }
        case OpCode::CLASS:
        {
            Value value = read_constant_long();
            if (!value.is_string())
            {
                throw std::logic_error("invalid operand to opcode CLASS, expected a string");
            }
            push(value);
            auto class_ = allocator.new_class(value.as_string());
            pop();
            push(class_);
            break;
        }
        case OpCode::LOAD_PROPERTY:
        {
            if (!peek(0).is_instance())
            {
                report_error("Runtime Error: Can only access properties on instances");
                return InterpretResult::RUNTIME_ERROR;
            }
            auto instance = static_cast<ObjectInstance *>(peek(0).as_object());
            auto value = read_constant_long();
            auto property = static_cast<ObjectString *>(value.as_object());

            auto val = instance->get_fields().get(property);
            if (val)
            {
                // Pop the instance from the stack
                pop();
                push(val.value());
                break;
            }
            else
            {
                // The field does not exist
                // Check if a method with the same name exists

                // In lox, fields shadow methods, so if a method and a field have the same name,
                // fields take the priority

                auto method = instance->get_class()->methods().get(property);
                if (!method)
                {
                    report_error("Runtime Error: Instance of class '{}' has no property '{}'",
                                 instance->get_class()->name()->get(), property->get());
                    return InterpretResult::RUNTIME_ERROR;
                }

                // A method with the requested name was found, bind the method (closure) to this
                // instance and push it onto the stack
                auto bound_method = allocator.new_bound_method(
                    peek(0), static_cast<ObjectClosure *>(method.value().as_object()));

                pop(); // instance
                push(bound_method);
            }
            break;
        }
        case OpCode::INVOKE:
        {
            auto value = read_constant_long();
            auto method_name = static_cast<ObjectString *>(value.as_object());
            auto arg_count = read_byte();
            if (!invoke(method_name, arg_count))
            {
                return InterpretResult::RUNTIME_ERROR;
            }
            // After the call, a new frame is created for the function
            current_frame = &frames[frame_count - 1];
            break;
        }
        case OpCode::LOAD_PROPERTY_SAFE:
        {
            // TODO: Implement short circuiting, i.e. if the base is nil, there is no point
            // evaluating further LOAD_PROPERTY_SAFE instructions, implement it in the compiler

            // If the target of the load opcode is not an instance, push nil
            if (!peek(0).is_instance())
            {
                // Read and discard the operand of the opcode
                read_constant_long();

                pop(); // Pop the instance from the stack
                push(Value{});
                break;
            }

            auto instance = static_cast<ObjectInstance *>(peek(0).as_object());
            auto property = static_cast<ObjectString *>(read_constant_long().as_object());
            auto val = instance->get_fields().get(property);
            if (val)
            {
                pop(); // the instance
                push(val.value());
                break;
            }
            // Check if a method with that name exists
            auto method = instance->get_class()->methods().get(property);
            if (method)
            {
                auto bound_method = allocator.new_bound_method(
                    peek(0), static_cast<ObjectClosure *>(method.value().as_object()));

                pop(); // the instance
                push(bound_method);
                break;
            }
            pop();
            push(Value{});
            break;
        }
        case OpCode::STORE_PROPERTY:
        {
            // The top of the stack contains the value to be set
            // while the second element from the top contains the instance
            if (!peek(1).is_object())
            {
                report_error("Runtime Error: Can only set properties on objects");
                return InterpretResult::RUNTIME_ERROR;
            }
            if (!peek(1).is_instance())
            {
                report_error("Runtime Error: Can only set properties on instances");
                return InterpretResult::RUNTIME_ERROR;
            }

            auto instance = static_cast<ObjectInstance *>(peek(1).as_object());
            auto value = read_constant_long();
            auto property = static_cast<ObjectString *>(value.as_object());

            // Set the value in the hash table, creating the key if it does not exist
            instance->get_fields().get_ref(property) = peek(0);

            // We need to do this because STORE_PROPERTY is an expression, so we remove the second
            // element from the top of the stack, then push back the top element of the stack so
            // that it can be used in other expresssions

            auto top_stack_val = pop(); // Pop the value of the expression
            pop();                      // Pop the instance
            push(top_stack_val);
            break;
        }
        case OpCode::METHOD:
        {
            Value value_string = read_constant_long();
            if (!value_string.is_string())
            {
                throw std::logic_error("invalid operand to opcode METHOD, expected a string");
            }
            auto name = static_cast<ObjectString *>(value_string.as_object());
            // The top of the stack will contain the closure for the method, and the slot below it
            // will be the class
            auto method = peek(0);
            ObjectClass *class_ = static_cast<ObjectClass *>(peek(1).as_object());
            class_->methods().insert(name, method);
            if (name == constant_string_init)
                class_->get_init_method() = method;
            pop(); // Pop the closure
            break;
        }
        case OpCode::INHERIT:
        {
            if (!peek(1).is_class())
            {
                report_error("Runtime Error: Cannot inherit from a non-class value");
                return InterpretResult::RUNTIME_ERROR;
            }
            auto superclass = static_cast<ObjectClass *>(peek(1).as_object());
            auto subclass = static_cast<ObjectClass *>(peek(0).as_object());
            auto &subclass_methods = subclass->methods();

            // Copy over all methods from the superclass into the subclass
            for (auto slot : superclass->methods().get_slots())
            {
                if (slot.state == StringValueTable::Slot::State::FILLED)
                {
                    subclass_methods.insert(slot.key, slot.value);
                }
            }
            pop(); // Remove the subclass from the stack
            break;
        }
        case OpCode::LOAD_SUPER:
        {
            // Since this is generated by the compiler, assume that it is safe
            Value value = read_constant_long();
            auto method_name = value.as_string();
            ObjectClass *superclass = static_cast<ObjectClass *>(pop().as_object());
            ObjectInstance *instance = static_cast<ObjectInstance *>(peek(0).as_object());

            // In Lox, inheritance does not inherit fields, it only inherits methods
            // Get the method from the superclass instead of the instance's own class
            auto method = superclass->methods().get(method_name);
            if (!method)
            {
                report_error("Runtime Error: Instance of class '{}', which is a subclass of class "
                             "'{}' has no method '{}'",
                             instance->get_class()->name()->get(), superclass->name()->get(),
                             method_name->get());
                return InterpretResult::RUNTIME_ERROR;
            }
            auto bound_method = allocator.new_bound_method(
                peek(0), static_cast<ObjectClosure *>(method.value().as_object()));

            pop(); // instance
            push(bound_method);
            break;
        }
        case OpCode::SUPER_INVOKE:
        {
            auto value = read_constant_long();
            auto method_name = static_cast<ObjectString *>(value.as_object());
            auto arg_count = read_byte();
            auto super_class = static_cast<ObjectClass *>(pop().as_object());
            if (!invoke_from_class(super_class, method_name, arg_count))
            {
                return InterpretResult::RUNTIME_ERROR;
            }
            // After the call, a new frame is created for the function
            current_frame = &frames[frame_count - 1];
            break;
        }
        case OpCode::ZERO:
        {
            push(Value{0});
            break;
        }
        case OpCode::MINUS_ONE:
        {
            push(Value{-1});
            break;
        }
        case OpCode::ONE:
        {
            push(Value{1});
            break;
        }
        case OpCode::LIST:
        {
            int list_length = read_byte();
            auto list_obj = allocator.new_list(list_length, list_length);
            for (int i = (list_length - 1); i >= 0; i--)
            {
                list_obj->set(i, peek(0));
                // It's safe to pop off the elements here since append does not call any allocator
                // methods so a garbage collection will not be triggered
                pop();
            }
            push(list_obj);
            break;
        }
        case OpCode::LIST_APPEND:
        {
            // The top element of the stack contains the value to be appended
            // and the value below the top contains the list object
            // This instruction consumes the value to be appended, and leaves the list object on the
            // top
            // Note: This instruction is only generated by the compiler, so no checks
            auto list_obj = static_cast<ObjectList *>(peek(1).as_object());
            list_obj->append(peek(0));
            pop();
            break;
        }
        case OpCode::STORE_INDEX:
        {
            if (peek(2).is_string())
            {
                report_error(
                    "Runtime error: Strings are immutable, and do not support index assignment");
                return InterpretResult::RUNTIME_ERROR;
            }
            if (!peek(2).is_list())
            {
                report_error("Runtime error: Cannot set index of value, can only index lists");
                return InterpretResult::RUNTIME_ERROR;
            }
            if (!peek(1).is_integer())
            {
                report_error("Runtime error: Index of a list must be an integer");
                return InterpretResult::RUNTIME_ERROR;
            }
            auto index = peek(1).as_integer();
            auto list_obj = static_cast<ObjectList *>(peek(2).as_object());
            if (index >= list_obj->size())
            {
                report_error("Runtime error: List index out of range, attempt to subscript list at "
                             "[{}] (list contains {} elements)",
                             index, list_obj->size());
                return InterpretResult::RUNTIME_ERROR;
            }
            Value val = peek(0);
            list_obj->set(index, val);
            pop(); // value
            pop(); // index
            pop(); // list
            // Push back the value on the top of the stack so that it can be used in expressions
            push(val);
            break;
        }
        case OpCode::LOAD_INDEX:
        {
            if (!(peek(1).is_list() || peek(1).is_string()))
            {
                report_error(
                    "Runtime error: Cannot get index of value, can only index lists and strings");
                return InterpretResult::RUNTIME_ERROR;
            }
            if (!peek(0).is_integer())
            {
                report_error("Runtime error: Index of a list or string must be an integer");
                return InterpretResult::RUNTIME_ERROR;
            }
            auto index = peek(0).as_integer();
            if (peek(1).is_list())
            {
                auto list_obj = static_cast<ObjectList *>(peek(1).as_object());
                if (index >= list_obj->size())
                {
                    report_error(
                        "Runtime error: List index out of range, attempt to subscript list at "
                        "[{}] (list contains {} elements)",
                        index, list_obj->size());
                    return InterpretResult::RUNTIME_ERROR;
                }
                Value value = list_obj->get(index).value();
                pop(); // index
                pop(); // list
                push(value);
            }
            else
            {
                // Is a string
                auto string_obj = static_cast<ObjectString *>(peek(1).as_object());
                if (index >= string_obj->get().size())
                {
                    report_error(
                        "Runtime error: String index out of range, attempt to subscript string at "
                        "[{}] (string contains {} characters)",
                        index, string_obj->get().size());
                    return InterpretResult::RUNTIME_ERROR;
                }
                char ch = string_obj->get()[index];
                pop(); // index
                pop(); // list

                // TODO: Optimize this: Wasteful since a new string has to be interned, along with
                // std::string for a single character
                push(allocator.intern_string(std::string(1, ch)));
            }
            break;
        }
        case OpCode::MAP:
        {
            int map_length = read_byte();
            auto map_obj = allocator.new_map();
            for (int i = 0; i < map_length; i++)
            {
                // Read a key & a value from the top of the stack, the key is below the value
                // Then pop them off the stack
                if (!map_obj->set(peek(1), peek(0)))
                {
                    report_error("unhashable type used as map key");
                    return InterpretResult::RUNTIME_ERROR;
                }
                pop();
                pop();
            }
            push(map_obj);
            break;
        }
        case OpCode::MAP_ADD:
        {
            // The top element of the stack contains the value to be set
            // The value below that contains the key
            // and the second element below contains the map object
            // This instruction consumes the value & key to be added, and leaves the map object on
            // the top Note: This instruction is only generated by the compiler, so no checks
            auto map_obj = static_cast<ObjectMap *>(peek(2).as_object());
            if (!map_obj->set(peek(1), peek(0)))
            {
                report_error("unhashable type used as map key");
                return InterpretResult::RUNTIME_ERROR;
            }
            pop();
            pop();
            break;
        }
        default:
            throw std::logic_error("Invalid instruction");
        }
    }
    return InterpretResult::OK;
}

auto VM::close_upvalues(Value *last_value) -> void
{
    while (open_upvalues != nullptr && open_upvalues->get() >= last_value)
    {
        auto current = open_upvalues;
        // Copy from the stack slot to a value inside the upvalue
        current->closed = *current->get();
        // Set the location to variable within the upvalue
        current->set(&current->closed);
        open_upvalues = current->next;
    }
}

auto VM::capture_upvalue(Value *slot) -> ObjectUpvalue *
{
    // open_upvalues point to the upvalue which points to a location closest to the top of the stack

    ObjectUpvalue *current = open_upvalues;
    ObjectUpvalue *prev = nullptr;

    // Check if we already have a captured upvalue for this particular stack slot
    // Since the values are stored in a vector, they are stored in contiguous memory locations
    while (current != nullptr && current->get() > slot)
    {
        prev = current;
        current = current->next;
    }

    // We exit the loop due to three cases:
    // Case 1: The current location is equal to the slot, i.e. we found an existing upvalue
    // Case 2: current is nullptr, i.e. either the open upvalue list is empty, or all open upvalues
    // point to locations above the slot
    // Case 3: current points to an upvalue with location below `slot`, so we need to insert the
    // upvalue before it

    if (current != nullptr && current->get() == slot)
    {
        return current;
    }

    auto new_upvalue = allocator.new_upvalue(slot);
    new_upvalue->next = current;

    if (prev == nullptr)
    {
        // This is the first node in the linked list
        open_upvalues = new_upvalue;
    }
    else
    {
        prev->next = new_upvalue;
    }

    return new_upvalue;
}

auto VM::invoke(ObjectString *method_name, int arg_count) -> bool
{
    Value receiver = peek(arg_count);
    if (!receiver.is_instance())
    {
        report_error("Runtime Error: Can only invoke methods on instances");
        return false;
    }
    // Check whether a field of the same name exists on the instance
    auto instance = static_cast<ObjectInstance *>(receiver.as_object());
    auto val = instance->get_fields().get(method_name);
    if (val)
    {
        // If a field of the same name exists, instead of invoking a method,
        // Replace the receiver with the value of the property
        // Then call the value normally
        Value value = val.value();
        *(stack_top - arg_count - 1) = value;
        return call_value(value, arg_count);
    }

    return invoke_from_class(instance->get_class(), method_name, arg_count);
}

auto VM::invoke_from_class(ObjectClass *class_, ObjectString *property_name, int arg_count) -> bool
{
    auto method = class_->methods().get(property_name);
    if (!method)
    {
        report_error("Runtime Error: Instance of class '{}' has no method '{}'",
                     class_->name()->get(), property_name->get());
        return false;
    }
    return call(static_cast<ObjectClosure *>(method.value().as_object()), arg_count);
}

auto VM::call_value(Value callee, int arg_count) -> bool
{
    if (callee.is_function())
    {
        throw std::logic_error("naked functions should not exist");
    }
    if (callee.is_closure())
    {
        return call(static_cast<ObjectClosure *>(callee.as_object()), arg_count);
    }
    if (callee.is_bound_method())
    {
        ObjectBoundMethod *bound_method = static_cast<ObjectBoundMethod *>(callee.as_object());
        // If we are calling a bound method, modify the stack so that slot 0 of the new call frame
        // holds the receiver instead of the function being called
        *(stack_top - arg_count - 1) = bound_method->receiver();
        return call(bound_method->method(), arg_count);
    }
    if (callee.is_class())
    {
        // The language does not have a new keyword, so when a class is "called", an instance of
        // that class is created
        ObjectClass *class_ = static_cast<ObjectClass *>(callee.as_object());
        // At stack_top - arg_count - 1 location, the class object is present. We replace that
        // object with an instance of the class.
        // For now, we are going to ignore additional arguments passed (initializers)
        *(stack_top - arg_count - 1) = Value{allocator.new_instance(class_)};

        auto initializer = class_->get_init_method();
        // Call the initializer (if an init() method is defined)
        if (!initializer.is_nil())
        {
            return call(static_cast<ObjectClosure *>(initializer.as_object()), arg_count);
        }

        // If there is no initializer, but args were passed, it's an error
        if (arg_count != 0)
        {
            report_error("Expected 0 arguments to call {}(), got {}", class_->name()->get(),
                         arg_count);
            return false;
        }

        return true;
    }
    else if (callee.is_native_function())
    {
        ObjectNativeFunction *func = static_cast<ObjectNativeFunction *>(callee.as_object());
        // If the arity is set to -1, do not check for number of arguments
        if (func->arity() != -1 && arg_count != func->arity())
        {
            report_error("Expected {} arguments to call native function, got {}", func->arity(),
                         arg_count);
            return false;
        }
        NativeFunction native_func = func->get();
        // One is subtracted so that the pointer points to the function on the stack
        // i.e. the arguments start from args[1] .... while args[0] contains the function object
        // itself
        auto [result, ok] = native_func(this, arg_count, stack_top - arg_count - 1);
        if (!ok)
        {
            return false;
        }
        stack_top -= arg_count + 1;
        push(result);
        return true;
    }
    report_error("invalid call to non function, can only call functions and classes");
    return false;
}

auto VM::call(ObjectClosure *closure, int arg_count) -> bool
{
    if (arg_count != closure->get()->arity())
    {
        report_error("Expected {} arguments to call {}(), got {}", closure->get()->arity(),
                     closure->get()->name()->get(), arg_count);
        return false;
    }
    if (frame_count == MAX_FRAME_SIZE)
    {
        report_error("Stack overflow: too many frames");
        return false;
    }
    auto new_frame = &frames[frame_count++];
    new_frame->closure = closure;
    new_frame->ip = closure->get()->get()->get_code().data();
    // From the top of the eval stack, skip the args, and -1 so that the function is at slot 0
    new_frame->slots = stack_top - arg_count - 1;
    return true;
}

auto VM::define_native_function(std::string_view name, int arity, NativeFunction func) -> void
{
    push(Value{allocator.intern_string(name)});
    push(Value{allocator.new_native_function(arity, func)});

    int index = context->get_global(peek(1).as_string());
    context->get_value(index) = peek(0);
    context->set_initialized(index, true);
    context->set_defined(index, true);
    context->get_internal_value(index).is_created_by_runtime = true;

    pop();
    pop();
}

auto VM::run(ObjectFunction *function, std::ostream &os) -> InterpretResult
{
    output_stream = &os;
    if (function == nullptr)
    {
        throw std::logic_error("function is null");
    }
    push(Value{function});
    auto closure = allocator.new_closure(function);
    pop();

    push(Value{closure});
    call(closure, 0);
    current_frame = &frames[frame_count - 1];
    return execute(os);
}
