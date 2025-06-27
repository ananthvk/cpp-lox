cpp = meson.get_compiler('cpp')

if meson.get_compiler('cpp').get_id() == 'clang'
    sanitizers = [
        'address',
    ]
    available_sanitizers = []
    foreach flag : sanitizers
        if cpp.has_argument('-fsanitize=' + flag)
            available_sanitizers += flag
        endif
    endforeach

    if available_sanitizers.length() != 0
        additional_sanitizers = '-fsanitize=' + ','.join(available_sanitizers)
    else
        additional_sanitizers = ''
    endif
else
    additional_sanitizers = ''
endif

dev_raw_flags = [
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
    '-Wno-sign-compare',
    '-Wno-unused-parameter',
    '-Wno-sign-conversion',
    '-ftrapv',
    '-D_GLIBCXX_DEBUG',
    '-D_GLIBCXX_DEBUG_PEDANTIC',
    '-D_FORTIFY_SOURCE=2',
    '-fstack-protector',
    '-Wno-implicit-int-float-conversion', additional_sanitizers,
]

dev_flags = []

if get_option('development')
    foreach flag : dev_raw_flags
        if cpp.has_argument(flag)
            dev_flags += flag
        endif
    endforeach
endif