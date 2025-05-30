#include "vm.hpp"
#include "debug.hpp"

// TODO: For now, perform all mathematical operations using double

#define BINARY_OP(operator)                                                                        \
    do                                                                                             \
    {                                                                                              \
        auto b = pop();                                                                            \
        auto a = pop();                                                                            \
        double b_d, a_d;                                                                           \
        if (b.type == Value::NUMBER_INT)                                                           \
            b_d = b.data.i;                                                                        \
        if (b.type == Value::NUMBER_DOUBLE)                                                        \
            b_d = b.data.d;                                                                        \
        if (a.type == Value::NUMBER_INT)                                                           \
            a_d = a.data.i;                                                                        \
        if (a.type == Value::NUMBER_DOUBLE)                                                        \
            a_d = a.data.d;                                                                        \
        Value v;                                                                                   \
        v.data.d = a_d operator b_d;                                                               \
        v.type = Value::NUMBER_DOUBLE;                                                             \
        push(v);                                                                                   \
    } while (false)

auto VM::init() -> void
{
    // Reserve a few more elements to ensure that we do not trigger resize of the vector when the
    // stack is full
    stack.reserve(opts.value_stack_max + 4);
    chunk = nullptr;
    ip = nullptr;
}

auto VM::execute() -> InterpretResult
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
            disassemble_instruction(*chunk, static_cast<int>(ip - chunk->get_code().data()));
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
            fmt::print(fmt::fg(fmt::color::crimson), "{}\n", pop().to_string());
            return InterpretResult::OK;
        case OpCode::LOAD_CONSTANT:
            push(read_constant());
            break;
        case OpCode::LOAD_CONSTANT_LONG:
            push(read_constant_long());
            break;
        case OpCode::NEGATE:
        {
            Value v = pop();
            if (v.type == Value::NUMBER_DOUBLE)
                v.data.d = -v.data.d;
            else if (v.type == Value::NUMBER_INT)
                v.data.i = -v.data.i;
            else
            {
                // TODO: Throw an error here
                throw std::runtime_error("cannot negate a non numeric value");
            }
            push(v);
            break;
        }
        case OpCode::ADD:
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
        default:
            throw std::logic_error("Invalid instruction");
        }
    }
    return InterpretResult::OK;
}

auto VM::run(const Chunk *chunk) -> InterpretResult
{
    if (chunk == nullptr)
    {
        throw std::logic_error("chunk is null");
    }
    this->chunk = chunk;
    this->ip = chunk->get_code().data();
    return execute();
}