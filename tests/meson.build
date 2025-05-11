gtest_dep = dependency('gtest')

cppstarter_test = executable(
    'cpplox_test',
    sources: ['cpplox_test.cpp'],
    dependencies: [gtest_dep],
    link_with: [liblox],
    include_directories: include_dirs,
    cpp_args: extra_args,
)
test('cppstarter_test', cppstarter_test)