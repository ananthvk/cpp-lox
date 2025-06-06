#pragma once
#include "lexer.hpp"

class Parser
{
    Lexer::const_token_iterator previous_, current, next;
    bool had_error_, panic_mode;
    ErrorReporter &reporter;

  public:
    Parser(Lexer::const_token_iterator iter, ErrorReporter &reporter)
        : previous_(iter), current(iter), next(iter), had_error_(false), panic_mode(false),
          reporter(reporter)
    {
        if ((*iter).token_type == TokenType::ERROR)
        {
            report_error_token(*iter);
        }
        ++next;
    }

    auto peek() const -> Token { return *current; }

    auto peek_next() const -> Token { return *next; }

    auto advance() -> void
    {
        previous_ = current;
        while (true)
        {
            current = next;
            ++next;

            auto token = *current;
            if (token.token_type != TokenType::ERROR)
                break;

            report_error_token(token);
        }
    }

    auto report_error_token(Token token) -> void
    {
        switch (token.err)
        {
        case ErrorCode::INVALID_DECIMAL_LITERAL:
            report_error("Invalid decimal '{}'", token.lexeme);
            break;
        case ErrorCode::UNRECOGNIZED_CHARACTER:
            report_error("Invaild character '{}'", token.lexeme);
            break;
        case ErrorCode::UNTERMINATED_STRING:
            report_error("Unterminated string '{}'", token.lexeme);
            break;
        default:
            break;
        }
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

    auto consume(TokenType expected, std::string_view error_message = "") -> void
    {
        auto token = *current;
        if (token.token_type == expected)
            advance();
        else
        {
            if (error_message.empty())
            {
                if (token.token_type == TokenType::END_OF_FILE)
                    report_error("Syntax Error: Expected {}, but reached end of input",
                                 token_type_to_string(expected));
                else
                    report_error("Syntax Error: Expected {}, found '{}'",
                                 token_type_to_string(expected), token.lexeme);
            }
            else
            {
                report_error("Syntax Error: {}", error_message);
            }
        }
    }

    template <typename... Args> auto report_error(const std::string &message, Args... args) -> void
    {
        if (panic_mode)
            return;
        panic_mode = true;
        reporter.report(ErrorReporter::ERROR, *current, message, args...);
        had_error_ = true;
    }

    /**
     * Note: If previous() is called when the parser is at the first token, it returns current
     */
    auto previous() const -> Token { return *previous_; }

    auto had_error() const -> bool { return had_error_; }
};