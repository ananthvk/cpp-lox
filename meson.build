project(
    'cpplox',
    'cpp',
    version: '0.0.1',
    default_options: [
        'warning_level=3',
        'cpp_std=c++20',
    ],
)

include_dirs = include_directories(['include', 'thirdparty'])
project_deps = [dependency('fmt')]

# Sources used by all binaries
common_sources = files(
    'src/common/allocator.cpp',
    'src/common/chunk.cpp',
    'src/common/debug_vm.cpp',
    'src/common/gc.cpp',
    'src/common/value.cpp',
)

# Sources used only by the compiler
compiler_sources = files(
    'src/compiler/compiler.cpp',
    'src/compiler/debug_compiler.cpp',
    'src/compiler/lexer.cpp',
    'src/compiler/parser.cpp',
)

# Sources used only by the VM
vm_sources = files(
    'src/native/classes.cpp',
    'src/native/list.cpp',
    'src/native/map.cpp',
    'src/native/math.cpp',
    'src/native/memory.cpp',
    'src/native/stdio.cpp',
    'src/native/stdlib.cpp',
    'src/native/string.cpp',
    'src/native/test.cpp',
    'src/native/time.cpp',
    'src/vm/register_native_functions.cpp',
    'src/vm/vm.cpp',
)

# Sources used by serialization
serialization_sources = files(
    'src/serialization/deserializer.cpp',
    'src/serialization/serializer.cpp',
)

# Sources used by the lox library
lox_sources = files('src/lox/lox.cpp')

# Stubs
compiler_stub = files('src/stubs/compiler_stub.cpp')

subdir('flags')
subdir('src')

if get_option('enable-tests')
    subdir('tests')
endif

if get_option('enable-benchmarks')
    subdir('benchmarks')
endif