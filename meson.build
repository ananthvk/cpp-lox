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
    'src/allocator.cpp',
    'src/chunk.cpp',
    'src/compiler.cpp',
    'src/debug.cpp',
    'src/gc.cpp',
    'src/lexer.cpp',
    'src/lox.cpp',
    'src/native/classes.cpp',
    'src/native/list.cpp',
    'src/native/math.cpp',
    'src/native/memory.cpp',
    'src/native/stdio.cpp',
    'src/native/stdlib.cpp',
    'src/native/string.cpp',
    'src/native/test.cpp',
    'src/native/time.cpp',
    'src/parser.cpp',
    'src/register_native_functions.cpp',
    'src/serializer.cpp',
    'src/value.cpp',
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