#pragma once
#include "allocator.hpp"
#include "chunk.hpp"
#include "config.hpp"
#include "context.hpp"
#include "debug.hpp"
#include "error_reporter.hpp"
#include "hashmap.hpp"
#include "result.hpp"
#include <ostream>

struct VMOpts
{
    /**
     * Max number of entries allowed in the VM's stack, this stack is used to execute bytecode
     * in the VM. It is set to a default of 1024
     */
    int value_stack_max = MAX_STACK_EVALUATION_SIZE;

    bool debug_trace_execution = false;

    bool debug_trace_value_stack = false;

    /**
     * Waits for input before executing the next instruction
     */
    bool debug_step_mode_enabled = false;
};

struct VMStringValueTableHasher
{
    auto operator()(const ObjectString *str) const -> size_t { return str->hash(); }
};

using VMStringValueTable = HashMap<ObjectString *, Value, VMStringValueTableHasher>;

class VM
{
  private:
    std::vector<Value> stack;
    VMOpts opts;
    ErrorReporter &reporter;
    Allocator &allocator;
    const Chunk *chunk_;
    const uint8_t *ip;
    Context *context;

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

    auto read_constant() -> Value { return chunk_->get_value_unchecked(read_byte()); }

    auto read_constant_long() -> Value
    {
        uint16_t constant_index = read_byte();
        constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(read_byte()) << 8);
        return chunk_->get_value_unchecked(static_cast<int>(constant_index));
    }

    auto read_uint16_le() -> uint16_t
    {
        uint16_t constant_index = read_byte();
        constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(read_byte()) << 8);
        return constant_index;
    }

    auto execute(std::ostream &os) -> InterpretResult;

    auto peek(int offset) const -> Value { return *(stack.end() - offset - 1); }

    template <typename... Args> auto report_error(const std::string &message, Args... args) -> void
    {
        int offset = static_cast<int>(ip - chunk_->get_code().data());
        reporter.report(ErrorReporter::ERROR, chunk_->get_line_number(offset), message, args...);
    }

  public:
    VM(const VMOpts &opts, ErrorReporter &reporter, Allocator &allocator, Context *context)
        : opts(opts), reporter(reporter), allocator(allocator), chunk_(nullptr), ip(nullptr),
          context(context)
    {
        init();
    }

    auto init() -> void;

    auto run(const Chunk *chunk, std::ostream &os) -> InterpretResult;

    auto clear_evaluation_stack() -> void { stack.clear(); }
};