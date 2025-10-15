#include "compiler.hpp"
#include "debug.hpp"
#include "fast_float.h"

Compiler::Compiler(Parser &parser, const CompilerOpts &opts, Allocator &allocator, Context *context,
                   FunctionType function_type)
    : opts(opts), allocator(allocator), context(context), scope_depth(0), parser(parser),
      rules(static_cast<int>(TokenType::TOKEN_COUNT)), loop_depth(0), loop_scope_depth(-1),
      loop_start_offset(-1), function_type(function_type), enclosing(nullptr)
{
#define F(function) [this](bool canAssign) { function(canAssign); }
    // clang-format off
    rules[+TokenType::LEFT_PAREN]       = {F(grouping),    F(call),          ParsePrecedence::CALL};
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
    rules[+TokenType::AND]              = {nullptr,        F(and_),          ParsePrecedence::AND};
    rules[+TokenType::CLASS]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::CONST]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::ELSE]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FALSE]            = {F(literal),     nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FOR]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::FUN]              = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::IF]               = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NIL]              = {F(literal),     nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::NOT]              = {F(unary),       nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::OR]               = {nullptr,        F(or_),           ParsePrecedence::OR};
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
    rules[+TokenType::COLON]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::SWITCH]           = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::CASE]             = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::DEFAULT]          = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::BREAK]            = {nullptr,        nullptr,          ParsePrecedence::NONE};
    rules[+TokenType::CONTINUE]         = {nullptr,        nullptr,          ParsePrecedence::NONE};
    
    function = allocator.new_function(0, "");
    
    // Reserve the first slot of the locals array
    auto token = Token{};
    add_local(token, false);

    // clang-format on
#undef F
}

auto Compiler::compile() -> std::pair<ObjectFunction *, InterpretResult>
{
    while (!parser.match(TokenType::END_OF_FILE))
    {
        declaration();
    }

    emit_return();

    // TODO: Delete the object in the allocator

    if (parser.had_error())
        return {nullptr, InterpretResult::COMPILE_ERROR};

    return {function, InterpretResult::OK};
}

auto Compiler::emit_opcode(OpCode code) -> void
{
    chunk()->write_simple_op(code, parser.previous().line);
}

auto Compiler::emit_uint16_le(uint16_t bytes) -> void
{
    chunk()->write_uint16_le(bytes, parser.previous().line);
}

auto Compiler::emit_opcode(OpCode code, uint8_t byte) -> void
{
    emit_opcode(code);
    emit_byte(byte);
}

auto Compiler::emit_byte(uint8_t byte) -> void
{
    chunk()->write_byte(byte, parser.previous().line);
}

auto Compiler::emit_return() -> void
{
    emit_opcode(OpCode::NIL);
    emit_opcode(OpCode::RETURN);
}

auto Compiler::emit_jump(OpCode code) -> int
{
    emit_opcode(code);
    emit_uint16_le(0);
    // Returns offset to the start byte of the two-byte jump location
    // [code] 00000000 00000000
    //        ^
    return static_cast<int>(chunk()->get_code().size()) - 2;
}

// This function patches a JUMP instruction by filling in the two byte offset with the jump location
auto Compiler::patch_jump(int offset) -> void
{
    // `jump` is the number of bytes to move the ip forward so that it can jump to the last byte
    // For example,
    //  1 byte   2 byte jmp    1    1    1
    //  [JUMP] [00..] [00..] [..] [..] [..]
    //    0      1      2     3    4    5
    //  In this case, size=6, offset=1, so jump is calculated to be 3
    //  That means that after executing the jump, ip will move from 3 to 6
    int jump = static_cast<int>(chunk()->get_code().size()) - offset - 2;

    if (jump > UINT16_MAX)
    {
        parser.report_error("Too much code to jump over");
    }

    chunk()->get_code()[offset] = static_cast<uint8_t>(jump & 0xFF);
    chunk()->get_code()[offset + 1] = static_cast<uint8_t>((jump >> 8) & 0xFF);
}

auto Compiler::emit_jump_back(int absolute_location) -> void
{
    emit_opcode(OpCode::JUMP_BACKWARD);

    // +2 to account for the size of the operand of this bytecode
    int offset = static_cast<int>(chunk()->get_code().size()) - absolute_location + 2;
    if (offset > UINT16_MAX)
    {
        parser.report_error("Too much code to jump back");
    }
    emit_uint16_le(static_cast<uint16_t>(offset));
}

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
            chunk()->write_load_constant(index_opt.value(), token.line);
        else
        {
            auto index = chunk()->add_constant(value);
            chunk()->write_load_constant(index, token.line);
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

        chunk()->write_load_constant(chunk()->add_constant(value), token.line);
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
    case TokenType::NOT:
        emit_opcode(OpCode::NOT);
        break;
    default:
        return;
    }
}

auto Compiler::and_(bool canAssign) -> void
{
    // When this and is parsed, the left hand expression has already been compiled.
    // At runtime, the value of the left hand side expression will remain at the top of the stack.

    // If LHS value is false, `and` short circuits and skips the right hand side expression using
    // a jump
    int end_jump = emit_jump(OpCode::JUMP_IF_FALSE);

    // If LHS value is true, the jump does not execute, and the LHS expression value is
    // discared. The value of the expression becomes the value of RHS expression
    emit_opcode(OpCode::POP_TOP);

    // This function call parses the entire right hand side expression after and
    parse_precedence(ParsePrecedence::AND);

    patch_jump(end_jump);
}

auto Compiler::or_(bool canAssign) -> void
{
    // Difference between 'or', 'and' is that for 'or', we jump if the LHS becomes true
    int end_jump = emit_jump(OpCode::JUMP_IF_TRUE);

    // If LHS value is false, discard it
    emit_opcode(OpCode::POP_TOP);

    parse_precedence(ParsePrecedence::OR);

    patch_jump(end_jump);
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
        chunk()->write_load_constant(val.value(), token.line);
    }
    else
    {
        int index = chunk()->add_constant(obj);
        chunk()->write_load_constant(index, token.line);
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
    else if (parser.match(TokenType::CONST))
    {
        const_declaration();
    }
    else if (parser.match(TokenType::FUN))
    {
        fun_declaration();
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
    else if (parser.match(TokenType::IF))
    {
        if_statement();
    }
    else if (parser.match(TokenType::WHILE))
    {
        while_statement();
    }
    else if (parser.match(TokenType::FOR))
    {
        for_statement();
    }
    else if (parser.match(TokenType::SWITCH))
    {
        switch_statement();
    }
    else if (parser.match(TokenType::LEFT_BRACE))
    {
        begin_scope();
        block();
        end_scope();
    }
    else if (parser.match(TokenType::CONTINUE))
    {
        continue_statement();
    }
    else if (parser.match(TokenType::BREAK))
    {
        break_statement();
    }
    else if (parser.match(TokenType::RETURN))
    {
        return_statement();
    }
    else
    {
        expression_statement();
    }
}

auto Compiler::block() -> void
{
    while (!parser.check(TokenType::RIGHT_BRACE) && !parser.check(TokenType::END_OF_FILE))
    {
        declaration();
    }
    parser.consume(TokenType::RIGHT_BRACE, "Expected \"}\" after a block");
}

auto Compiler::begin_scope() -> void { scope_depth++; }

auto Compiler::end_scope() -> void
{
    scope_depth--;

    // After a scope ends, pop all temporary values from the stack
    // TODO: Make it more efficient, the current version is O(n)
    while (!locals.empty() && locals.back().depth > scope_depth)
    {
        // If the local is captured by some other nested function, do not pop the variable
        // Instead emit an instruction to move the value to the heap
        if (locals.back().is_captured)
        {
            emit_opcode(OpCode::CLOSE_UPVALUE);
        }
        else
        {
            emit_opcode(OpCode::POP_TOP);
        }
        locals.pop_back();
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
    int variable_constant_index = parse_variable("Expected variable name after 'var'", false);
    if (parser.match(TokenType::EQUAL))
        expression();
    else
        emit_opcode(OpCode::UNINITIALIZED);

    parser.consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    define_variable(variable_constant_index, false);
}

auto Compiler::fun_declaration() -> void
{
    int function_name_constant_index = parse_variable("Expected function name after 'fun'", false);
    // Mark the function as initialized if in local scope so that a function can refer
    // to itself in it's body. This is to support recursive local functions.
    mark_initialized();
    std::string_view function_name = parser.previous().lexeme;
    compile_function(FunctionType::FUNCTION, function_name);
    define_variable(function_name_constant_index, false);
}

auto Compiler::compile_function([[maybe_unused]] FunctionType fun_type, std::string_view name)
    -> void
{
    // The compiler used to compile the function uses the same parser as the parent compiler
    // so that it consumes the complete function.
    Compiler compiler(parser, opts, allocator, context, FunctionType::FUNCTION);
    compiler.enclosing = this;
    compiler.function->set_name(allocator.intern_string(name, Allocator::StorageType::DYNAMIC));
    compiler.begin_scope();
    parser.consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    compiler.parameters(TokenType::RIGHT_PAREN);
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after function parameters");
    parser.consume(TokenType::LEFT_BRACE, "Expected '{' before function body");

    compiler.block();
    compiler.emit_return();

    auto fn = compiler.function;
    if (opts.dump_function_bytecode)
    {
        disassemble_chunk(*fn->get(), std::string("function ") + std::string(name), context);
    }

    int index = chunk()->add_constant(Value{fn});
    // Instead of loading the constant, emit an OP_CLOSURE instruction to tell the VM
    // to wrap the function in a closure
    emit_opcode(OpCode::CLOSURE);
    emit_uint16_le(static_cast<uint16_t>(index));

    // OP_CLOSURE is a variable sized instruction, after the constant index for the function,
    // It emits pairs of one byte & two bytes for every upvalue that the function captures.
    // If the first byte is 0, it means that the function has captured an upvalue
    // If the first byte is 1, it means that the function has captured a local of the parent
    // function The scond byte is an unsigned 16 bit integer that is either the local slot (of the
    // parent function), or an upvalue index

    for (auto upvalue : compiler.upvalues)
    {
        if (upvalue.is_local)
            emit_byte(1);
        else
            emit_byte(0);
        emit_uint16_le(static_cast<uint16_t>(upvalue.index));
    }
}

auto Compiler::parameters(TokenType end_type) -> void
{
    if (parser.check(end_type))
        // No parameters
        return;
    while (1)
    {
        function->arity_++;
        if (function->arity_ > MAX_FUNCTION_PARAMETERS)
        {
            parser.report_error("Too many parameters ({}), has to be less than {}",
                                function->arity_, MAX_FUNCTION_PARAMETERS);
            return;
        }
        int parameter_constant_index = parse_variable("Expected parameter name", false);
        define_variable(parameter_constant_index, false);
        if (!parser.match(TokenType::COMMA))
            return;
    }
}

auto Compiler::mark_initialized() -> void
{
    if (scope_depth > 0)
    {
        locals.back().uninitialized = false;
    }
}

auto Compiler::const_declaration() -> void
{
    int const_constant_index = parse_variable("Expected variable name after 'const'", true);
    if (parser.match(TokenType::EQUAL))
        expression();
    else
    {
        parser.report_error("Expected initializer after const declaration");
        return;
    }

    parser.consume(TokenType::SEMICOLON, "Expected ';' after const declaration");
    define_variable(const_constant_index, true);
}

auto Compiler::declare_variable(bool is_constant) -> void
{
    // If we are in global scope, do not declare the variable
    if (scope_depth == 0)
        return;

    // If it's in a scope, create a local variable with this name
    auto token = parser.previous();

    // Check if a local variable with the same name has been declared in the same scope
    for (auto it = locals.rbegin(); it != locals.rend(); ++it)
    {
        // All other local variables were declared in previous scopes
        if (it->depth < scope_depth)
            break;
        if (it->name.lexeme == token.lexeme)
        {
            parser.report_error("Variable '{}' redeclared in the same scope", token.lexeme);
            return;
        }
    }

    add_local(token, is_constant);
}

auto Compiler::add_local(Token name, bool is_const) -> void
{
    if (locals.size() == MAX_NUMBER_OF_LOCAL_VARIABLES)
    {
        parser.report_error("Limit on the number of local variables reached");
        return;
    }
    locals.push_back({name, scope_depth, true, is_const, false});
}

auto Compiler::variable(bool canAssign) -> void { named_variable(parser.previous(), canAssign); }

auto Compiler::parse_variable(std::string_view err_message, bool is_constant) -> int
{
    parser.consume(TokenType::IDENTIFIER, err_message);
    // First declare the variable, does nothing for global variables
    // For local variables, it stores the variable name along with it's stack position
    declare_variable(is_constant);

    if (scope_depth > 0)
        return 0;

    auto token = parser.previous();
    return identifier(token.lexeme);
}

auto Compiler::define_variable(int constant_index, bool is_const) -> void
{
    // We are not in the global scope, do not do anything
    if (scope_depth > 0)
    {
        locals.back().uninitialized = false;
        return;
    }
    auto &val = context->get_internal_value(constant_index);
    if (val.defined && val.is_const != is_const)
    {
        parser.report_error("Syntax Error: Variable '{}' has been declared, cannot be redeclared",
                            context->get_name(constant_index)->get());
        return;
    }
    val.is_const = is_const;
    emit_opcode(OpCode::DEFINE_GLOBAL);
    emit_uint16_le(static_cast<uint16_t>(constant_index));
}

auto Compiler::identifier(std::string_view name) -> int
{
    ObjectString *obj = allocator.intern_string(name, Allocator::StorageType::DYNAMIC);
    return context->get_global(obj);
}

auto Compiler::resolve_local(Token name) -> int
{
    for (int i = static_cast<int>(locals.size()) - 1; i >= 0; i--)
    {
        if (locals[i].name.lexeme == name.lexeme)
        {
            if (locals[i].uninitialized)
            {
                parser.report_error("Can't use local variable in it's own initializer");
                return -1;
            }
            return i;
        }
    }
    return -1;
}

auto Compiler::named_variable(Token name, bool canAssign) -> void
{
    OpCode store_op, load_op;
    bool is_local = false;
    std::pair<int, bool> resolved;
    bool is_const = false;

    int index = resolve_local(name);
    if (index != -1)
    {
        // It's a local variable
        store_op = OpCode::STORE_LOCAL;
        load_op = OpCode::LOAD_LOCAL;
        is_local = true;
    }
    else if ((resolved = resolve_upvalue(name)).first != -1)
    {
        store_op = OpCode::STORE_UPVALUE;
        load_op = OpCode::LOAD_UPVALUE;
        is_const = resolved.second;
        index = resolved.first;
    }
    else
    {
        // Assume it's a global variable
        index = identifier(name.lexeme);
        store_op = OpCode::STORE_GLOBAL;
        load_op = OpCode::LOAD_GLOBAL;
        is_local = false;
    }

    if (canAssign && parser.match(TokenType::EQUAL))
    {
        // If it's a const variable, disallow assignment
        if (is_local && locals[index].is_const)
        {
            parser.report_error("Syntax Error: Assignment to const variable '{}'",
                                locals[index].name.lexeme);
            return;
        }
        if (is_const)
        {
            parser.report_error("Syntax Error: Assignment to const variable disallowed");
            return;
        }
        expression();
        emit_opcode(store_op);
        emit_uint16_le(static_cast<uint16_t>(index));
    }
    else
    {
        emit_opcode(load_op);
        emit_uint16_le(static_cast<uint16_t>(index));
    }
}

auto Compiler::resolve_upvalue(Token name) -> std::pair<int, bool>
{
    if (enclosing == nullptr)
    {
        // This is the top level compiler
        return {-1, false};
    }

    int local = enclosing->resolve_local(name);
    if (local != -1)
    {
        // The local variable was found in the outer function

        // Also mark the local variable as captured in the parent compiler
        enclosing->locals[local].is_captured = true;
        bool is_const = enclosing->locals[local].is_const;

        return {add_upvalue(local, true), is_const};
    }

    // The local variable does not exist in the immediate outer scope, but it might
    // exist if the function is deeply nested. To solve this issue, recursively resolve upvalues.
    // For example: consider three functions : outer() -> mid() -> inner(), and assume that outer
    // defines a variable "x", and inner accesses that variable. Now the local will not be found
    // since x is not defined in mid, so when resolve_upvalue is called recursively, mid resolves
    // the variable in it's enclosing function (outer), and creates an upvalue for it and returns it
    // The inner function then uses the upvalue that mid captured (and marks is_local as false)
    auto [upvalue, is_const] = enclosing->resolve_upvalue(name);
    if (upvalue != -1)
    {
        return {add_upvalue(upvalue, false), is_const};
    }

    // Not found, assume that it's a global variable
    return {-1, false};
}

auto Compiler::add_upvalue(int index, bool is_local) -> int
{
    // The index here is the index of the variable in the outer function's locals stack
    // The inner compiler (this one) creates an upvalue that references the variable of the outer
    // compiler
    if (upvalues.size() > 0xFFFF)
        throw std::logic_error("Too many upvalues in function");

    // If we find an upvalue that already references the same slot, return that instead of creating
    // a new upvalue
    int idx = 0;
    for (auto upvalue : upvalues)
    {
        if (upvalue.index == index && upvalue.is_local == is_local)
            return idx;
        idx++;
    }
    upvalues.push_back({index, is_local});
    function->upvalue_count_ = static_cast<int>(upvalues.size());
    return static_cast<int>(upvalues.size()) - 1;
}

auto Compiler::if_statement() -> void
{
    parser.consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");

    int then_jump = emit_jump(OpCode::POP_JUMP_IF_FALSE);
    statement();


    if (parser.match(TokenType::ELSE))
    {
        // After the end of the if block, emit a jump statement to after the else block
        int else_jump = emit_jump(OpCode::JUMP_FORWARD);
        patch_jump(then_jump);
        statement();
        patch_jump(else_jump);
    }
    else
    {
        // If there is no else statement this works correctly and there is no fall through
        // If the condition is truthy, it'll execute the then block and executes the next bytecode
        // If it's falsey, it skips the block and directly executes the next bytecode
        patch_jump(then_jump);
    }
}

auto Compiler::while_statement() -> void
{
    loop_depth++;
    begin_scope();
    int loop_begin = static_cast<int>(chunk()->get_code().size());
    parser.consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");

    int outer_loop_scope_depth = loop_scope_depth;
    int outer_loop_start_offset = loop_start_offset;
    loop_scope_depth = scope_depth;

    int exit_jump = emit_jump(OpCode::POP_JUMP_IF_FALSE);

    loop_start_offset = loop_begin;
    // Compile the loop body
    statement();
    // After the body, jump back to the loop start
    emit_jump_back(loop_begin);

    patch_jump(exit_jump);
    loop_depth--;

    loop_scope_depth = outer_loop_scope_depth;
    loop_start_offset = outer_loop_start_offset;

    end_scope();
}

auto Compiler::for_statement() -> void
{
    // A for statement is of the form:
    // for (initializer; condition; update) { ... body ...}
    // Initializer, condition, and update can all be omitted

    loop_depth++;

    // Start a new scope so that loop variables are scoped only to the loop
    begin_scope();


    parser.consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");

    // All the below statements consume the ";" after the end of the initializer
    if (parser.match(TokenType::SEMICOLON))
    {
        // There is no initializer
    }
    else if (parser.match(TokenType::VAR))
    {
        var_declaration();
    }
    else if (parser.match(TokenType::CONST))
    {
        const_declaration();
    }
    else
    {
        expression_statement();
    }

    // Store the previous (enclosing) loop scope depth & loop start offset
    int outer_loop_scope_depth = loop_scope_depth;
    int outer_loop_start_offset = loop_start_offset;

    loop_scope_depth = scope_depth;

    // Compile the test condition
    // Note: This expression has to be executed in every iteration of the loop, so set loop_begin to
    // this location
    int loop_begin = static_cast<int>(chunk()->get_code().size());
    int exit_jump = -1;
    if (!parser.match(TokenType::SEMICOLON))
    {
        expression();
        parser.consume(TokenType::SEMICOLON, "Expected ';' after loop condition");

        // If the condition evaluates to false, jump out of the loop
        exit_jump = emit_jump(OpCode::POP_JUMP_IF_FALSE);
    }

    // Compile the update/increment expression
    // Note: Since we are not building an AST before generating the bytecode, we need to add extra
    // JUMP instructions to make it compile in a single pass.

    // Example:
    // [INITIALIZER]
    // [CONDITION]
    // <JUMP TO BODY>
    // [INCREMENT EXPRESSION]
    // <JUMP TO CONDITION>
    // [BODY]
    // <JUMP TO INCREMENT EXPRESSION>
    if (!parser.match(TokenType::RIGHT_PAREN))
    {
        // Emit a jump to start of body of the loop
        int body_jump = emit_jump(OpCode::JUMP_FORWARD);
        int increment_begin = static_cast<int>(chunk()->get_code().size());

        // Compile the increment/update expression and discard the result
        expression();
        emit_opcode(OpCode::POP_TOP);
        parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after for statement");

        emit_jump_back(loop_begin);
        loop_begin = increment_begin;
        patch_jump(body_jump);
    }

    loop_start_offset = loop_begin;

    // Compile the loop body
    statement();
    emit_jump_back(loop_begin);

    if (exit_jump != -1)
    {
        // If there was a loop condition, backpatch it to the loop's end
        patch_jump(exit_jump);
    }

    loop_scope_depth = outer_loop_scope_depth;
    loop_start_offset = outer_loop_start_offset;

    end_scope();
    loop_depth--;
}

auto Compiler::switch_statement() -> void
{
    // Parenthesis around expression are optional

    expression();
    parser.consume(TokenType::LEFT_BRACE, "Expected '{' after switch statement");
    std::vector<int> exit_jumps;

    bool default_case_compiled = false;


    while (!parser.check(TokenType::RIGHT_BRACE) && !parser.check(TokenType::END_OF_FILE))
    {
        // Look for a case or default statement
        if (parser.match(TokenType::CASE) || parser.match(TokenType::DEFAULT))
        {
            // Duplicate the value of the switch variable since EQUAL consumes it
            if (parser.previous().token_type == TokenType::CASE)
            {
                emit_opcode(OpCode::DUP_TOP);
            }

            begin_scope();
            // Just add a dummy local variable so that the local variable stack offset will be
            // computed correctly
            auto dummy = parser.previous();
            dummy.lexeme = "<switch_variable>";
            add_local(dummy, false);

            int skip_jump = -1;

            if (parser.previous().token_type == TokenType::CASE)
            {
                if (default_case_compiled)
                {
                    parser.report_error("Cannot have cases following the 'default' case");
                }
                expression();
                // Check for equality
                emit_opcode(OpCode::EQUAL);
                skip_jump = emit_jump(OpCode::POP_JUMP_IF_FALSE);
            }
            else
            {
                if (default_case_compiled)
                {
                    parser.report_error(
                        "Cannot have multiple 'default' statements in a single switch case");
                }
                default_case_compiled = true;
            }

            parser.consume(TokenType::COLON, "Expected ':' after case or default statement");

            if (parser.check(TokenType::DEFAULT) || parser.check(TokenType::CASE) ||
                parser.check(TokenType::RIGHT_BRACE))
            {
                parser.report_error("Empty cases not allowed in switch statement");
            }

            // Add declarations (both statements & variable declarations) to the current arm until
            // we hit a case, default keyword or a right brace
            while (!(parser.check(TokenType::RIGHT_BRACE) || parser.check(TokenType::CASE) ||
                     parser.check(TokenType::DEFAULT)) &&
                   !parser.check(TokenType::END_OF_FILE))
            {
                declaration();
            }

            end_scope();

            int exit_jump = emit_jump(OpCode::JUMP_FORWARD);
            exit_jumps.push_back(exit_jump);
            if (skip_jump != -1)
            {
                patch_jump(skip_jump);
            }
        }
        else
        {
            parser.report_error("Expected 'case' or 'default' statements inside switch statement");
            return;
        }
    }
    parser.consume(TokenType::RIGHT_BRACE, "Expected '}' after switch statement");
    for (auto exit_jump : exit_jumps)
        patch_jump(exit_jump);
}

auto Compiler::break_statement() -> void
{
    if (loop_depth == 0)
        parser.report_error("Cannot use break statement outside loop");
    parser.consume(TokenType::SEMICOLON, "Expected ';' after break statement");
    parser.report_error("NOT YET IMPLEMENTED");
}

auto Compiler::continue_statement() -> void
{
    if (loop_depth == 0)
        parser.report_error("Cannot use continue statement outside loop");
    parser.consume(TokenType::SEMICOLON, "Expected ';' after continue statement");

    // POP locals created in the loop body
    for (int i = static_cast<int>(locals.size()) - 1; i >= 0; i--)
    {
        if (locals[i].depth <= loop_scope_depth)
            break;
        emit_opcode(OpCode::POP_TOP);
    }

    // Add a backward jump to the loop test condition / initialization
    emit_jump_back(loop_start_offset);
}

auto Compiler::call(bool canAssign) -> void
{
    auto arg_count = argument_list();
    emit_opcode(OpCode::CALL);
    emit_byte(arg_count);
}

auto Compiler::argument_list() -> uint8_t
{
    int arg_count = 0;
    if (parser.check(TokenType::RIGHT_PAREN))
    {
        parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
        return 0;
    }
    while (1)
    {
        expression();
        if (arg_count == MAX_FUNCTION_PARAMETERS)
        {
            parser.report_error("Too many arguments: {}", arg_count);
            break;
        }
        arg_count++;
        if (!parser.match(TokenType::COMMA))
            break;
    }
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
    return static_cast<uint8_t>(arg_count);
}

auto Compiler::return_statement() -> void
{
    if (function_type == FunctionType::SCRIPT)
    {
        parser.report_error("cannot return from top level script, must be used inside a function");
    }
    if (parser.match(TokenType::SEMICOLON))
    {
        // No value to return
        emit_return();
        return;
    }
    expression();
    parser.consume(TokenType::SEMICOLON, "Expected ';' after return statement");
    emit_opcode(OpCode::RETURN);
}