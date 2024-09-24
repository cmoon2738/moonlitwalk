sources += files(
    'hadal.c',
    'lake.c',
    'main.c',
    'common/log.c',
)

# Unix
if have_posix
    sources += files(
        'unix/dll.c',
        'unix/posix_mutex.c',
        'unix/posix_sem.c',
        'unix/posix_threads.c',
    )
    defs += '-DAMW_PLATFORM_UNIX'
endif

# Windows
if sys == 'windows'
    error('Windows not supported right now, sorry')

    defs += [
        '-DAMW_PLATFORM_WINDOWS',
        '-DVK_USE_PLATFORM_WIN32_KHR' ]
    platform += dep_threads
    platform += cc.find_library('ole32', required: true)
	platform += cc.find_library('winmm', required: true)
    enabled_display_backends += [ 'win32' ]

# MacOSX / iOS
elif sys == 'darwin'
    error('Apple platforms not supported right now, sorry')

    defs += [
        '-DAMW_PLATFORM_APPLE',
        '-DVK_USE_PLATFORM_METAL_EXT' ]
    platform += [ dep_threads, dep_math, dep_rt, dep_dl ]
    platform += declare_dependency(link_args: [
        '-framework', 'Cocoa',
        '-framework', 'IOKit',
        '-framework', 'CoreFoundation' ])
    add_languages('objc')

    if host_machine.subsystem() == 'ios'
        defs += '-DAMW_PLATFORM_IOS'
        platform += declare_dependency(link_args: [
            '-framework', 'UIKit' ])
    else
        defs += '-DAMW_PLATFORM_MACOSX'
    endif
    enabled_display_backends += [ 'cocoa' ]

# Android
elif sys == 'android'
    error('Android not supported right now, sorry')

    defs += [
        '-DAMW_PLATFORM_ANDROID',
        '-DVK_USE_PLATFORM_ANDROID_KHR' ]
    cross_link_args += [ '-u', 'ANativeActivity_onCreate' ]
    android_ndk = '/opt/android-ndk'

    dep_android = cc.find_library('android', required : true)
    dep_log = cc.find_library('log', required : true)

    # Native App Glue
    lib_android_native_glue = static_library('android_app_glue',
        include_directories : [include_directories(android_ndk + '/sources/android/native_app_glue')],
        [android_ndk + '/sources/android/native_app_glue/android_native_app_glue.c'])

    dep_android_native_glue = declare_dependency(
        include_directories : [include_directories(android_ndk + '/sources/android/native_app_glue')],
        link_with : lib_android_native_glue)

    platform += [ dep_threads, dep_math, dep_rt, dep_dl, dep_android, dep_log, dep_android_native_glue ]
    enabled_display_backends += [ 'android' ]

# Linux
elif sys == 'linux' or sys == 'bsd'
    sources += files(
        'linux/linux_system.c',
    )

    # Wayland
    dep_wayland_client = dependency('wayland-client', required : get_option('wayland'))
    dep_wayland_protocols = dependency('wayland-protocols', version : '>= 1.12', required : get_option('wayland'))
    dep_xkbcommon = dependency('xkbcommon', required : get_option('wayland'))
    wayland_scanner = find_program('wayland-scanner', required : get_option('wayland'))
    build_wayland_ws = (dep_wayland_client.found() and dep_wayland_protocols.found() and
                        wayland_scanner.found() and dep_xkbcommon.found())
    if build_wayland_ws
        sources += files(
            'linux/wl_display.c',
            'linux/wl_window.c',
        )

        defs += [ 
            '-DAMW_NATIVE_WAYLAND', 
            '-DVK_USE_PLATFORM_WAYLAND_KHR' ]
        platform += [ dep_wayland_client, dep_xkbcommon ]

        wayland_protocols_dir = dep_wayland_protocols.get_pkgconfig_variable('pkgdatadir')
        client_header_args = [ wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@' ]
        private_code_args = [ wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@' ]

        wayland_protocols = {
            'xdg-shell'           : '',
            #'xdg-output'          : '-unstable-v1',
            #'xdg-decoration'      : '-unstable-v1',
            #'xdg-activation'      : '-v1',
            #'pointer-constraints' : '-unstable-v1',
            #'viewporter'          : '',
        }

        foreach name, version : wayland_protocols
            if version == ''
                type = 'stable'
            elif version == '-v1'
                type = 'staging'
            else
                type = 'unstable'
            endif

            protocol_xml_path = wayland_protocols_dir + '/@0@/@1@/@1@@2@.xml'.format(type, name, version)
            client_header = custom_target(
                ['@0@ client-header'.format(name)],
                command : client_header_args,
                input : protocol_xml_path,
                output : name + version + '-protocol.h',
                install : false)
            private_code = custom_target(
                ['@0@ private-code'.format(name)],
                command : private_code_args,
                input : protocol_xml_path,
                output : name + version + '-protocol.c',
                install : false)
            sources += [ client_header, private_code ]
        endforeach

        enabled_display_backends += [ 'wayland' ]
    endif

    # X11 (xcb)
    dep_xcb = dependency('xcb', required : get_option('xcb'))
    dep_xcb_icccm = dependency('xcb-icccm', required : get_option('xcb'))
    build_xcb_ws = dep_xcb.found() and dep_xcb_icccm.found()
    if build_xcb_ws
        #sources += files('')

        defs += [ 
            '-DAMW_NATIVE_XCB', 
            '-DVK_USE_PLATFORM_XCB_KHR' ]
        platform += [ dep_xcb, dep_xcb_icccm ]
        enabled_display_backends += [ 'xcb' ]
    endif

    # DRM/KMS
    dep_udev = dependency('libudev', required : get_option('kms'))
    dep_drm = dependency('libdrm', required : get_option('kms'))
    dep_gbm = dependency('gbm', required : get_option('kms'))
    build_kms_ws = dep_drm.found() and dep_gbm.found() and not get_option('kms').disabled()
    if build_kms_ws
        #sources += files('')

        defs += [
            '-DAMW_NATIVE_KMS',
            '-DVK_USE_PLATFORM_KMS_KHR' ]
        platform += [ dep_udev, dep_drm, dep_gbm ]
        enabled_display_backends += [ 'kms' ]
    endif

    defs += '-DAMW_PLATFORM_LINUX'
    platform += [ dep_threads, dep_math, dep_rt, dep_dl ]
else
    error('Platform not recognized: ' + sys)
endif

subdir('audio')
subdir('renderer')
