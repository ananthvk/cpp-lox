#pragma once
#include "chunk.hpp"
#include "error_reporter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "result.hpp"
#include <functional>
#include <string_view>

struct CompilerOpts
{
    bool debug_print_tokens = false;
};

enum class ParsePrecedence
{
    NONE,
    ASSIGNMENT,  // =
    CONDITIONAL, // ?:
    OR,          // or
    AND,         // and
    EQUALITY,    // == !=
    COMPARISON,  // > >= < <=
    TERM,        // + -
    FACTOR,      // * /
    UNARY,       // ! -
    CALL,        // . ()
    PRIMARY
};

inline auto operator+(ParsePrecedence type) -> int { return static_cast<int>(type); }

using ParseFunction = std::function<void()>;

struct ParseRule
{
    ParseFunction prefix;
    ParseFunction infix;
    ParsePrecedence precedence;
};

class Compiler
{
    std::string_view source;
    CompilerOpts opts;
    ErrorReporter &reporter;

    Chunk chunk;
    Lexer lexer;
    Parser parser;

    std::vector<ParseRule> rules;

    auto emit_opcode(OpCode code) -> void { chunk.write_simple_op(code, parser.previous().line); }

    auto emit_opcode(OpCode code, uint8_t byte) -> void
    {
        emit_opcode(code);
        emit_byte(byte);
    }

    auto emit_byte(uint8_t byte) -> void { chunk.write_byte(byte, parser.previous().line); }

    auto emit_return() -> void { emit_opcode(OpCode::RETURN); }

    /**
     * Parses any expression by parsing the expression with lowest precedence i.e. assignment
     */
    auto expression() -> void { parse_precedence(ParsePrecedence::ASSIGNMENT); }

    auto number() -> void
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

    auto grouping() -> void
    {
        expression();
        parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
    }

    auto unary() -> void
    {
        auto operator_type = parser.previous().token_type;

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

    auto get_rule(TokenType type) const -> ParseRule { return rules[static_cast<int>(type)]; }

    auto binary() -> void
    {
        auto op = parser.previous().token_type;
        auto rule = get_rule(op);

        // Parse the right hand side of the infix expression
        parse_precedence(static_cast<ParsePrecedence>(+rule.precedence + 1));

        // Note: The operator is emitted after both the left hand and right hand side of the operand
        // is compiled. This is because this VM follows a stack architecture, so to evaluate an
        // addition like 2*3 + 5*6, the LHS and RHS has to be compiled, only then can + work
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

    auto ternary() -> void
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

    /**
     * This function parses any expression at `precedence` level or higher
     * For example if this function is called ParsePrecedence::TERM, it can either match a term,
     * factor, unary, call or primary
     */
    auto parse_precedence(ParsePrecedence precedence) -> void
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

  public:
    Compiler(std::string_view source, const CompilerOpts &opts, ErrorReporter &reporter);

    auto compile() -> InterpretResult;

    auto take_chunk() -> Chunk &&;
};