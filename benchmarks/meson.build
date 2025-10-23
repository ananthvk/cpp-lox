# Include current directory
current_dir = include_directories('.')

# Library for your Lox runtime
benchmark_liblox = static_library(
    'liblox_bench',
    sources: sources,
    include_directories: [include_dirs, current_dir],
    dependencies: project_deps,
    override_options: ['b_sanitize=none', 'b_lto=false', 'optimization=3'],
    build_by_default: false,
)

# Benchmark dependency
benchmark_dep = dependency(
    'benchmark',
    required: true,
    default_options: ['benchmark:BENCHMARK_ENABLE_TESTING=false'],
)

# List of benchmark sources
benchmark_sources = files(
    'bench_expressions.cpp',
    'bench_invocations.cpp',
)

# Generate executables for each benchmark source
benchmarks = []
foreach src : benchmark_sources
    exe_name = src.full_path().split('/')[-1].split('.')[0]
    exe = executable(
        exe_name,
        src,
        dependencies: [benchmark_dep] + project_deps,
        include_directories: [include_dirs, current_dir],
        link_with: [benchmark_liblox],
        override_options: ['b_sanitize=none', 'b_lto=false', 'optimization=3'],
        install: false,
        build_by_default: false,
    )
    benchmarks += [exe]
    benchmark(
        exe_name,
        exe,
        args: ['--benchmark_time_unit=ms'],
        env: [],
        timeout: 0,
    )
endforeach