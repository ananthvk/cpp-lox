#include "compiler.hpp"
#include "debug.hpp"

Compiler::Compiler(std::string_view source, const CompilerOpts &opts, ErrorReporter &reporter)
    : source(source), opts(opts), lexer(source), parser(lexer.begin(), reporter),
      rules(static_cast<int>(TokenType::TOKEN_COUNT))
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
    rules[+TokenType::QUESTION_MARK]    = {nullptr,        F(ternary),       ParsePrecedence::CONDITIONAL};
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

auto Compiler::emit_opcode(OpCode code) -> void
{
    chunk.write_simple_op(code, parser.previous().line);
}

auto Compiler::emit_opcode(OpCode code, uint8_t byte) -> void
{
    emit_opcode(code);
    emit_byte(byte);
}

auto Compiler::emit_byte(uint8_t byte) -> void { chunk.write_byte(byte, parser.previous().line); }

auto Compiler::emit_return() -> void { emit_opcode(OpCode::RETURN); }

auto Compiler::expression() -> void { parse_precedence(ParsePrecedence::ASSIGNMENT); }

auto Compiler::number() -> void
{
    auto token = parser.previous();

    // TODO: Inefficient since a new string is allocated, write own number conversion function
    if (token.token_type == TokenType::NUMBER_INT)
        chunk.write_load_constant(chunk.add_constant(std::stoll(std::string(token.lexeme))),
                                  token.line);
    if (token.token_type == TokenType::NUMBER_REAL)
        chunk.write_load_constant(chunk.add_constant(std::stod(std::string(token.lexeme))),
                                  token.line);
}

auto Compiler::grouping() -> void
{
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
}

auto Compiler::unary() -> void
{
    auto operator_type = parser.previous().token_type;

    // Note: Here parse_precedence is called with the same precedence
    // i.e. UNARY, this makes unary operators right associative.
    // For example, the following expression: --3 will be parsed as
    // -(-3)
    parse_precedence(ParsePrecedence::UNARY);

    switch (operator_type)
    {
    case TokenType::MINUS:
        emit_opcode(OpCode::NEGATE);
        break;
    default:
        return;
    }
}

auto Compiler::get_rule(TokenType type) const -> ParseRule { return rules[static_cast<int>(type)]; }

auto Compiler::binary() -> void
{
    auto op = parser.previous().token_type;
    auto rule = get_rule(op);

    // Parse the right hand side of the infix expression, precedence is one higher
    // so it's left associative
    parse_precedence(static_cast<ParsePrecedence>(+rule.precedence + 1));

    // Note: The operator is emitted after both the left hand and right hand side of the operand
    // is compiled. This is because this VM follows a stack architecture, so to evaluate an
    // addition like 2*3 + 5*6, the LHS and RHS has to be compiled first, aftewards + is performed
    switch (op)
    {
    case TokenType::PLUS:
        emit_opcode(OpCode::ADD);
        break;
    case TokenType::MINUS:
        emit_opcode(OpCode::SUBTRACT);
        break;
    case TokenType::SLASH:
        emit_opcode(OpCode::DIVIDE);
        break;
    case TokenType::STAR:
        emit_opcode(OpCode::MULTIPLY);
        break;

    default:
        throw std::logic_error("Invalid token type to binary()");
    }
}

auto Compiler::ternary() -> void
{
    // The compiler has compiled the condition
    // Check for "then" expression, followed by a ":", then a "else" expression
    // When parsing the "else" branch, parse it with the same precedence

    // TODO: understand this solution

    // Parse the "then" expression
    parse_precedence(ParsePrecedence::CONDITIONAL);

    // Check if there is a ":"
    parser.consume(TokenType::COLON, "Expected ':' after then branch of ternary expression");

    // Parse the "else" expression
    parse_precedence(ParsePrecedence::ASSIGNMENT);
}

auto Compiler::parse_precedence(ParsePrecedence precedence) -> void
{
    parser.advance();
    auto prefix_rule = get_rule(parser.previous().token_type).prefix;
    if (prefix_rule == nullptr)
    {
        parser.report_error("Expected expression");
        return;
    }

    prefix_rule();

    while ((+precedence) <= +get_rule(parser.peek().token_type).precedence)
    {
        parser.advance();
        auto infix_rule = get_rule(parser.previous().token_type).infix;
        infix_rule();
    }
}