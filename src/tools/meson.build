executable(
    'loxdump',
    sources: ['loxdump.cpp'] + compiler_stub,
    dependencies: project_deps,
    link_with: [libcommon, libserialization],
    cpp_args: dev_flags,
    include_directories: include_dirs,
)