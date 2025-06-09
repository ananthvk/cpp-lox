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
            auto aval = a.coerce_real();                                                           \
            auto bval = b.coerce_real();                                                           \
            v.set_value(aval operator bval);                                                       \
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

    return allocator.allocate_string(buffer, len, Allocator::StorageType::TAKE_OWNERSHIP);
}

auto VM::init() -> void
{
    // Reserve a few more elements to ensure that we do not trigger resize of the vector when the
    // stack is full
    stack.reserve(opts.value_stack_max + 4);
    chunk_ = nullptr;
    ip = nullptr;
}

auto VM::execute(std::ostream &os) -> InterpretResult
{
    while (1)
    {
        if (opts.debug_trace_value_stack)
        {
            fmt::print("[ ");
            for (auto elem : stack)
            {
                fmt::print(fmt::fg(fmt::color::beige), "{}, ", elem.to_string());
            }
            fmt::println(" ]");
        }
        if (opts.debug_trace_execution)
        {
            disassemble_instruction(*chunk_, static_cast<int>(ip - chunk_->get_code().data()));
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
            // TODO: For now, pop the value and print it
            os << pop().to_string() << std::endl;
            return InterpretResult::OK;
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
            auto &v = stack.back();
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
        case OpCode::NOT:
            push(Value(pop().is_falsey()));
            break;
        case OpCode::EQUAL:
        {
            auto b = pop();
            auto a = pop();
            if (a.is_number() && b.is_number() && (a.is_real() || b.is_real()))
                push(a.coerce_real() == b.coerce_real());
            else
            {
                push(a == b);
            }
            break;
        }
        case OpCode::GREATER:
            // TODO: Fix this: Inefficient, since it performs a binary op, then pops the value from
            // the stack, then again pushes it as bool
            BINARY_OP(>);
            break;
        case OpCode::LESS:
            BINARY_OP(<);
            break;
        default:
            throw std::logic_error("Invalid instruction");
        }
    }
    return InterpretResult::OK;
}

auto VM::run(const Chunk *chunk, std::ostream &os) -> InterpretResult
{
    if (chunk == nullptr)
    {
        throw std::logic_error("chunk is null");
    }
    this->chunk_ = chunk;
    this->ip = chunk->get_code().data();
    return execute(os);
}
