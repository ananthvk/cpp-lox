#include "compiler.hpp"
#include "debug.hpp"

Compiler::Compiler(const CompilerOpts &opts) : opts(opts) {}

auto Compiler::compile(std::string_view source) const -> std::pair<Chunk, InterpretResult>
{
    Lexer lexer(source);
    Parser parser(lexer.begin());
    Chunk chunk;

    if (opts.debug_print_tokens)
    {
        print_tokens(lexer);
    }

    auto iter = lexer.begin();
    while ((*iter).token_type != TokenType::END_OF_FILE)
    {
        if ((*iter).token_type == TokenType::ERROR)
        {
            return {chunk, InterpretResult::COMPILE_ERROR};
        }
        ++iter;
    }

    chunk.write_load_constant(chunk.add_constant(51), 1);
    chunk.write_simple_op(OpCode::RETURN, 2);
    return {chunk, InterpretResult::OK};
}