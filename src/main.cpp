#include "lexer.hpp"
#include "lox.hpp"
#include <fmt/color.h>
#include <fmt/format.h>

int main(int argc, char *argv[])
{
    int status = 0;
    try
    {
        Lox lox;
        if (argc == 1)
            status = lox.run_repl();
        else if (argc == 2)
        {
            if (argv[1] == std::string("-c"))
            {
                fmt::print(fmt::fg(fmt::color::red), "Usage: cpplox -c \"<source code>\"\n");
                status = 3;
            }
            else
            {
                status = lox.run_file(argv[1]);
            }
        }
        else if (argc == 3)
        {
            if (argv[1] == std::string("-c"))
            {
                status = lox.run_source(argv[2]);
            }
            else
            {
                fmt::print(fmt::fg(fmt::color::red), "Usage: cpplox -c \"<source code>\"\n");
                status = 3;
            }
        }
        else
        {
            fmt::print(fmt::fg(fmt::color::red), "Usage: cpplox [-c] [filename]\n");
            status = 3;
        }
    }
    catch (std::exception &e)
    {
        fmt::print(fmt::fg(fmt::color::red), "System Error: {}\n", e.what());
        status = 4;
    }
    return status;
}