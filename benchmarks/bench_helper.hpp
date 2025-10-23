#pragma once
#include "compiler.hpp"
#include "gc.hpp"
#include "vm.hpp"

class LoxRuntime
{
    ErrorReporter reporter;
    GarbageCollector gc;
    Allocator allocator;
    Context context;
    VM vm;

    class nullbuffer : public std::streambuf
    {
      public:
        int overflow(int c) override { return c; }
    };

    class nullostream : public std::ostream
    {
      public:
        nullostream() : std::ostream(&m_sb) {}

      private:
        nullbuffer m_sb;
    };

  public:
    LoxRuntime(VMOpts opts) : gc(opts), allocator(opts), vm(opts, reporter, allocator, &context)
    {
        allocator.set_gc(&gc);
        gc.set_allocator(&allocator);
        gc.set_vm(&vm);
    }

    auto compile(std::string_view src) -> ObjectFunction *
    {
        CompilerOpts opts;
        Lexer lexer(src);
        ErrorReporter reporter;
        Parser parser(lexer.begin(), reporter);
        Compiler compiler(parser, opts, allocator, &context, FunctionType::SCRIPT);

        auto [fn, result] = compiler.compile();
        if (reporter.has_error() || result != InterpretResult::OK)
        {
            throw std::logic_error("syntax error");
        }
        return fn;
    }

    auto execute(ObjectFunction *fn) -> void
    {
        nullostream os;
        auto result = vm.run(fn, os);
        auto has_error = reporter.has_error();
        reporter.clear();
        vm.clear_evaluation_stack();
        vm.clear_frames();
        Compiler::current = nullptr;
        if (has_error || result != InterpretResult::OK)
            throw std::logic_error("runtime error");
    }
};