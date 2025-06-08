srcs = [
    'lox.cpp',
    'debug.cpp',
    'chunk.cpp',
    'vm.cpp',
    'lexer.cpp',
    'compiler.cpp',
    'parser.cpp',
]

liblox = shared_library(
    'lox',
    sources: srcs,
    include_directories: include_dirs,
    cpp_args: extra_args,
)

lox_exec = executable(
    'cpplox',
    sources: ['main.cpp'],
    dependencies: [fmt],
    link_with: [liblox],
    cpp_args: extra_args,
    include_directories: include_dirs,
)