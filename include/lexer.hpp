#pragma once
#include "logger.hpp"
#include "token.hpp"
#include "token_type.hpp"
#include <limits>
#include <stddef.h>
#include <stdint.h>
#include <string_view>

class Lexer
{
  public:
    /**
     * A token iterator represents a single token in the stream of tokens, it is a forward iterator
     */
    class const_token_iterator
    {
        /**
         * `current` is the index of the current character / next character to be consumed. It is
         * not part of the current token, and comes after it
         */

        std::string_view source;
        size_t start, current;
        int line;
        TokenType current_token_type;

        const_token_iterator(std::string_view source, size_t start, size_t current, int line)
            : source(source), start(start), current(current), line(line),
              current_token_type(TokenType::END_OF_FILE)
        {
        }

        /**
         * Returns the the next character to be processed, and moves to the next one
         */
        auto advance() -> char
        {
            char ch = source[current];
            ++current;
            return ch;
        }

        /**
         * Returns the next character to be processed, but does not consume it
         */
        auto peek() const -> char { return source[current]; }

        auto is_at_end() const -> bool { return current >= source.size(); }

        auto scan_token() -> TokenType
        {
            if (is_at_end())
                return TokenType::END_OF_FILE;

            /*
             * Process single character tokens
             */
            char ch = advance();
            switch (ch)
            {
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
            }
            return TokenType::ERROR;
        }

      public:
        const_token_iterator() {}

        auto operator==(const_token_iterator other) const -> bool
        {
            return start == other.start && current == other.current;
        }

        auto operator!=(const_token_iterator other) const -> bool { return !operator==(other); }

        auto operator*() const -> Token
        {
            Token token;
            token.line = line;
            token.lexeme = source.substr(start, current - start);
            token.token_type = current_token_type;
            return token;
        }

        auto operator++() -> const_token_iterator &
        {
            // Pre-increment, advance the token and return the current token
            // If there are no more characters that can be consumed, return END_OF_FILE
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
            return *this;
        }

        auto operator++(int) -> const_token_iterator
        {
            // Post increment
            const_token_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend class Lexer;
    };

    auto begin() const -> const_token_iterator { return cbegin(); }

    auto end() const -> const_token_iterator { return cend(); }

    auto cbegin() const -> const_token_iterator
    {
        const_token_iterator it(src, 0, 0, 1);
        ++it;
        return it;
    }

    auto cend() const -> const_token_iterator
    {
        return const_token_iterator(src, src.size(), src.size(), 1);
    }

    Lexer(std::string_view src) : src(src) {}

  private:
    std::string_view src;
};