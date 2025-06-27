benchmark_liblox = static_library(
    'liblox_bench',
    sources: sources,
    include_directories: include_dirs,
    dependencies: project_deps,
    override_options: ['b_sanitize=none', 'b_lto=false', 'optimization=3'],
    build_by_default: false,
)

benchmark_dep = dependency(
    'benchmark',
    required: true,
    default_options: ['benchmark:BENCHMARK_ENABLE_TESTING=false'],
)

benchmark_sources = files(
    'bench_expressions.cpp',
)

benchmark_exe = executable(
    'bench_expressions',
    benchmark_sources,
    dependencies: [benchmark_dep] + project_deps,
    include_directories: include_dirs,
    link_with: [benchmark_liblox],
    override_options: ['b_sanitize=none', 'b_lto=false', 'optimization=3'],
    install: false,
    build_by_default: false,
)

benchmark(
    'bench_expressions',
    benchmark_exe,
    args: ['--benchmark_time_unit=us'],
    env: [],
    timeout: 0,
)