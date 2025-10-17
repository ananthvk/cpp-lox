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
        if (opts.debug_trace_execution)
        {
            auto current_chunk = current_frame->closure->get()->get();
            disassemble_instruction(
                *current_chunk,
                static_cast<int>(current_frame->ip - current_chunk->get_code().data()), context);
        }
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
            if (peek(0).is_string() && peek(1).is_string())
            {
                auto b = pop().as_string();
                auto a = pop().as_string();
                push(concatenate(allocator, a, b));
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

auto VM::call_value(Value callee, int arg_count) -> bool
{
    ;
    if (callee.is_function())
    {
        throw std::logic_error("naked functions should not exist");
    }
    if (callee.is_closure())
    {
        return call(static_cast<ObjectClosure *>(callee.as_object()), arg_count);
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
