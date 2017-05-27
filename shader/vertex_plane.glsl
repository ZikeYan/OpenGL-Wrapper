#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 inUV;

out vec2 outUV;
out vec3 vertex_position_w;
out vec3 vertex_normal_c;
out vec3 eye_dir_c;
out vec3 light_dir_c;

uniform mat4 c_T_w;
uniform mat4 mvp; // K * c_T_w

void main() {
    vec3 light_position_w = vec3(4, 5, 4);

    // clip coordinate
    gl_Position =  mvp * vec4(position, 1.0);

    // out
    outUV = inUV;
    vertex_position_w = (vec4(position, 1)).xyz;
    vertex_normal_c = (c_T_w * vec4(normal, 0)).xyz;

    eye_dir_c   = vec3(0, 0, 0) - (c_T_w * vec4(position, 1)).xyz;
    light_dir_c = vec3(0, 0, 0) - (c_T_w * vec4(light_position_w, 1)).xyz;
}
