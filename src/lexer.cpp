#include "lexer.hpp"

Lexer::const_token_iterator::const_token_iterator(std::string_view source, size_t start,
                                                  size_t current, int line)
    : source(source), start(start), current(current), line(line),
      current_token_type(TokenType::END_OF_FILE), err(ErrorCode::NO_ERROR)
{
}

auto Lexer::const_token_iterator::advance() -> char
{
    char ch = source[current];
    ++current;
    return ch;
}

auto Lexer::const_token_iterator::match(char ch) -> bool
{
    if (is_at_end())
        return false;
    if (source[current] != ch)
        return false;
    advance();
    return true;
}

auto Lexer::const_token_iterator::is_at_end() const -> bool { return current >= source.size(); }

auto Lexer::const_token_iterator::peek() const -> char
{
    if (current < source.size())
        return source[current];
    return '\0';
}

auto Lexer::const_token_iterator::peek_next() const -> char
{
    if ((current + 1) < source.size())
        return source[current + 1];
    return '\0';
}

auto Lexer::const_token_iterator::skip_whitespace() -> void
{
    while (true)
    {
        char ch = peek();
        switch (ch)
        {
        case ' ':
        case '\t':
        case '\r':
            advance();
            break;
        case '\n':
            ++line;
            advance();
            break;
        case '/':
            if (peek_next() == '/')
            {
                while (!is_at_end() && peek() != '\n')
                    advance();
            }
            else
            {
                return;
            }
            break;
        default:
            return;
        }
    }
}

auto Lexer::const_token_iterator::match_string() -> TokenType
{
    while (peek() != '"' && !is_at_end())
    {
        if (peek() == '\n')
            ++line;
        advance();
    }

    if (is_at_end())
    {
        err = ErrorCode::UNTERMINATED_STRING;
        return TokenType::ERROR;
    }

    advance();
    return TokenType::STRING;
}

auto Lexer::const_token_iterator::match_number() -> TokenType
{
    bool is_int = true, is_err = false;
    while (isdigit(peek()))
        advance();
    // Check for a decimal point
    if (peek() == '.')
    {
        is_int = false;
        // Consume the decimal point
        // Note: This also supports numbers like 3.
        advance();

        while (isdigit(peek()))
            advance();
    }

    // Check if some other character is at the end of the number

    char ch = peek();
    while (isalnum(ch) || ch == '_')
    {
        advance();
        ch = peek();
        is_err = true;
    }

    if (is_err)
    {
        err = ErrorCode::INVALID_DECIMAL_LITERAL;
        return TokenType::ERROR;
    }


    return is_int ? TokenType::NUMBER_INT : TokenType::NUMBER_REAL;
}

auto Lexer::const_token_iterator::lexeme_length() const -> size_t { return current - start; }

auto Lexer::const_token_iterator::check_keyword(int idx, int length, const char *rest,
                                                TokenType type) const -> TokenType
{
    if ((current - start == static_cast<size_t>(idx + length)) &&
        source.substr(start + idx, length) == rest)
        return type;
    return TokenType::IDENTIFIER;
}

auto Lexer::const_token_iterator::get_identifier_type() const -> TokenType
{
    switch (source[start])
    {
    case 'a':
        return check_keyword(1, 2, "nd", TokenType::AND);
    case 'c':
        if (lexeme_length() > 1)
        {
            switch (source[start + 1])
            {
            case 'a':
                return check_keyword(2, 2, "se", TokenType::CASE);
            case 'l':
                return check_keyword(2, 3, "ass", TokenType::CLASS);
            case 'o':
                if (lexeme_length() > 3 && source[start + 2] == 'n')
                {
                    switch (source[start + 3])
                    {
                    case 's':
                        return check_keyword(4, 1, "t", TokenType::CONST);
                    case 't':
                        return check_keyword(4, 4, "inue", TokenType::CONTINUE);
                    }
                }
            }
        }
        break;
    case 'b':
        return check_keyword(1, 4, "reak", TokenType::BREAK);
    case 'd':
        return check_keyword(1, 6, "efault", TokenType::DEFAULT);
    case 'e':
        return check_keyword(1, 3, "lse", TokenType::ELSE);
    case 'f':
        if (lexeme_length() > 1)
        {
            switch (source[start + 1])
            {
            case 'a':
                return check_keyword(2, 3, "lse", TokenType::FALSE);
            case 'o':
                return check_keyword(2, 1, "r", TokenType::FOR);
            case 'u':
                return check_keyword(2, 1, "n", TokenType::FUN);
            }
        }
        break;
    case 'i':
        return check_keyword(1, 1, "f", TokenType::IF);
    case 'n':
        if (lexeme_length() > 1)
        {
            switch (source[start + 1])
            {
            case 'i':
                return check_keyword(2, 1, "l", TokenType::NIL);
            case 'o':
                return check_keyword(2, 1, "t", TokenType::NOT);
            }
        }
        break;
    case 'o':
        return check_keyword(1, 1, "r", TokenType::OR);
    case 'p':
        return check_keyword(1, 4, "rint", TokenType::PRINT);
    case 'r':
        return check_keyword(1, 5, "eturn", TokenType::RETURN);
    case 's':
        if (lexeme_length() > 1)
        {
            switch (source[start + 1])
            {
            case 'w':
                return check_keyword(2, 4, "itch", TokenType::SWITCH);
            case 'u':
                return check_keyword(2, 3, "per", TokenType::SUPER);
            }
        }
        break;
    case 't':
        if (lexeme_length() > 1)
        {
            switch (source[start + 1])
            {
            case 'h':
                return check_keyword(2, 2, "is", TokenType::THIS);
            case 'r':
                return check_keyword(2, 2, "ue", TokenType::TRUE);
            }
        }
        break;
    case 'v':
        return check_keyword(1, 2, "ar", TokenType::VAR);
    case 'w':
        return check_keyword(1, 4, "hile", TokenType::WHILE);
    }
    return TokenType::IDENTIFIER;
}

auto Lexer::const_token_iterator::match_identifier() -> TokenType
{
    char ch = peek();
    while (isalnum(ch) || ch == '_')
    {
        advance();
        ch = peek();
    }
    return get_identifier_type();
}

auto Lexer::const_token_iterator::scan_token() -> TokenType
{
    skip_whitespace();
    start = current;

    if (is_at_end())
        return TokenType::END_OF_FILE;

    /*
     * Process single character tokens
     */
    char ch = advance();
    if (isalpha(ch) || ch == '_')
        return match_identifier();
    switch (ch)
    {
    case ':':
        return TokenType::COLON;
    case '?':
        return TokenType::QUESTION_MARK;
    case '(':
        return TokenType::LEFT_PAREN;
    case ')':
        return TokenType::RIGHT_PAREN;
    case '{':
        return TokenType::LEFT_BRACE;
    case '}':
        return TokenType::RIGHT_BRACE;
    case ';':
        return TokenType::SEMICOLON;
    case ',':
        return TokenType::COMMA;
    case '.':
        return TokenType::DOT;
    case '-':
        return TokenType::MINUS;
    case '+':
        return TokenType::PLUS;
    case '/':
        return TokenType::SLASH;
    case '*':
        return TokenType::STAR;
    case '!':
        return match('=') ? TokenType::BANG_EQUAL : TokenType::BANG;
    case '=':
        return match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL;
    case '<':
        return match('=') ? TokenType::LESS_EQUAL : TokenType::LESS;
    case '>':
        return match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER;
    case '"':
        return match_string();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return match_number();
    default:
        err = ErrorCode::UNRECOGNIZED_CHARACTER;
    }
    return TokenType::ERROR;
}

Lexer::const_token_iterator::const_token_iterator() {}

auto Lexer::const_token_iterator::operator==(const_token_iterator other) const -> bool
{
    return start == other.start && current == other.current;
}

auto Lexer::const_token_iterator::operator!=(const_token_iterator other) const -> bool
{
    return !operator==(other);
}

auto Lexer::const_token_iterator::operator*() const -> Token
{
    if (current_token_type == TokenType::END_OF_FILE)
        return Token{};
    return cached_token;
}

auto Lexer::const_token_iterator::operator++() -> const_token_iterator &
{
    // Pre-increment, advance the token and return the current token
    // If there are no more characters that can be consumed, return END_OF_FILE
    err = ErrorCode::NO_ERROR;
    if (is_at_end())
    {
        current_token_type = TokenType::END_OF_FILE;
        start = source.size();
        current = source.size();
    }
    else
    {
        start = current;
        current_token_type = scan_token();
    }
    cached_token.line = line;
    cached_token.lexeme = source.substr(start, current - start);
    cached_token.token_type = current_token_type;
    cached_token.err = err;
    return *this;
}

auto Lexer::const_token_iterator::operator++(int) -> const_token_iterator
{
    // Post increment
    const_token_iterator tmp = *this;
    ++(*this);
    return tmp;
}

auto Lexer::begin() const -> const_token_iterator { return cbegin(); }

auto Lexer::end() const -> const_token_iterator { return cend(); }

auto Lexer::cbegin() const -> const_token_iterator
{
    const_token_iterator it(src, 0, 0, 1);
    ++it;
    return it;
}

auto Lexer::cend() const -> const_token_iterator
{
    return const_token_iterator(src, src.size(), src.size(), 1);
}

Lexer::Lexer(std::string_view src) : src(src) {}