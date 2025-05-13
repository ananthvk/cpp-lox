srcs = ['lox.cpp', 'debug.cpp', 'chunk.cpp']

liblox = static_library(
    'lox',
    sources: srcs,
    include_directories: include_dirs,
)

executable(
    'cpplox',
    sources: ['main.cpp'],
    dependencies: [fmt],
    link_with: [liblox],
    cpp_args: extra_args,
    include_directories: include_dirs,
)