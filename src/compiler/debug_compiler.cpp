#include "debug.hpp"
#include "utils.hpp"

auto print_tokens(const Lexer &lexer) -> void
{
    int previous_line = 0;
    Lexer::const_token_iterator it;

    for (it = lexer.begin(); it != lexer.end(); ++it)
    {
        auto token = *it;
        if (token.line != previous_line)
        {
            fmt::print(fmt::fg(fmt::color::cyan), "{:>5} ", token.line);
        }
        else
        {
            fmt::print("    | ");
        }
        fmt::print(fmt::fg(fmt::color::green), "{:<14} ", token_type_to_string(token.token_type));
        fmt::print(fmt::fg(fmt::color::blue), "'{}'", escape_string(token.lexeme));
        if (token.err != ErrorCode::NO_ERROR)
        {
            fmt::print(fmt::fg(fmt::color::red), "{:>30}\n", error_code_to_string(token.err));
        }
        else
        {
            fmt::println("");
        }
        previous_line = token.line;
    }

    fmt::print(fmt::fg(fmt::color::purple), "{:>5} {:<14} ''\n", previous_line + 1,
               token_type_to_string((*it).token_type));
}