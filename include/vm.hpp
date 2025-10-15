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
    ObjectClosure *closure;
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
    std::ostream *output_stream;

    Value *stack_top;

    auto push(Value value) -> void
    {
        // The stack is full
        if (stack_top == (evalstack.data() + evalstack.size()))
        {
            throw std::runtime_error("Stack overflow");
        }
        *stack_top++ = value;
    }

    auto pop() -> Value
    {
        if (stack_top == evalstack.data())
        {
            throw std::runtime_error("Stack underflow");
        }
        return *(--stack_top);
    }

    auto read_byte() -> uint8_t { return *current_frame->ip++; }

    auto read_constant() -> Value
    {
        return current_frame->closure->get()->get()->get_value_unchecked(read_byte());
    }

    auto read_constant_long() -> Value
    {
        uint16_t constant_index = read_byte();
        constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(read_byte()) << 8);
        return current_frame->closure->get()->get()->get_value_unchecked(
            static_cast<int>(constant_index));
    }

    auto read_uint16_le() -> uint16_t
    {
        uint16_t constant_index = read_byte();
        constant_index |= static_cast<uint16_t>(static_cast<uint16_t>(read_byte()) << 8);
        return constant_index;
    }

    auto execute(std::ostream &os) -> InterpretResult;

    auto peek(int offset) const -> Value
    {
        auto location = stack_top - offset - 1;
        if (location < evalstack.data())
        {
            throw std::runtime_error("Stack underflow while peek()");
        }
        return *location;
    }

    auto call_value(Value callee, int arg_count) -> bool;

    auto call(ObjectClosure *closure, int arg_count) -> bool;
    
    auto capture_upvalue(Value *slot) -> ObjectUpvalue*;

  public:
    VM(const VMOpts &opts, ErrorReporter &reporter, Allocator &allocator, Context *context)
        : opts(opts), reporter(reporter), allocator(allocator), context(context),
          current_frame(nullptr), frame_count(0)
    {
        init();
    }

    auto init() -> void;

    auto run(ObjectFunction *function, std::ostream &os) -> InterpretResult;

    auto clear_evaluation_stack() -> void { stack_top = evalstack.data(); }

    auto clear_frames() -> void { frame_count = 0; }

    auto get_allocator() -> Allocator * { return &allocator; }

    auto define_native_function(std::string_view name, int arity, NativeFunction func) -> void;

    auto register_native_functions() -> void;

    template <typename... Args> auto report_error(const std::string &message, Args... args) -> void
    {
        // TODO: Add flag to enable/disable stack traces
        // For now just use the error reporter to report it
        for (int i = (frame_count - 1); i >= 0; i--)
        {
            CallFrame *frame = &frames[i];
            ObjectFunction *func = frame->closure->get();
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
        int offset = static_cast<int>(current_frame->ip -
                                      current_frame->closure->get()->get()->get_code().data());
        reporter.report(ErrorReporter::ERROR,
                        current_frame->closure->get()->get()->get_line_number(offset), message,
                        args...);
    }

    auto get_output_stream() -> std::ostream & { return *output_stream; }
};