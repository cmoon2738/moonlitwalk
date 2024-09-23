#version 450
// #extension GL_EXT_debug_printf: enable
#extension GL_EXT_scalar_block_layout: enable

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

layout(scalar, binding = 0) uniform UniformBufferObject {
    mat2 view;
} ubo;

void main() {
    vec2 out_position = ubo.view * in_position;
    // debugPrintfEXT(
    //     "view: { { %f, %f, }, { %f, %f, }, },",
    //     ubo.view[0][0],
    //     ubo.view[0][1],
    //     ubo.view[1][0],
    //     ubo.view[1][1]
    // );
    gl_Position = vec4(out_position, 0.0, 1.0);
    frag_color = in_color;
}
