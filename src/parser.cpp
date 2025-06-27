#include "parser.hpp"

Parser::Parser(Lexer::const_token_iterator iter, ErrorReporter &reporter)
    : previous_(iter), current(iter), next(iter), had_error_(false), panic_mode(false),
      reporter(reporter)
{
    if ((*iter).token_type == TokenType::ERROR)
    {
        report_error_token(*iter);
    }
    ++next;
}

auto Parser::peek() const -> Token { return *current; }

auto Parser::peek_next() const -> Token { return *next; }

auto Parser::advance() -> void
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

auto Parser::report_error_token(Token token) -> void
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

auto Parser::match(TokenType expected) -> bool
{
    if ((*current).token_type == expected)
    {
        advance();
        return true;
    }
    return false;
}

auto Parser::check(TokenType expected) const -> bool { return (*current).token_type == expected; }

auto Parser::is_at_end() const -> bool { return (*current).token_type == TokenType::END_OF_FILE; }

auto Parser::consume(TokenType expected, std::string_view error_message) -> void
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

auto Parser::previous() const -> Token { return *previous_; }

auto Parser::had_error() const -> bool { return had_error_; }

auto Parser::synchronize() -> void
{
    // Either look for a semicolon (current is the next token after the semicolon) or look for a
    // statement boundary such as function definition of if
    panic_mode = false;
    auto token = peek();
    while (token.token_type != TokenType::END_OF_FILE)
    {
        if (previous().token_type == TokenType::SEMICOLON)
            return;
        switch (token.token_type)
        {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;

        default:;
        }
        advance();
        token = peek();
    }
}