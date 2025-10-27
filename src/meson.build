libcommon = static_library(
    'loxcommon',
    sources: common_sources,
    include_directories: include_dirs,
    cpp_args: dev_flags,
    dependencies: project_deps,
)

libcompiler = static_library(
    'loxcompiler',
    sources: compiler_sources,
    include_directories: include_dirs,
    cpp_args: dev_flags,
    dependencies: project_deps,
    link_with: [libcommon],
)

libvm = static_library(
    'loxvm',
    sources: vm_sources,
    include_directories: include_dirs,
    cpp_args: dev_flags,
    dependencies: project_deps,
    link_with: [libcommon],
)

libserialization = static_library(
    'loxserialization',
    sources: serialization_sources,
    include_directories: include_dirs,
    cpp_args: dev_flags,
    dependencies: project_deps,
    link_with: [libcommon],
)

liblox = static_library(
    'lox',
    sources: lox_sources,
    include_directories: include_dirs,
    cpp_args: dev_flags,
    dependencies: project_deps,
    link_with: [libcommon, libcompiler, libvm, libserialization],
)

subdir('tools')
subdir('bin')