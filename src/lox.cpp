#include "lox.hpp"
#include "allocator.hpp"
#include "compiler.hpp"
#include "file_header.hpp"
#include "debug.hpp"
#include "gc.hpp"
#include "lexer.hpp"
#include "serializer.hpp"
#include "vm.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>

Lox::Lox(const CompilerOpts &compiler_opts, const VMOpts &vm_opts, const LoxOpts &lox_opts)
    : compiler_opts(compiler_opts), vm_opts(vm_opts), lox_opts(lox_opts)
{
    if (lox_opts.dump_bytecode)
    {
        this->compiler_opts.dump_function_bytecode = true;
    }
}

auto Lox::compile_and_execute(std::string_view src, ErrorReporter &reporter, VM &vm,
                              Allocator &allocator, Context *context) -> InterpretResult
{
    Lexer lexer(src);
    if (compiler_opts.debug_print_tokens)
    {
        print_tokens(lexer);
    }
    Parser parser(lexer.begin(), reporter);
    Compiler compiler(parser, compiler_opts, allocator, context, FunctionType::SCRIPT);

    auto [obj, result] = compiler.compile();

    if (result != InterpretResult::OK)
        return result;

    if (lox_opts.dump_bytecode)
        disassemble_chunk(*obj->get(), "program", context);

    if (lox_opts.compile_only)
        return InterpretResult::OK;

    result = vm.run(obj, std::cout);

    return result;
}

auto Lox::run_file(const std::filesystem::path &path) -> int
{

    std::ifstream file(path);
    if (!file)
    {
        fmt::print(fmt::fg(fmt::color::red), "Unable to read \"{}\" Error: {}\n", path.string(),
                   std::strerror(errno));
        return 1;
    }
    if (!std::filesystem::is_regular_file(path))
    {
        fmt::print(fmt::fg(fmt::color::red), "Unable to read \"{}\" Error: Not a file\n",
                   path.string());
        return 1;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    auto source = ss.str();

    ErrorReporter reporter;

    GarbageCollector gc(vm_opts);
    Allocator allocator(vm_opts);
    allocator.set_gc(&gc);
    gc.set_allocator(&allocator);

    Context context;

    VM vm(vm_opts, reporter, allocator, &context);
    gc.set_vm(&vm);
    vm.register_native_functions();

    compile_and_execute(source, reporter, vm, allocator, &context);

    if (reporter.has_messages())
    {
        reporter.display(stderr);
    }
    return 0;
}

auto Lox::compile_file(const std::filesystem::path &source_path,
                       const std::filesystem::path &output_path) -> int
{
    // Similar to run file but do not create the VM
    std::ifstream file(source_path);
    if (!file)
    {
        fmt::print(fmt::fg(fmt::color::red), "Unable to read \"{}\" Error: {}\n",
                   source_path.string(), std::strerror(errno));
        return 1;
    }
    if (!std::filesystem::is_regular_file(source_path))
    {
        fmt::print(fmt::fg(fmt::color::red), "Unable to read \"{}\" Error: Not a file\n",
                   source_path.string());
        return 1;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    auto source = ss.str();

    ErrorReporter reporter;

    GarbageCollector gc(vm_opts);
    Allocator allocator(vm_opts);
    allocator.set_gc(&gc);
    gc.set_allocator(&allocator);

    Context context;

    Lexer lexer(source);
    if (compiler_opts.debug_print_tokens)
    {
        print_tokens(lexer);
    }
    Parser parser(lexer.begin(), reporter);
    Compiler compiler(parser, compiler_opts, allocator, &context, FunctionType::SCRIPT);

    auto [obj, result] = compiler.compile();

    if (result != InterpretResult::OK)
    {
        if (reporter.has_messages())
        {
            reporter.display(stderr);
        }
        return 1;
    }

    if (lox_opts.dump_bytecode)
        disassemble_chunk(*obj->get(), "program", &context);

    Serializer serializer;
    auto serialized = serializer.serialize_program(obj, &context);
    
    FileHeader header_writer;
    header_writer.write(output_path, serialized);
    return 0;
}

auto Lox::run_repl() -> int
{
    std::string line;
    ErrorReporter reporter;
    GarbageCollector gc(vm_opts);
    Allocator allocator(vm_opts);
    allocator.set_gc(&gc);
    gc.set_allocator(&allocator);
    Context context;

    VM vm(vm_opts, reporter, allocator, &context);
    gc.set_vm(&vm);

    vm.register_native_functions();

    while (true)
    {
        fmt::print(fmt::fg(fmt::color::blue), ">>> ");
        if (!std::getline(std::cin, line))
            return 0;
        if (line == "exit")
            break;
        if (line == "")
            continue;
        compile_and_execute(line, reporter, vm, allocator, &context);
        if (reporter.has_messages())
        {
            reporter.display(stderr);
            reporter.clear();
        }
        // Clear the vm evaluation stack (in case of errors while executing inside a scope)
        // Errors occur when an unkown global variable is referenced within a scope, after which the
        // local variables remain on the stack
        vm.clear_evaluation_stack();
        vm.clear_frames();
        Compiler::current = nullptr;
    }
    return 0;
}

/**
 * This function is used for automated testing of the interpreter/compiler
 * Pass the source code of the program as a string argument after "-c"
 */
auto Lox::run_source(std::string_view src) -> int
{
    ErrorReporter reporter;
    GarbageCollector gc(vm_opts);
    Allocator allocator(vm_opts);
    allocator.set_gc(&gc);
    gc.set_allocator(&allocator);
    Context context;

    Lexer lexer(src);
    Parser parser(lexer.begin(), reporter);

    Compiler compiler(parser, compiler_opts, allocator, &context, FunctionType::SCRIPT);

    // The VM should be created before the compiler performs compilation
    // because otherwise, the compiled function gets discarded when
    // the vm gets created since the compiled function is not present in the compiler
    // nor in the stack
    VM vm(vm_opts, reporter, allocator, &context);
    gc.set_vm(&vm);
    auto [obj, result] = compiler.compile();
    // Register the native functions after the compiler finishes compiling so that the global table
    // does not get polluted
    vm.register_native_functions();
    if (result != InterpretResult::OK)
    {
        reporter.display(stderr);
        return 1;
    }

    if (lox_opts.compile_only)
        return 0;


    result = vm.run(obj, std::cout);

    if (reporter.has_error() || result != InterpretResult::OK)
    {
        reporter.display(stderr);
        return 2;
    }
    return 0;
}