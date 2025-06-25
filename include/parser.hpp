#pragma once
#include "error_reporter.hpp"
#include "lexer.hpp"

class Parser
{
    Lexer::const_token_iterator previous_, current, next;
    bool had_error_, panic_mode;
    ErrorReporter &reporter;

  public:
    Parser(Lexer::const_token_iterator iter, ErrorReporter &reporter);

    template <typename... Args> auto report_error(const std::string &message, Args... args) -> void
    {
        if (panic_mode)
            return;
        panic_mode = true;
        reporter.report(ErrorReporter::ERROR, *current, message, args...);
        had_error_ = true;
    }

    template <typename... Args>
    auto report_error_previous(const std::string &message, Args... args) -> void
    {
        if (panic_mode)
            return;
        panic_mode = true;
        reporter.report(ErrorReporter::ERROR, *previous_, message, args...);
        had_error_ = true;
    }

    auto report_error_token(Token token) -> void;

    /**
     * This function returns the last token which was consumed
     * Note: If previous() is called when the parser is at the first token, it returns the first
     * token.
     */
    auto previous() const -> Token;
    /**
     * If the current token is of type `expected`, the parser is advanced. Otherwise an error is
     * reported
     */
    auto consume(TokenType expected, std::string_view error_message = "") -> void;

    /**
     * Returns true if an error had occured while parsing
     */

    auto is_panic() const -> bool { return panic_mode; }

    auto had_error() const -> bool;
    /**
     * Returns true if the parser does not have any more tokens to process and has parsed
     * TokenType::END_OF_FILE
     */
    auto is_at_end() const -> bool;
    /**
     * Returns true if the current token has type `expected`
     */
    auto check(TokenType expected) const -> bool;
    /**
     * Consumes the current token and returns `true` if the current token has type `expected`,
     * otherwise does not consume current and returns false
     */
    auto match(TokenType expected) -> bool;
    /**
     * Advances the parser by consuming the current token
     */
    auto advance() -> void;
    /**
     * Returns the current token without advancing the parser
     */
    auto peek() const -> Token;
    /**
     * Returns the token after current without advancing the parser
     */
    auto peek_next() const -> Token;

    auto synchronize() -> void;
};