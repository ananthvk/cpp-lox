#include "bench_helper.hpp"
#include <benchmark/benchmark.h>

static void BM_RunMethodInvocations(benchmark::State &state)
{
    VMOpts opts;
    LoxRuntime runtime(opts);
    const char *setup_src = "class Counter { init() { this.count = 100; } doesnothing() {} }"
                            "var c = Counter();";
    auto setup_fn = runtime.compile(setup_src);
    runtime.mark_never_delete(setup_fn);
    runtime.execute(setup_fn);

    const char *call_src =
        "for(var i = 0; i < 1000000; i = i + 1) { c.doesnothing(); }"; // takes around 130ms
                                                                       // reduced to around 85ms
                                                                       // after OP_INVOKE
    auto call_fn = runtime.compile(call_src);
    runtime.mark_never_delete(call_fn);

    for (auto _ : state)
    {
        runtime.execute(call_fn);
    }
}

BENCHMARK(BM_RunMethodInvocations);

static void BM_InitMethod(benchmark::State &state)
{
    VMOpts opts;
    LoxRuntime runtime(opts);
    const char *setup_src = "class Foo { init() {} }";
    auto setup_fn = runtime.compile(setup_src);
    runtime.mark_never_delete(setup_fn);
    runtime.execute(setup_fn);

    const char *call_src =
        "for(var i = 0; i < 1000000; i = i + 1) { var x = Foo(); }"; // Takes ~120 ms without
                                                                     // optimizations, and around
                                                                     // the same time with the
                                                                     // optimization.

    auto call_fn = runtime.compile(call_src);
    runtime.mark_never_delete(call_fn);

    for (auto _ : state)
    {
        runtime.execute(call_fn);
    }
}

BENCHMARK(BM_InitMethod);


BENCHMARK_MAIN();
