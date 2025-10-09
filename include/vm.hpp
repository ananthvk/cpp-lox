#pragma once
#include "allocator.hpp"
#include "chunk.hpp"
#include "config.hpp"
#include "context.hpp"
#include "debug.hpp"
#include "error_reporter.hpp"
#include "function.hpp"
#include "hashmap.hpp"
#include "object.hpp"
#include "result.hpp"
#include <ostream>

struct VMOpts
{
    /**
     * Max number of entries allowed in the VM's stack, this stack is used to execute bytecode
     * in the VM. It is set to a default of 1024
     */
    int value_stack_max = MAX_STACK_EVALUATION_SIZE;

    int frames_max = MAX_FRAME_SIZE;

    bool debug_trace_execution = false;

    bool debug_trace_value_stack = false;

    /**
     * Waits for input before executing the next instruction
     */
    bool debug_step_mode_enabled = false;
};

struct CallFrame
{
    ObjectFunction *function;
    uint8_t *ip;
    Value *slots;
};

struct VMStringValueTableHasher

{
    auto operator()(const ObjectString *str) const -> size_t { return str->hash(); }
};

using VMStringValueTable = HashMap<ObjectString *, Value, VMStringValueTableHasher>;

class VM
{
  private:
    std::vector<Value> evalstack;
    VMOpts opts;
    ErrorReporter &reporter;
    Allocator &allocator;
    Context *context;

    std::vector<CallFrame> frames;
    CallFrame *current_frame;
    int frame_count;

    auto push(Value value) -> void
    {
        if (static_cast<int>(evalstack.size()) == opts.value_stack_max)
        {
            // TODO: Throw a custom exception here
            throw std::runtime_error("Stack overflow");
        }
        evalstack.push_back(value);
    }

    auto pop() -> Value
    {
        if (evalstack.size() == 0)
        {
            throw std::runtime_error("Stack underflow");
        }
        Value value = evalstack.back();
        evalstack.pop_back();
        return value;
    }

    auto read_byte() -> uint8_t { return *current_frame->ip++; }

    auto read_constant() -> Value
    {
        return current_frame->function->get()->get_value_unchecked(read_byte());
    }

    auto read_constant_long() -> Value
    {
        uint16_t constant_index = read_byte();
        constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(read_byte()) << 8);
        return current_frame->function->get()->get_value_unchecked(
            static_cast<int>(constant_index));
    }

    auto read_uint16_le() -> uint16_t
    {
        uint16_t constant_index = read_byte();
        constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(read_byte()) << 8);
        return constant_index;
    }

    auto execute(std::ostream &os) -> InterpretResult;

    auto peek(int offset) const -> Value { return *(evalstack.end() - offset - 1); }

    template <typename... Args> auto report_error(const std::string &message, Args... args) -> void
    {
        // TODO: Add flag to enable/disable stack traces
        // For now just use the error reporter to report it
        int offset =
            static_cast<int>(current_frame->ip - current_frame->function->get()->get_code().data());
        reporter.report(ErrorReporter::ERROR,
                        current_frame->function->get()->get_line_number(offset), message, args...);
        for (int i = (frame_count - 1); i >= 0; i--)
        {
            CallFrame *frame = &frames[i];
            ObjectFunction *func = frame->function;
            int instruction = static_cast<int>(frame->ip - func->get()->get_code().data() - 1);

            std::string_view func_name = func->name()->get();
            if (func_name == "")
            {
                reporter.report(ErrorReporter::ERROR_STACK_TRACE,
                                func->get()->get_line_number(instruction), "in {}", "script");
            }
            else
            {
                reporter.report(ErrorReporter::ERROR_STACK_TRACE,
                                func->get()->get_line_number(instruction), "in {}()", func_name);
            }
        }
    }

    auto call_value(Value callee, int arg_count) -> bool;

    auto call(ObjectFunction *function, int arg_count) -> bool;

  public:
    VM(const VMOpts &opts, ErrorReporter &reporter, Allocator &allocator, Context *context)
        : opts(opts), reporter(reporter), allocator(allocator), context(context),
          current_frame(nullptr), frame_count(0)
    {
        init();
    }

    auto init() -> void;

    auto run(ObjectFunction *function, std::ostream &os) -> InterpretResult;

    auto clear_evaluation_stack() -> void { evalstack.clear(); }

    auto clear_frames() -> void { frame_count = 0; }
};