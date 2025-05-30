#include "lexer.hpp"
#include "lox.hpp"
#include <fmt/color.h>
#include <fmt/format.h>

int main(int argc, char *argv[])
{
    Lox lox;
    if (argc == 1)
        return lox.run_repl();
    if (argc == 2)
        return lox.run_file(argv[1]);
    fmt::print(fmt::fg(fmt::color::red), "Usage: cpplox [filename]\n");
    return 2;
}