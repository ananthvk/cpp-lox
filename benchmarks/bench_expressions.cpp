#include "bench_helper.hpp"
#include <benchmark/benchmark.h>

static void BM_CompileExpression(benchmark::State &state)
{
    VMOpts opts;
    LoxRuntime runtime(opts);
    const char *src =
        "echo "
        "(44*63-(-61*-47--49/(51/(93*-21-(((-32*(11*-50-(((52*-39-(48/(-33/(-96*77-76/99-(((91/"
        "-75-60/-3-87*83--75/19)*79-59*43-85/-50--25*-92)/90-58/67--26*35-33*-100)/69-95/-29)-39/"
        "-17--57*65--3*-90)-42*75--11*-70-92*55)*-78+-41/85-34/-57)*62-11/-87-29/93-34/2)/-15--23/"
        "-41-61/-75--92*-74)/-83--82/93--59/68)-21*82--7*7-16*-69)/-13-82*-8+58*73-29*43)/"
        "-100-63*50+-28/71-58/-29)/-5--39*37--24*-23)-85*-17-96/-72--3*49)--87*76-33/59)*39-77/"
        "-90--44*27)/-91-58*-5-0*-62--50*78;";
    for (auto _ : state)
        runtime.compile(src);
}

BENCHMARK(BM_CompileExpression);

static void BM_RunExpression(benchmark::State &state)
{
    VMOpts opts;
    LoxRuntime runtime(opts);
    const char *src =
        "(44*63-(-61*-47--49/(51/(93*-21-(((-32*(11*-50-(((52*-39-(48/(-33/(-96*77-76/99-(((91/"
        "-75-60/-3-87*83--75/19)*79-59*43-85/-50--25*-92)/90-58/67--26*35-33*-100)/69-95/-29)-39/"
        "-17--57*65--3*-90)-42*75--11*-70-92*55)*-78+-41/85-34/-57)*62-11/-87-29/93-34/2)/-15--23/"
        "-41-61/-75--92*-74)/-83--82/93--59/68)-21*82--7*7-16*-69)/-13-82*-8+58*73-29*43)/"
        "-100-63*50+-28/71-58/-29)/-5--39*37--24*-23)-85*-17-96/-72--3*49)--87*76-33/59)*39-77/"
        "-90--44*27)/-91-58*-5-0*-62--50*78;";
    auto fn = runtime.compile(src);
    runtime.mark_never_delete(fn);
    for (auto _ : state)
        runtime.execute(fn);
}

BENCHMARK(BM_RunExpression);

static void BM_RunExpressionNegation(benchmark::State &state)
{
    VMOpts opts;
    LoxRuntime runtime(opts);
    const char *src =
        "------------------------------------------------------------------------------------------"
        "------------------------------------------------------------------------------------------"
        "-------------8+---------------------------------------------------------------------------"
        "------------------------------------------------------------------------------------------"
        "----------------------------------------------------------7;";
    auto fn = runtime.compile(src);
    for (auto _ : state)
        runtime.execute(fn);
}

BENCHMARK(BM_RunExpressionNegation);

static void BM_GlobalVariables(benchmark::State &state)
{
    VMOpts opts;
    LoxRuntime runtime(opts);
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
        "echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;";

    auto fn = runtime.compile(src);
    runtime.mark_never_delete(fn);
    for (auto _ : state)
        runtime.execute(fn);
}

BENCHMARK(BM_GlobalVariables);

static void BM_CompileGlobalVariables(benchmark::State &state)
{
    VMOpts opts;
    LoxRuntime runtime(opts);
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
        "echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo "
        "abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;echo abcdefghij;";
    for (auto _ : state)
    {
        runtime.compile(src);
    }
}

BENCHMARK(BM_CompileGlobalVariables);

BENCHMARK_MAIN();
