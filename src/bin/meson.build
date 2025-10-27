executable(
    'loxvm',
    sources: ['loxvm.cpp'] + compiler_stub,
    dependencies: project_deps,
    link_with: [libcommon, libvm, libserialization],
    cpp_args: dev_flags,
    include_directories: include_dirs,
)

lox_executable = executable(
    'cpplox',
    sources: ['cpplox.cpp'],
    dependencies: project_deps,
    link_with: [liblox],
    cpp_args: dev_flags,
    include_directories: include_dirs,
)