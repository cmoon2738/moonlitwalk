#sources += files('')

# CoreAudio
dep_core = dependency('appleframeworks', modules : [ 'CoreAudio' ], required : get_option('core'))
if dep_core.found()
    #sources += files('backends/coreaudio.m')

    defs += '-DAMW_NATIVE_COREAUDIO'
    audio_deps += dep_core
    enabled_audio_backends += [ 'coreaudio' ]
endif

# Steinberg ASIO
build_asiosdk = cc.check_header('windows.h', required : get_option('asio')) and fs.is_dir('backends/asiosdk/common')
if build_asiosdk
    # The ASIO SDK sources are pulled from:
    # https://www.steinberg.net/asiosdk
    sources += files([
        'backends/asiosdk/common/asio.cpp',
        'backends/asiosdk/common/asiolist.cpp',
        'backends/asiosdk/common/asiodrivers.cpp',
        'backends/iasiothiscallresolver.cpp',
        'backends/asio.cpp' ])
    defs += '-DAMW_NATIVE_ASIO'
    enabled_audio_backends += [ 'asio' ]
endif

# WASAPI
found_wasapi = cc.check_header('audioclient.h', required : get_option('wasapi'))
if found_wasapi
    #sources += files('backends/wasapi.c')

    defs += '-DAMW_NATIVE_WASAPI'
    audio_deps += cc.find_library('mfplat', required : true)
    audio_deps += cc.find_library('mfuuid', required : true)
    audio_deps += cc.find_library('ksuser', required : true)
    audio_deps += cc.find_library('wmcodecdspuuid', required : true)
    enabled_audio_backends += [ 'wasapi' ]
endif

# XAudio2
dep_xaudio = cc.find_library('xaudio2', required : get_option('xaudio2'))
if dep_xaudio.found()
    #sources += files('backends/xaudio2.c')
    
    defs += '-DAMW_NATIVE_XAUDIO2'
    audio_deps += dep_xaudio
    enabled_audio_backends += [ 'xaudio2' ]
endif

# AAudio
dep_aaudio = cc.find_library('aaudio', required : get_option('aaudio'))
if dep_aaudio.found()
    #sources += files('backends/aaudio.c')

    defs += '-DAMW_NATIVE_AAUDIO'
    audio_deps += dep_aaudio
    enabled_audio_backends += [ 'aaudio' ]
endif

# ALSA
dep_alsa = dependency('alsa', required : get_option('alsa'))
if dep_alsa.found()
    #sources += files('backends/alsa.c')

    defs += '-DAMW_NATIVE_ALSA'
    audio_deps += dep_alsa
    enabled_audio_backends += [ 'alsa' ]
endif

# JACK
dep_jack = dependency('jack', required : get_option('jack'))
if dep_jack.found()
    #sources += files('backends/jack.c')

    defs += '-DAMW_NATIVE_JACK'
    audio_deps += dep_jack
    enabled_audio_backends += [ 'jack' ]
endif

# PulseAudio
dep_pulse = dependency('libpulse-simple', required : get_option('pulse'))
if dep_pulse.found()
    #sources += files('backends/pulseaudio.c')

    defs += '-DAMW_NATIVE_PULSEAUDIO'
    audio_deps += dep_pulse
    enabled_audio_backends += [ 'pulseaudio' ]
endif

# PipeWire
dep_pipewire = dependency('libpipewire-0.3', required : get_option('pipewire'))
if dep_pipewire.found()
    #sources += files('backends/pipewire.c')

    defs += [ 
        '-DAMW_NATIVE_PIPEWIRE', 
        '-D_GNU_SOURCE' ]
    audio_deps += dep_pipewire
    enabled_audio_backends += [ 'pipewire' ]
endif

# OSS
found_oss = not get_option('oss').disabled() and ((sys == 'linux') or (sys == 'bsd'))
if found_oss
    #sources += files('backends/oss.c')

    defs += '-DAMW_NATIVE_OSS'
    enabled_audio_backends += [ 'oss' ]
endif

