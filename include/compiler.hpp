#pragma once
#include "allocator.hpp"
#include "chunk.hpp"
#include "error_reporter.hpp"
#include "globals.hpp"
#include "hashmap.hpp"
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

using ParseFunction = std::function<void(bool)>;

struct ParseRule
{
    ParseFunction prefix;
    ParseFunction infix;
    ParsePrecedence precedence;
};

class Compiler
{

    struct StringIndexTableHasher
    {
        auto operator()(const ObjectString *str) const -> size_t { return str->hash(); }
    };

    using StringIndexTable = HashMap<ObjectString *, int, StringIndexTableHasher>;

    std::string_view source;
    CompilerOpts opts;
    Allocator &allocator;
    Globals *globals;
    StringIndexTable constant_strings;
    HashMap<int64_t, int> constant_numbers;

    Chunk chunk;
    Lexer lexer;
    Parser parser;

    std::vector<ParseRule> rules;

    // These functions generate bytecode, and add it to the chunk
    // held by the compiler.

    auto emit_opcode(OpCode code) -> void;
    auto emit_opcode(OpCode code, uint8_t byte) -> void;
    auto emit_byte(uint8_t byte) -> void;
    auto emit_uint16_le(uint16_t bytes) -> void;
    auto emit_return() -> void;

    auto define_global_variable(int constant_index) -> void;

    auto get_rule(TokenType type) -> ParseRule &;

    /**
     * Parses an expression. Internally this function calls parse_precedence with the lowest
     * precedence so that it can parse any expression.
     */
    auto expression() -> void;

    auto number(bool canAssign) -> void;
    auto grouping(bool canAssign) -> void;
    auto unary(bool canAssign) -> void;
    auto binary(bool canAssign) -> void;
    auto ternary(bool canAssign) -> void;
    auto literal(bool canAssign) -> void;
    auto string(bool canAssign) -> void;
    auto variable(bool canAssign) -> void;
    auto identifier(std::string_view name) -> int;
    auto named_variable(Token name, bool canAssign) -> void;

    /**
     * Statements
     */
    auto statement() -> void;
    auto declaration() -> void;
    auto print_statement() -> void;
    auto expression_statement() -> void;
    auto var_declaration() -> void;


    /**
     * This function parses any expression at `precedence` level or higher
     * For example if this function is called ParsePrecedence::TERM, it can either match a term,
     * factor, unary, call or primary
     */
    auto parse_precedence(ParsePrecedence precedence) -> void;

    auto parse_variable(std::string_view err_message) -> int;

  public:
    Compiler(std::string_view source, const CompilerOpts &opts, Allocator &allocator,
             ErrorReporter &reporter, Globals *globals);

    auto compile() -> InterpretResult;

    /**
     * @brief Transfers ownership of the Chunk object held by the compiler
     */
    auto take_chunk() -> Chunk &&;
};