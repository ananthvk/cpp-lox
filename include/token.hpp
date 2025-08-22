#pragma once
#include "errcodes.hpp"
#include <string_view>

#define TOKEN_TYPE_LIST                                                                            \
    X(LEFT_PAREN)                                                                                  \
    X(RIGHT_PAREN)                                                                                 \
    X(LEFT_BRACE)                                                                                  \
    X(RIGHT_BRACE)                                                                                 \
    X(COMMA)                                                                                       \
    X(DOT)                                                                                         \
    X(MINUS)                                                                                       \
    X(PLUS)                                                                                        \
    X(SEMICOLON)                                                                                   \
    X(SLASH)                                                                                       \
    X(STAR)                                                                                        \
    X(BANG)                                                                                        \
    X(BANG_EQUAL)                                                                                  \
    X(EQUAL)                                                                                       \
    X(EQUAL_EQUAL)                                                                                 \
    X(GREATER)                                                                                     \
    X(GREATER_EQUAL)                                                                               \
    X(LESS)                                                                                        \
    X(LESS_EQUAL)                                                                                  \
    X(IDENTIFIER)                                                                                  \
    X(STRING)                                                                                      \
    X(NUMBER_INT)                                                                                  \
    X(NUMBER_REAL)                                                                                 \
    X(AND)                                                                                         \
    X(CLASS)                                                                                       \
    X(CONST)                                                                                       \
    X(ELSE)                                                                                        \
    X(FALSE)                                                                                       \
    X(FOR)                                                                                         \
    X(FUN)                                                                                         \
    X(IF)                                                                                          \
    X(NIL)                                                                                         \
    X(NOT)                                                                                         \
    X(OR)                                                                                          \
    X(PRINT)                                                                                       \
    X(RETURN)                                                                                      \
    X(SUPER)                                                                                       \
    X(THIS)                                                                                        \
    X(TRUE)                                                                                        \
    X(VAR)                                                                                         \
    X(WHILE)                                                                                       \
    X(ERROR)                                                                                       \
    X(END_OF_FILE)                                                                                 \
    X(COLON)                                                                                       \
    X(QUESTION_MARK)                                                                               \
    X(SWITCH)                                                                                      \
    X(CASE)                                                                                        \
    X(DEFAULT)                                                                                     \
    X(TOKEN_COUNT)

enum class TokenType : uint8_t
{
#define X(name) name,
    TOKEN_TYPE_LIST
#undef X
};

inline auto token_type_to_string(TokenType type) -> const char *
{
    switch (type)
    {
#define X(name)                                                                                    \
    case TokenType::name:                                                                          \
        return #name;
        TOKEN_TYPE_LIST
#undef X
    default:
        return "TOKEN_UNKNOWN";
    }
}

inline auto operator+(TokenType type) -> int { return static_cast<int>(type); }

struct Token
{
    TokenType token_type;
    int line;
    ErrorCode err;
    std::string_view lexeme;

    Token() : token_type(TokenType::END_OF_FILE), line(1), err(ErrorCode::NO_ERROR) {}
};