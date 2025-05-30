#pragma once
#include "lexer.hpp"

class Parser
{
    Lexer::const_token_iterator previous_, current, next;
    bool had_error, panic_mode;

  public:
    Parser(Lexer::const_token_iterator iter)
        : previous_(iter), current(iter), next(iter), had_error(false), panic_mode(false)
    {
        ++next;
    }

    auto peek() const -> Token { return *current; }

    auto peek_next() const -> Token { return *next; }

    auto advance() -> void
    {
        previous_ = current;
        current = next;
        ++next;
    }

    /**
     * Consumes the `current` token if it's type is the expected type and returns true. Otherwise
     * returns false
     */
    auto match(TokenType expected) -> bool
    {
        if ((*current).token_type == expected)
        {
            advance();
            return true;
        }
        return false;
    }

    auto check(TokenType expected) const -> bool { return (*current).token_type == expected; }

    auto is_at_end() const -> bool { return (*current).token_type == TokenType::END_OF_FILE; }

    auto consume(TokenType expected, std::string_view err_message) -> void
    {
        if ((*current).token_type == expected)
            advance();
        // TODO: Report error here
    }

    /**
     * Note: If previous() is called when the parser is at the first token, it returns current
     */
    auto previous() const -> Token { return *previous_; }
};