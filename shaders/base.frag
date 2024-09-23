#version 450
// #extension GL_EXT_debug_printf: enable
#extension GL_EXT_scalar_block_layout: enable

layout(location = 0) in vec3 fragColor; 

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
    // debugPrintfEXT(
    //     "pos: { %f, %f, %f },",
    //     gl_FragCoord[0],
    //     gl_FragCoord[1],
    //     gl_FragCoord[2]
    // );
}
