#version 460

layout(location = 8) uniform sampler2DArray texsamplerarray;

in vec3 f_texcoords;

out vec4 fragcolor;

void main() {
    vec4 texel = texelFetch(
        texsamplerarray, 
        ivec3(
            round(f_texcoords.x),
            round(f_texcoords.y),
            round(f_texcoords.z)
        ), 
        0
    );
    
    fragcolor = texel;
}