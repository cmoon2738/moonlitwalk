build_asiosdk = cc.check_header('windows.h', required : get_option('asio')) and fs.is_dir('asiosdk/common')
if build_asiosdk
    add_languages('cpp')

    # The ASIO SDK sources are pulled from:
    # https://www.steinberg.net/asiosdk
    sources += files(
        'asiosdk/common/asio.cpp',
        'asiosdk/common/asiolist.cpp',
        'asiosdk/common/asiodrivers.cpp',
        'iasiothiscallresolver.cpp')
endif
