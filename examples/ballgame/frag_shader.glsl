#version 460

layout(location = 8) uniform sampler2DArray texsamplerarray;
layout(location = 9) uniform uvec3 texarraydims;

in vec3 f_texcoords;

out vec4 fragcolor;

void main() {
    // normalize "raw" texture coordinates with full texture size
    vec4 texel = texture(texsamplerarray, f_texcoords / vec3(texarraydims.xy, 1));

    if (texel.xyz == vec3(255.0 / 255.0, 0.0, 128.0 / 255.0))
        discard;
    
    fragcolor = texel;
}