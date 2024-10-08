#sources += files('')

# Vulkan is loaded at runtime
if not dependency('vulkan', required : get_option('vulkan')).found()
    error('Vulkan SDK is needed to build this project.')
endif

sources += files(
    'vk_common.c',
    'vk_driver.c',
    'vk_shaders.c',
    'vk_validation.c',
)

defs += '-DAMW_NATIVE_VULKAN'
