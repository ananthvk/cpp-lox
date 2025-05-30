#include "compiler.hpp"
#include "debug.hpp"

Compiler::Compiler(std::string_view source, const CompilerOpts &opts, ErrorReporter &reporter)
    : source(source), opts(opts), reporter(reporter), lexer(source), parser(lexer.begin(), reporter)
{
}

auto Compiler::compile() -> InterpretResult
{
    if (opts.debug_print_tokens)
    {
        print_tokens(lexer);
    }

    // Parse a single expression for now
    expression();
    parser.consume(TokenType::END_OF_FILE);

    if (parser.had_error())
        return InterpretResult::COMPILE_ERROR;

    return InterpretResult::OK;
}

auto Compiler::expression() -> void {}

auto Compiler::take_chunk() -> Chunk && { return std::move(chunk); }
