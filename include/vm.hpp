#pragma once
#include "chunk.hpp"
#include "debug.hpp"
#include "error_reporter.hpp"
#include "result.hpp"

struct VMOpts
{
    /**
     * Max number of entries allowed in the VM's stack, this stack is used to execute bytecode
     * in the VM. It is set to a default of 1024
     */
    int value_stack_max = 1024;

    bool debug_trace_execution = false;

    bool debug_trace_value_stack = false;

    /**
     * Waits for input before executing the next instruction
     */
    bool debug_step_mode_enabled = false;
};

class VM
{
  private:
    std::vector<Value> stack;
    VMOpts opts;
    ErrorReporter &reporter;
    const Chunk *chunk;
    const uint8_t *ip;

    auto push(Value value) -> void
    {
        if (static_cast<int>(stack.size()) == opts.value_stack_max)
        {
            // TODO: Throw a custom exception here
            throw std::runtime_error("Stack overflow");
        }
        stack.push_back(value);
    }

    auto pop() -> Value
    {
        if (stack.size() == 0)
        {
            throw std::runtime_error("Stack underflow");
        }
        Value value = stack.back();
        stack.pop_back();
        return value;
    }

    auto read_byte() -> uint8_t { return *ip++; }

    auto read_constant() -> Value { return chunk->get_value_unchecked(read_byte()); }

    auto read_constant_long() -> Value
    {
        uint32_t constant_index = read_byte();
        constant_index |= static_cast<uint32_t>(static_cast<uint32_t>(read_byte()) << 8);
        constant_index |= static_cast<uint32_t>(static_cast<uint32_t>(read_byte()) << 16);
        return chunk->get_value_unchecked(static_cast<int>(constant_index));
    }

    auto execute() -> InterpretResult;

    auto peek(int offset) const -> Value { return *(stack.end() - offset - 1); }

    template <typename... Args> auto report_error(const std::string &message, Args... args) -> void
    {
        int offset = static_cast<ptrdiff_t>(ip - chunk->get_code().data());
        reporter.report(ErrorReporter::ERROR, chunk->get_line_number(offset), message, args...);
    }

  public:
    VM(const VMOpts &opts, ErrorReporter &reporter)
        : opts(opts), reporter(reporter), chunk(nullptr), ip(nullptr)
    {
        init();
    }

    auto init() -> void;

    auto run(const Chunk *chunk) -> InterpretResult;
};