#pragma once
#include "token_type.hpp"
#include <string_view>

struct Token
{
    TokenType token_type;
    int line;
    std::string_view lexeme;

    Token() : token_type(TokenType::END_OF_FILE), line(1) {}
};