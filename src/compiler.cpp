#include "compiler.hpp"
#include "debug.hpp"
#include "fast_float.h"

Compiler::Compiler(std::string_view source, const CompilerOpts &opts, Allocator &allocator,
                   ErrorReporter &reporter, Context *context)
    : source(source), opts(opts), allocator(allocator), context(context), lexer(source),
      parser(lexer.begin(), reporter), rules(static_cast<int>(TokenType::TOKEN_COUNT))
{
#define F(function) [this](bool canAssign) { function(canAssign); }
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
    rules[+TokenType::BANG]             = {F(unary),       nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::BANG_EQUAL]       = {nullptr,        F(binary),        ParsePrecedence::EQUALITY};
    rules[+TokenType::EQUAL]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::EQUAL_EQUAL]      = {nullptr,        F(binary),        ParsePrecedence::EQUALITY};
    rules[+TokenType::GREATER]          = {nullptr,        F(binary),        ParsePrecedence::COMPARISON};
    rules[+TokenType::GREATER_EQUAL]    = {nullptr,        F(binary),        ParsePrecedence::COMPARISON};
    rules[+TokenType::LESS]             = {nullptr,        F(binary),        ParsePrecedence::COMPARISON};
    rules[+TokenType::LESS_EQUAL]       = {nullptr,        F(binary),        ParsePrecedence::COMPARISON};
    rules[+TokenType::IDENTIFIER]       = {F(variable),    nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::STRING]           = {F(string),      nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NUMBER_INT]       = {F(number),      nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NUMBER_REAL]      = {F(number),      nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::AND]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::CLASS]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::CONST]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::ELSE]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FALSE]            = {F(literal),     nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FOR]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FUN]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::IF]               = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NIL]              = {F(literal),     nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NOT]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::OR]               = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::PRINT]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::RETURN]           = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::SUPER]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::THIS]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::TRUE]             = {F(literal),     nullptr,          ParsePrecedence::NONE};
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

    while (!parser.match(TokenType::END_OF_FILE))
    {
        declaration();
    }

    emit_return();

    if (parser.had_error())
        return InterpretResult::COMPILE_ERROR;

    return InterpretResult::OK;
}

auto Compiler::take_chunk() -> Chunk && { return std::move(chunk); }

auto Compiler::emit_opcode(OpCode code) -> void
{
    chunk.write_simple_op(code, parser.previous().line);
}

auto Compiler::emit_uint16_le(uint16_t bytes) -> void
{
    chunk.write_uint16_le(bytes, parser.previous().line);
}

auto Compiler::emit_opcode(OpCode code, uint8_t byte) -> void
{
    emit_opcode(code);
    emit_byte(byte);
}

auto Compiler::emit_byte(uint8_t byte) -> void { chunk.write_byte(byte, parser.previous().line); }

auto Compiler::emit_return() -> void { emit_opcode(OpCode::RETURN); }

auto Compiler::expression() -> void { parse_precedence(ParsePrecedence::ASSIGNMENT); }

auto Compiler::number([[maybe_unused]] bool canAssign) -> void
{
    auto token = parser.previous();
    // Only deduplicate integer constants

    if (token.token_type == TokenType::NUMBER_INT)
    {
        int64_t value;
        auto answer = fast_float::from_chars(token.lexeme.data(),
                                             token.lexeme.data() + token.lexeme.size(), value);
        if (answer.ec != std::errc())
        {
            parser.report_error("Error while parsing number as integer '{}'", token.lexeme);
            return;
        }

        auto index_opt = constant_numbers.get(value);
        if (index_opt)
            chunk.write_load_constant(index_opt.value(), token.line);
        else
        {
            auto index = chunk.add_constant(value);
            chunk.write_load_constant(index, token.line);
            constant_numbers.insert(value, index);
        }
    }
    if (token.token_type == TokenType::NUMBER_REAL)
    {
        double value;
        auto answer = fast_float::from_chars(token.lexeme.data(),
                                             token.lexeme.data() + token.lexeme.size(), value);
        if (answer.ec != std::errc())
        {
            parser.report_error("Error while parsing number as double '{}'", token.lexeme);
            return;
        }

        chunk.write_load_constant(chunk.add_constant(value), token.line);
    }
}

auto Compiler::grouping([[maybe_unused]] bool canAssign) -> void
{
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
}

auto Compiler::unary([[maybe_unused]] bool canAssign) -> void
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
    case TokenType::BANG:
        emit_opcode(OpCode::NOT);
        break;
    default:
        return;
    }
}

auto Compiler::get_rule(TokenType type) -> ParseRule & { return rules[static_cast<int>(type)]; }

auto Compiler::binary([[maybe_unused]] bool canAssign) -> void
{
    auto op = parser.previous().token_type;
    auto &rule = get_rule(op);

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

    case TokenType::EQUAL_EQUAL:
        emit_opcode(OpCode::EQUAL);
        break;
    case TokenType::BANG_EQUAL:
        emit_opcode(OpCode::EQUAL);
        emit_opcode(OpCode::NOT);
        break;
    case TokenType::GREATER:
        emit_opcode(OpCode::GREATER);
        break;
    case TokenType::GREATER_EQUAL:
        emit_opcode(OpCode::LESS);
        emit_opcode(OpCode::NOT);
        break;
    case TokenType::LESS:
        emit_opcode(OpCode::LESS);
        break;
    case TokenType::LESS_EQUAL:
        emit_opcode(OpCode::GREATER);
        emit_opcode(OpCode::NOT);
        break;

    default:
        throw std::logic_error("Invalid token type to binary()");
    }
}

auto Compiler::ternary([[maybe_unused]] bool canAssign) -> void
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
    auto &prefix_rule = get_rule(parser.previous().token_type).prefix;
    if (prefix_rule == nullptr)
    {
        parser.report_error("Expected expression");
        return;
    }
    bool canAssign = +precedence <= +ParsePrecedence::ASSIGNMENT;
    prefix_rule(canAssign);

    while ((+precedence) <= +get_rule(parser.peek().token_type).precedence)
    {
        parser.advance();
        auto &infix_rule = get_rule(parser.previous().token_type).infix;
        infix_rule(canAssign);
    }

    if (canAssign && parser.match(TokenType::EQUAL))
    {
        parser.report_error("Invalid assignment target");
    }
}

auto Compiler::literal([[maybe_unused]] bool canAssign) -> void
{
    auto token = parser.previous();
    switch (token.token_type)
    {
    case TokenType::FALSE:
        emit_opcode(OpCode::FALSE);
        break;
    case TokenType::TRUE:
        emit_opcode(OpCode::TRUE);
        break;
    case TokenType::NIL:
        emit_opcode(OpCode::NIL);
        break;
    default:
        throw std::logic_error("Invalid token type to literal");
        break;
    }
}

auto Compiler::string([[maybe_unused]] bool canAssign) -> void
{
    // TODO: This couples the compiler with the runtime (VM), fix this by making the runtime create
    // the strings instead. For example if the program is going to emit bytecode (like .pyc/.class
    // files), which will later be executed

    auto token = parser.previous();

    ObjectString *obj = allocator.intern_string(token.lexeme.substr(1, token.lexeme.size() - 2),
                                                Allocator::StorageType::DYNAMIC);
    auto val = constant_strings.get(obj);
    if (val)
    {
        chunk.write_load_constant(val.value(), token.line);
    }
    else
    {
        int index = chunk.add_constant(obj);
        chunk.write_load_constant(index, token.line);
        constant_strings.insert(obj, index);
    }
}

auto Compiler::declaration() -> void
{
    // A declaration is either a statement or a variable declaration
    if (parser.match(TokenType::VAR))
    {
        var_declaration();
    }
    else
    {
        statement();
    }
    if (parser.is_panic())
    {
        parser.synchronize();
    }
}

auto Compiler::statement() -> void
{
    if (parser.match(TokenType::PRINT))
    {
        print_statement();
    }
    else
    {
        expression_statement();
    }
}

auto Compiler::print_statement() -> void
{
    expression();
    parser.consume(TokenType::SEMICOLON, "Expected ';' after print statement");
    emit_opcode(OpCode::PRINT);
}

auto Compiler::expression_statement() -> void
{
    if (parser.peek().token_type == TokenType::SEMICOLON)
    {
        // Empty expression, so don't add POP_TOP
        parser.advance();
        return;
    }
    expression();
    parser.consume(TokenType::SEMICOLON, "Expected ';' after expression");
    // Pop the top of the value stack, i.e. discard the value
    emit_opcode(OpCode::POP_TOP);
}

auto Compiler::var_declaration() -> void
{
    int variable_constant_index = parse_variable("Expected variable name after 'var'");
    if (parser.match(TokenType::EQUAL))
        expression();
    else
        emit_opcode(OpCode::UNINITIALIZED);

    parser.consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    define_global_variable(variable_constant_index);
}

auto Compiler::variable(bool canAssign) -> void { named_variable(parser.previous(), canAssign); }

auto Compiler::parse_variable(std::string_view err_message) -> int
{
    parser.consume(TokenType::IDENTIFIER, err_message);
    auto token = parser.previous();
    return identifier(token.lexeme);
}

auto Compiler::define_global_variable(int constant_index) -> void
{
    emit_opcode(OpCode::DEFINE_GLOBAL);
    emit_uint16_le(static_cast<uint16_t>(constant_index));
}

auto Compiler::identifier(std::string_view name) -> int
{
    ObjectString *obj = allocator.intern_string(name, Allocator::StorageType::DYNAMIC);
    return context->get_global(obj);
}

auto Compiler::named_variable(Token name, bool canAssign) -> void
{
    int index = identifier(name.lexeme);

    if (canAssign && parser.match(TokenType::EQUAL))
    {
        expression();
        emit_opcode(OpCode::STORE_GLOBAL);
        emit_uint16_le(static_cast<uint16_t>(index));
    }
    else
    {
        emit_opcode(OpCode::LOAD_GLOBAL);
        emit_uint16_le(static_cast<uint16_t>(index));
    }
}