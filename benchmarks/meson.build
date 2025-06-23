if not get_option('enable-benchmarks')
    subdir_done()
endif

benchmark_liblox = static_library(
    'liblox_bench',
    sources: files(
        '../src/chunk.cpp',
        '../src/compiler.cpp',
        '../src/debug.cpp',
        '../src/lexer.cpp',
        '../src/lox.cpp',
        '../src/parser.cpp',
        '../src/vm.cpp',
    ),
    include_directories: include_dirs,
    dependencies: [fmt],
    override_options: ['b_sanitize=none', 'b_lto=false', 'optimization=3'],
    build_by_default: false,
)

benchmark_dep = dependency('benchmark', required: true)

benchmark_sources = files(
    'bench_expressions.cpp',
)

benchmark_exe = executable(
    'bench_expressions',
    benchmark_sources,
    dependencies: [benchmark_dep],
    include_directories: include_dirs,
    link_with: [benchmark_liblox],
    #override_options: ['b_sanitize=none', 'b_lto=false', 'optimization=3'],
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