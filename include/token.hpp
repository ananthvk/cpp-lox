#pragma once
#include "errcodes.hpp"
#include "token_type.hpp"
#include <string_view>

struct Token
{
    TokenType token_type;
    int line;
    ErrorCode err;
    std::string_view lexeme;

    Token() : token_type(TokenType::END_OF_FILE), line(1), err(ErrorCode::NO_ERROR) {}
};