shaders_src = [
    'base.frag',
    'base.vert',
]

if spirv_shader_translation
    glslang = find_program('glslangValidator')

    foreach shader : shaders_src
        name = shader.underscorify() + '_spv'
        args = [ glslang, '-V', '@INPUT@', '-o', '@OUTPUT@', '--vn', name ]
        header = custom_target(
            shader + '_spv',
            output : shader + '.spv.h',
            input : shader,
            command : args,
            install : false)
        shaders += [ header ]
    endforeach
endif
