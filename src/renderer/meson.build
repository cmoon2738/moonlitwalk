#sources += files('')

if (sys == 'emscripten' and not get_option('webgpu').disabled()) 
    #sources += files('')
    #link_args += 'lwebgpu.js'
    defs += [ '-DAMW_NATIVE_WEBGPU' ]
    enabled_rendering_backends = 'webgpu'

else
    # Vulkan is loaded at runtime
    vulkan_sdk = dependency('vulkan', required : get_option('vulkan')

    if vulkan_sdk.found()
        sources += files(
            'vulkan/vk_common.c',
            'vulkan/vk_driver.c',
            'vulkan/vk_shaders.c',
            'vulkan/vk_surface.c',
            'vulkan/vk_validation.c',
        )
        defs += [ '-DAMW_NATIVE_VULKAN' ]
        enabled_rendering_backends = 'vulkan'
    endif
endif

