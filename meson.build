project(
    'cpplox',
    'cpp',
    version: '0.1',
    default_options: [
        'warning_level=3',
        'cpp_std=c++20',
    ],
)

include_dirs = include_directories(['include', 'thirdparty'])
project_deps = [dependency('fmt')]

sources = files(
    'src/chunk.cpp',
    'src/compiler.cpp',
    'src/debug.cpp',
    'src/lexer.cpp',
    'src/lox.cpp',
    'src/parser.cpp',
    'src/vm.cpp',
)

subdir('flags')
subdir('src')

if get_option('enable-tests')
    subdir('tests')
endif

if get_option('enable-benchmarks')
    subdir('benchmarks')
endif