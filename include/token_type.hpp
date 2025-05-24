#pragma once
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
    X(NUMBER)                                                                                      \
    X(AND)                                                                                         \
    X(CLASS)                                                                                       \
    X(ELSE)                                                                                        \
    X(FALSE)                                                                                       \
    X(FOR)                                                                                         \
    X(FUN)                                                                                         \
    X(IF)                                                                                          \
    X(NIL)                                                                                         \
    X(OR)                                                                                          \
    X(PRINT)                                                                                       \
    X(RETURN)                                                                                      \
    X(SUPER)                                                                                       \
    X(THIS)                                                                                        \
    X(TRUE)                                                                                        \
    X(VAR)                                                                                         \
    X(WHILE)                                                                                       \
    X(ERROR)                                                                                       \
    X(END_OF_FILE)

enum class TokenType
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
