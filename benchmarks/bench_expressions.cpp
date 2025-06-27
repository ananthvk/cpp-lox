#include "allocator.hpp"
#include "compiler.hpp"
#include "lox.hpp"
#include "vm.hpp"
#include <benchmark/benchmark.h>

class nullbuffer : public std::streambuf
{
  public:
    int overflow(int c) override { return c; }
};

class nullostream : public std::ostream
{
  public:
    nullostream() : std::ostream(&m_sb) {}

  private:
    nullbuffer m_sb;
};

static Chunk compile(std::string_view src, ErrorReporter &reporter, Allocator &allocator)
{
    CompilerOpts copts;
    Compiler compiler(src, copts, allocator, reporter);
    auto result = compiler.compile();
    if (result != InterpretResult::OK)
        throw std::logic_error("Source code has syntax errors");

    auto chunk = compiler.take_chunk();
    return chunk;
}

static void execute(Chunk &chunk, VM &vm)
{
    nullostream os;
    auto result = vm.run(&chunk, os);
    if (result != InterpretResult::OK)
        throw std::logic_error("Runtime error");
}

static void BM_CompileExpression(benchmark::State &state)
{
    ErrorReporter reporter;
    Allocator allocator;
    const char *src =
        "print "
        "(44*63-(-61*-47--49/(51/(93*-21-(((-32*(11*-50-(((52*-39-(48/(-33/(-96*77-76/99-(((91/"
        "-75-60/-3-87*83--75/19)*79-59*43-85/-50--25*-92)/90-58/67--26*35-33*-100)/69-95/-29)-39/"
        "-17--57*65--3*-90)-42*75--11*-70-92*55)*-78+-41/85-34/-57)*62-11/-87-29/93-34/2)/-15--23/"
        "-41-61/-75--92*-74)/-83--82/93--59/68)-21*82--7*7-16*-69)/-13-82*-8+58*73-29*43)/"
        "-100-63*50+-28/71-58/-29)/-5--39*37--24*-23)-85*-17-96/-72--3*49)--87*76-33/59)*39-77/"
        "-90--44*27)/-91-58*-5-0*-62--50*78;";
    for (auto _ : state)
        compile(src, reporter, allocator);
}

BENCHMARK(BM_CompileExpression);

static void BM_RunExpression(benchmark::State &state)
{
    ErrorReporter reporter;
    Allocator allocator;
    VMOpts opts;
    VM vm(opts, reporter, allocator);
    const char *src =
        "(44*63-(-61*-47--49/(51/(93*-21-(((-32*(11*-50-(((52*-39-(48/(-33/(-96*77-76/99-(((91/"
        "-75-60/-3-87*83--75/19)*79-59*43-85/-50--25*-92)/90-58/67--26*35-33*-100)/69-95/-29)-39/"
        "-17--57*65--3*-90)-42*75--11*-70-92*55)*-78+-41/85-34/-57)*62-11/-87-29/93-34/2)/-15--23/"
        "-41-61/-75--92*-74)/-83--82/93--59/68)-21*82--7*7-16*-69)/-13-82*-8+58*73-29*43)/"
        "-100-63*50+-28/71-58/-29)/-5--39*37--24*-23)-85*-17-96/-72--3*49)--87*76-33/59)*39-77/"
        "-90--44*27)/-91-58*-5-0*-62--50*78;";
    auto chunk = compile(src, reporter, allocator);
    for (auto _ : state)
        execute(chunk, vm);
}

BENCHMARK(BM_RunExpression);

static void BM_RunExpressionNegation(benchmark::State &state)
{
    ErrorReporter reporter;
    Allocator allocator;
    VMOpts opts;
    VM vm(opts, reporter, allocator);
    const char *src =
        "------------------------------------------------------------------------------------------"
        "------------------------------------------------------------------------------------------"
        "-------------8+---------------------------------------------------------------------------"
        "------------------------------------------------------------------------------------------"
        "----------------------------------------------------------7;";
    auto chunk = compile(src, reporter, allocator);
    for (auto _ : state)
        execute(chunk, vm);
}

BENCHMARK(BM_RunExpressionNegation);

static void BM_GlobalVariables(benchmark::State &state)
{
    ErrorReporter reporter;
    Allocator allocator;
    VMOpts opts;
    VM vm(opts, reporter, allocator);
    const char *src =
        "var a = 3;\n"
        "var b = 77;\n"
        "var c = 180;\n"
        "var d = 15;\n"
        "var abcdefghij = 20;\n"
        "var x;\n"
        "var l;\n"
        "var abc12345;\n"
        "var ab;\n"
        "var bc;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "abcdefghij = abcdefghij + 1;\n"
        "print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print "
        "abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;print abcdefghij;";
    auto chunk = compile(src, reporter, allocator);
    for (auto _ : state)
        execute(chunk, vm);
}

BENCHMARK(BM_GlobalVariables);

BENCHMARK_MAIN();
