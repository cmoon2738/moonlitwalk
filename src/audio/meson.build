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
if build_asiosdk
    sources += files([
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

# DirectSound
dep_dsound = cc.find_library('dsound', required : get_option('dsound'))
if dep_dsound.found()
    #sources += files('backends/dsound.c')

    defs += '-DAMW_NATIVE_DSOUND'
    audio_deps += cc.find_library('avrt')
    audio_deps += cc.find_library('msacm32')
    audio_deps += dep_dsound
    enabled_audio_backends += [ 'dsound' ]
endif

# AAudio
dep_aaudio = cc.find_library('aaudio', required : get_option('aaudio'))
if dep_aaudio.found()
    #sources += files('backends/aaudio.c')

    defs += '-DAMW_NATIVE_AAUDIO'
    audio_deps += dep_aaudio
    enabled_audio_backends += [ 'aaudio' ]
endif

# OpenSL|ES
dep_opensles = cc.find_library('OpenSLES', required : get_option('opensles'))
if dep_opensles.found()
    #sources += files('backends/opensles.c')

    defs += '-DAMW_NATIVE_OPENSLES'
    audio_deps += dep_opensles
    enabled_audio_backends += [ 'opensles' ]
endif

# WebAudio
found_webaudio = not get_option('webaudio').disabled() and (sys == 'emscripten')
if found_webaudio
    #sources += files('backends/webaudio.c') 
 
    defs += '-DAMW_NATIVE_WEBAUDIO'
    #link_args += 'lwebaudio.js'
    enabled_audio_backends += [ 'webaudio' ]
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

