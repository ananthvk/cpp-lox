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

auto VM::init() -> void
{
    // Reserve a few more elements to ensure that we do not trigger resize of the vector when the
    // stack is full
    evalstack.reserve(opts.value_stack_max + 4);
    frames.resize(opts.frames_max);
}

auto VM::execute(std::ostream &os) -> InterpretResult
{
    while (1)
    {
        if (opts.debug_trace_value_stack)
        {
            fmt::print("[ ");
            for (auto elem : evalstack)
            {
                fmt::print(fmt::fg(fmt::color::beige), "{}, ", elem.to_string());
            }
            fmt::println(" ]");
        }
        if (opts.debug_trace_execution)
        {
            auto current_chunk = current_frame->function->get();
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
            --frame_count;
            if (frame_count == 0)
            {
                // The main script has finished executing, pop the script from the stack
                pop();
                return InterpretResult::OK;
            }

            // Otherwise, discard all stack values that were used by the function, and continue
            // executing the previous function

            // TODO: Inefficient, implement stack top pointer
            int top = static_cast<int>(current_frame->slots - evalstack.data());
            int n = evalstack.size() - top;
            for (int i = 0; i < n; i++)
                evalstack.pop_back();

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
            auto &v = evalstack.back();
            if (v.is_real())
                v.data.d = -v.as_real();
            else if (v.is_integer())
                v.data.i = -v.as_integer();
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
            if (slot >= evalstack.size())
            {
                report_error("Runtime Error: Local variable does not declared");
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
        default:
            throw std::logic_error("Invalid instruction");
        }
    }
    return InterpretResult::OK;
}

auto VM::call_value(Value callee, int arg_count) -> bool
{
    if (callee.is_function())
    {
        return call(static_cast<ObjectFunction *>(callee.as_object()), arg_count);
    }
    report_error("invalid call to non function, can only call functions and classes");
    return false;
}

auto VM::call(ObjectFunction *function, int arg_count) -> bool
{
    if (arg_count != function->arity())
    {
        report_error("Expected {} arguments to call {}(), got {}", function->arity(),
                     function->name()->get(), arg_count);
        return false;
    }
    if (frame_count == MAX_FRAME_SIZE)
    {
        report_error("Stack overflow: too many frames");
        return false;
    }
    auto new_frame = &frames[frame_count++];
    new_frame->function = function;
    new_frame->ip = function->get()->get_code().data();
    // From the top of the eval stack, skip the args, and -1 so that the function is at slot 0
    new_frame->slots = evalstack.data() + (evalstack.size() - arg_count - 1);
    return true;
}

auto VM::run(ObjectFunction *function, std::ostream &os) -> InterpretResult
{
    if (function == nullptr)
    {
        throw std::logic_error("function is null");
    }
    push(Value{function});
    call(function, 0);
    current_frame = &frames[frame_count - 1];
    return execute(os);
}
