liblox = static_library(
    'lox',
    sources: sources,
    include_directories: include_dirs,
    cpp_args: dev_flags,
    dependencies: project_deps,
)

lox_executable = executable(
    'cpplox',
    sources: ['main.cpp'],
    dependencies: project_deps,
    link_with: [liblox],
    cpp_args: dev_flags,
    include_directories: include_dirs,
)