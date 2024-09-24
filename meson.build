project('A Moonlit Walk', [ 'c' ],
    version : '2024',
    default_options : [ 
        'c_std=c99',
        #'cpp_std=c++17',
        'buildtype=debugoptimized',
        'default_library=static',
        'warning_level=3',
    ])

cc = meson.get_compiler('c')
sys = host_machine.system()
fs = import('fs')

sources = []
assembly = []
shaders = []

defs = []
common_c_args = [
    '-Wall',
    '-Wno-pedantic',
    '-Werror=implicit-function-declaration',
    '-Werror=incompatible-pointer-types',
    '-Werror=return-type',
]

if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
    defs += '-DAMW_DEBUG'
endif

dep_threads = dependency('threads')
dep_math = cc.find_library('m', required : false)
dep_rt = cc.find_library('rt', required : false)
dep_dl = cc.find_library('dl', required : false)

have_posix = cc.has_header_symbol('unistd.h', '_POSIX_VERSION')
systype = (have_posix ? '@0@ (POSIX)' : '@0@').format(host_machine.system())
systype = '@0@, @1@'.format(systype, host_machine.cpu())
if meson.is_cross_build()
    systype = '@0@ (cross-compiling)'.format(systype)
endif

platform = []
enabled_display_backends = []

audio_deps = []
enabled_audio_backends = []

subdir('shaders')
subdir('src')

moonlitwalk = executable('moonlitwalk',
    [ sources, shaders, assembly ],
    c_args : [ defs, common_c_args ],
    dependencies : [ platform, audio_deps ],
    include_directories : 'include',
    install : false)

summary({
    'System type' : systype,
    'Build type' : get_option('buildtype'),
    'Assembly config' : assembly_config,
}, bool_yn : true, section : 'Engine')

summary({
    'Display backends' : '@0@'.format(', '.join(enabled_display_backends)),
    'Audio backends' : '@0@'.format(', '.join(enabled_audio_backends)),
    'Rendering backends' : 'Vulkan',
}, bool_yn : true, section : 'Features')

#summary({
#}, bool_yn : true, section : 'Installation')
