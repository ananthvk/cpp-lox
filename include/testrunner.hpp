#pragma once
#include "compiler.hpp"
#include "lox.hpp"
#include "vm.hpp"
#include <filesystem>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <string>

struct TestOptions
{
    CompilerOpts compiler_opts;
    VMOpts vm_opts;
    LoxOpts lox_opts;
};

class TestRunner
{
  private:
    std::filesystem::path test_path;
    TestOptions opts;

    auto run_test(const std::filesystem::path &path) -> bool
    {
        // TODO: It cannot test exit() since the whole program exits in that cases, fix that if it's
        // possible
        fmt::print(fmt::fg(fmt::color::white), "TEST {:<50}", path.string());
        std::fflush(stdout);


        try
        {
            std::ifstream file(path);
            if (!file)
            {
                fmt::print(fmt::fg(fmt::color::magenta), " - FILE READ ERROR");
                fmt::println("");
                return false;
            }

            std::stringstream ss;
            ss << file.rdbuf();
            auto source = ss.str();

            ErrorReporter reporter;
            Allocator allocator;
            Context context;

            Lexer lexer(source);
            Parser parser(lexer.begin(), reporter);
            Compiler compiler(parser, opts.compiler_opts, allocator, &context,
                              FunctionType::SCRIPT);

            auto [obj, result] = compiler.compile();

            if (result != InterpretResult::OK)
            {
                fmt::print(fmt::fg(fmt::color::red), " - COMPILE ERROR\n");
                std::fflush(stdout);
                if (reporter.has_messages())
                {
                    reporter.display(stderr);
                }
                fmt::println("");
                return false;
            }

            if (!opts.lox_opts.compile_only)
            {
                VM vm(opts.vm_opts, reporter, allocator, &context);
                std::ostringstream output;
                result = vm.run(obj, output);

                if (result != InterpretResult::OK || reporter.has_error())
                {
                    fmt::print(fmt::fg(fmt::color::red), " - RUNTIME ERROR\n");
                    std::fflush(stdout);
                    if (reporter.has_messages())
                    {
                        reporter.display(stderr);
                    }
                    return false;
                }
            }

            fmt::print(fmt::fg(fmt::color::green), " - PASSED");
            std::fflush(stdout);
        }
        catch (std::exception &e)
        {
            fmt::print(fmt::fg(fmt::color::magenta), " - EXCEPTION: {}", e.what());
            std::fflush(stdout);
            return false;
        }
        fmt::println("");
        return true;
    }

  public:
    explicit TestRunner(const std::filesystem::path &path, TestOptions opts)
        : test_path(path), opts(opts)
    {
    }

    void walk_and_run_tests()
    {
        int total_tests = 0, passed = 0;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(test_path))
        {
            if (entry.is_regular_file() && entry.path().filename().string().ends_with("_test.lox"))
            {
                total_tests++;
                passed += run_test(entry.path());
                std::fflush(stdout);
                std::fflush(stderr);
            }
        }
        fmt::println("=== Test files: {} | Passed: {} | Failed: {} ===", total_tests, passed,
                     total_tests - passed);
        if (passed != total_tests)
        {
            exit(1);
        }
    }
};
