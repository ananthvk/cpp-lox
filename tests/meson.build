test_py = find_program('pytest', required: true)

lox_path = lox_exec.full_path()

test(
    'pytest',
    test_py,
    args: ['-q', meson.current_source_dir()],
    env: ['PYTHONPATH=' + meson.project_source_root(), 'LOX_BINARY=' + lox_path],
    suite: 'python',
    depends: [lox_exec]
)