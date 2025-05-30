#include "compiler.hpp"
#include "debug.hpp"

Compiler::Compiler(const CompilerOpts &opts, ErrorReporter &reporter)
    : opts(opts), reporter(reporter)
{
}

auto Compiler::compile(std::string_view source) const -> std::pair<Chunk, InterpretResult>
{
    Lexer lexer(source);
    Parser parser(lexer.begin(), reporter);
    Chunk chunk;

    if (opts.debug_print_tokens)
    {
        print_tokens(lexer);
    }

    auto iter = lexer.begin();
    while ((*iter).token_type != TokenType::END_OF_FILE)
    {
        auto token = *iter;
        if (token.token_type == TokenType::ERROR)
        {
            reporter.report(ErrorReporter::ERROR, token, "Syntax Error: {}",
                            error_code_to_string(token.err));
            return {chunk, InterpretResult::COMPILE_ERROR};
        }
        ++iter;
    }

    chunk.write_load_constant(chunk.add_constant(51), 1);
    chunk.write_simple_op(OpCode::RETURN, 2);
    return {chunk, InterpretResult::OK};
}