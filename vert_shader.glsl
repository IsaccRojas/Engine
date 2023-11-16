#version 460

layout(location = 0) in vec4 v_model;
layout(location = 1) in vec3 v_pos;
layout(location = 2) in vec3 v_scale;
layout(location = 3) in vec3 v_texpos;
layout(location = 4) in vec2 v_texsize;
layout(location = 5) in float v_draw;

layout(location = 6) uniform mat4 u_view;
layout(location = 7) uniform mat4 u_proj;

out vec3 f_texcoords;

void main() {
    //get final texture coordinates by adding: texsize multiplied by model positions (are either 0.0 or 1.0), and flip the vertical shift
    f_texcoords = 
        v_texpos 
        + 
        (
            vec3(v_model.x, 1.0 - v_model.y, 0.0) 
            * vec3(v_texsize - vec2(1.0f, 1.0f), 0.0)
        )
    ;
    
    //get final pos by shifting unit model to center, scaling it by attribute scale, and adding attribute pos
    vec4 final_pos = 
        (
            (
                (v_model + vec4(-0.5f, -0.5f, 0.0f, 0.0f)) 
                * vec4(v_scale, 1.0f)
            ) 
            + vec4(v_pos, 0.0f)
        ) 
        * v_draw
    ;

    gl_Position = u_proj * u_view * final_pos;
}