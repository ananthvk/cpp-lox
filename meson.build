project(
    'cpplox',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=3', 'cpp_std=c++20'],
)

if meson.get_compiler('cpp').get_id() == 'clang' and get_option('development')
    extra_args = [
        '-Wall',
        '-Wformat=2',
        '-Wshadow',
        '-Wfloat-equal',
        '-Wshift-overflow',
        '-Wextra',
        '-Wconversion',
        '-Wsign-conversion',
        '-Wcast-align',
        '-pedantic',
        '-gdwarf-4',
        '-O',
        '-fsanitize=integer,address,undefined,integer-divide-by-zero,shift,null,return,signed-integer-overflow,float-divide-by-zero,float-cast-overflow,bounds,alignment,vptr,leak',
        '-pedantic',
        '-Wno-sign-compare',
        '-Wno-unused-parameter',
        '-Wno-sign-conversion',
        '-ftrapv',
        '-D_GLIBCXX_DEBUG',
        '-D_GLIBCXX_DEBUG_PEDANTIC',
        '-D_FORTIFY_SOURCE=2',
        '-fstack-protector',
        '-Wno-unused-parameter',
        '-Wno-implicit-int-float-conversion',
    ]
else
    extra_args = []
endif

add_project_arguments('-DDATA_FOLDER="data"', language: 'cpp')
include_dirs = include_directories(['include'])
fmt = dependency('fmt')

subdir('src')

if get_option('enable-tests')
    subdir('tests')
endif