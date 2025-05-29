#pragma once
#include <stddef.h>
#include <stdint.h>

#define ERROR_LIST                                                                                 \
    X(NO_ERROR)                                                                                    \
    X(UNTERMINATED_STRING)                                                                         \
    X(UNRECOGNIZED_CHARACTER)                                                                      \
    X(INVALID_DECIMAL_LITERAL)


enum class ErrorCode : uint16_t
{
#define X(err) err,
    ERROR_LIST
#undef X
};

inline auto error_code_to_string(ErrorCode code) -> const char *
{
    switch (code)
    {
#define X(name)                                                                                    \
    case ErrorCode::name:                                                                          \
        return #name;
        ERROR_LIST
#undef X
    default:
        return "ERROR_UNKNOWN";
    }
}