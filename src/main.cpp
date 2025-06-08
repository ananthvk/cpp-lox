#include "lexer.hpp"
#include "lox.hpp"
#include <fmt/color.h>
#include <fmt/format.h>

int main(int argc, char *argv[])
{
    Lox lox;
    int status;
    if (argc == 1)
        status = lox.run_repl();
    else if (argc == 2)
        status = lox.run_file(argv[1]);
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "Usage: cpplox [filename]\n");
        status = 2;
    }
    return status;
}