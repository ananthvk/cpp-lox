#include "compiler.hpp"
#include "debug.hpp"

Compiler::Compiler(std::string_view source, const CompilerOpts &opts, ErrorReporter &reporter)
    : source(source), opts(opts), reporter(reporter), lexer(source),
      parser(lexer.begin(), reporter), rules(static_cast<int>(TokenType::TOKEN_COUNT))
{
#define F(function) [this]() { function(); }
    // clang-format off
    rules[+TokenType::LEFT_PAREN]       = {F(grouping),    nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::RIGHT_PAREN]      = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::LEFT_BRACE]       = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::RIGHT_BRACE]      = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::COMMA]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::DOT]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::MINUS]            = {F(unary),       F(binary),        ParsePrecedence::TERM};
    rules[+TokenType::PLUS]             = {nullptr,        F(binary),        ParsePrecedence::TERM};
    rules[+TokenType::SEMICOLON]        = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::SLASH]            = {nullptr,        F(binary),        ParsePrecedence::FACTOR};
    rules[+TokenType::STAR]             = {nullptr,        F(binary),        ParsePrecedence::FACTOR};
    rules[+TokenType::BANG]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::BANG_EQUAL]       = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::EQUAL]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::EQUAL_EQUAL]      = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::GREATER]          = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::GREATER_EQUAL]    = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::LESS]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::LESS_EQUAL]       = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::IDENTIFIER]       = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::STRING]           = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NUMBER_INT]       = {F(number),      nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NUMBER_REAL]      = {F(number),      nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::AND]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::CLASS]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::CONST]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::ELSE]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FALSE]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FOR]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FUN]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::IF]               = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NIL]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NOT]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::OR]               = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::PRINT]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::RETURN]           = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::SUPER]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::THIS]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::TRUE]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::VAR]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::WHILE]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::ERROR]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::END_OF_FILE]      = {nullptr,        nullptr,          ParsePrecedence::NONE};
    // clang-format on
#undef F
}

auto Compiler::compile() -> InterpretResult
{
    if (opts.debug_print_tokens)
    {
        print_tokens(lexer);
    }

    // Parse a single expression for now
    expression();
    emit_return();
    parser.consume(TokenType::END_OF_FILE);

    if (parser.had_error())
        return InterpretResult::COMPILE_ERROR;

    return InterpretResult::OK;
}

auto Compiler::take_chunk() -> Chunk && { return std::move(chunk); }
