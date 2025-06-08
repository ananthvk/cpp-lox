#pragma once
#include "token.hpp"
#include <ctype.h>
#include <limits>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
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
        ErrorCode err;

        const_token_iterator(std::string_view source, size_t start, size_t current, int line);

        /**
         * Returns the the next character to be processed, and moves to the next one
         */
        auto advance() -> char;

        auto match(char ch) -> bool;

        auto is_at_end() const -> bool;

        auto peek() const -> char;

        auto peek_next() const -> char;

        auto skip_whitespace() -> void;

        auto match_string() -> TokenType;

        auto match_number() -> TokenType;

        auto lexeme_length() const -> size_t;

        auto check_keyword(int idx, int length, const char *rest, TokenType type) const
            -> TokenType;

        auto get_identifier_type() const -> TokenType;

        auto match_identifier() -> TokenType;

        auto scan_token() -> TokenType;

      public:
        const_token_iterator();

        auto operator==(const_token_iterator other) const -> bool;

        auto operator!=(const_token_iterator other) const -> bool;

        auto operator*() const -> Token;

        auto operator++() -> const_token_iterator &;

        auto operator++(int) -> const_token_iterator;

        friend class Lexer;
    };

    auto begin() const -> const_token_iterator;

    auto end() const -> const_token_iterator;

    auto cbegin() const -> const_token_iterator;

    auto cend() const -> const_token_iterator;

    Lexer(std::string_view src);

  private:
    std::string_view src;
};