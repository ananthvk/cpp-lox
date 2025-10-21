#pragma once
#include "allocator.hpp"
#include "chunk.hpp"
#include "config.hpp"
#include "context.hpp"
#include "error_reporter.hpp"
#include "function.hpp"
#include "hashmap.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "result.hpp"
#include <functional>
#include <string_view>

class GarbageCollector;

struct CompilerOpts
{
    bool debug_print_tokens = false;

    bool dump_function_bytecode = false;
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
    struct Local
    {
        Token name;
        int depth;
        bool uninitialized;
        bool is_const;
        bool is_captured;
    };

    struct Upvalue
    {
        int index;
        bool is_local;
    };

    CompilerOpts opts;
    Allocator &allocator;
    Context *context;
    int scope_depth;
    std::vector<Local> locals;
    HashMap<int64_t, int> constant_numbers;

    ObjectFunction *function;
    Parser &parser;

    std::vector<ParseRule> rules;

    int loop_depth;
    int loop_scope_depth;
    int loop_start_offset;

    FunctionType function_type;

    Compiler *enclosing;
    std::vector<Upvalue> upvalues;


    // These functions generate bytecode, and add it to the chunk
    // held by the compiler.

    auto emit_opcode(OpCode code) -> void;
    auto emit_opcode(OpCode code, uint8_t byte) -> void;
    auto emit_byte(uint8_t byte) -> void;
    auto emit_uint16_le(uint16_t bytes) -> void;
    auto emit_return() -> void;

    auto emit_jump(OpCode code) -> int;
    auto emit_jump_back(int absolute_location) -> void;
    auto patch_jump(int offset) -> void;


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
    auto dot(bool canAssign) -> void;
    auto ternary(bool canAssign) -> void;
    auto literal(bool canAssign) -> void;
    auto string(bool canAssign) -> void;
    auto variable(bool canAssign) -> void;

    /**
     * Logical operators
     */
    auto and_(bool canAssign) -> void;
    auto or_(bool canAssign) -> void;

    /**
     * Function call
     */
    auto call(bool canAssign) -> void;
    auto argument_list() -> uint8_t;

    /**
     * Statements
     */
    auto statement() -> void;
    auto compile_function(FunctionType fun_type, std::string_view name) -> void;
    // Compiles a list of variables, separated by a comma and ending with end_type. doesn't consume
    // Token end_type
    auto parameters(TokenType end_type) -> void;
    auto block() -> void;
    auto begin_scope() -> void;
    auto end_scope() -> void;
    auto declaration() -> void;
    auto print_statement() -> void;
    auto expression_statement() -> void;
    auto var_declaration() -> void;
    auto fun_declaration() -> void;
    auto class_declaration() -> void;
    auto mark_initialized() -> void;
    auto const_declaration() -> void;
    auto if_statement() -> void;
    auto while_statement() -> void;
    auto for_statement() -> void;
    auto switch_statement() -> void;
    auto break_statement() -> void;
    auto return_statement() -> void;
    auto continue_statement() -> void;

    /**
     * Helper functions to declare and define variables
     */
    auto identifier(std::string_view name) -> int;
    auto declare_variable(bool is_constant) -> void;
    auto define_variable(int constant_index, bool is_const) -> void;
    auto add_local(Token name, bool is_constant) -> void;
    auto resolve_local(Token name) -> int;
    auto named_variable(Token name, bool canAssign) -> void;

    // Resolves the upvalue for the given name, returns the upvalue index if found, -1 otherwise.
    // The second value is true if the value is a const variable, false otherwise
    auto resolve_upvalue(Token name) -> std::pair<int, bool>;
    auto add_upvalue(int index, bool is_local) -> int;

    /**
     * This function parses any expression at `precedence` level or higher
     * For example if this function is called ParsePrecedence::TERM, it can either match a term,
     * factor, unary, call or primary
     */
    auto parse_precedence(ParsePrecedence precedence) -> void;

    auto parse_variable(std::string_view err_message, bool is_constant) -> int;

    auto chunk() -> Chunk * { return function->get(); }

  public:
    static Compiler *current;

    Compiler(Parser &parser, const CompilerOpts &opts, Allocator &allocator, Context *context,
             FunctionType function_type, Compiler *enclosing = nullptr, std::string_view name = "");

    ~Compiler()
    {
        // Once the compiler finishes, set the static curren variable to the enclosing compiler
        current = enclosing;
    }

    auto compile() -> std::pair<ObjectFunction *, InterpretResult>;

    auto static mark_compiler_roots(GarbageCollector &gc, bool is_vm_live) -> void;

    auto static does_compiler_exist() -> bool;
};