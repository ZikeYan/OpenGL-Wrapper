// vntf stands for vertex, normal, texture and face
#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

out vec2 uv;
out vec3 position_c;
out float z;

uniform mat4 c_T_w;
uniform mat4 mvp; // K * c_T_w

void main() {
    // clip coordinate
    gl_Position =  mvp * vec4(in_position, 1.0);

    // out
    uv = in_uv;
    position_c   = (c_T_w * vec4(in_position, 1)).xyz;
    z = (c_T_w * vec4(in_position, 1)).z;
}
