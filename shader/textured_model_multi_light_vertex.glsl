#version 330 core

#define LIGHT_COUNT 1

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out vec2 uv;

out vec3 position_c;
out vec3 normal_c;
out vec3 light_c[LIGHT_COUNT];

uniform mat4 c_T_w;
uniform mat4 mvp; // K * c_T_w
uniform int  light_cnt;
uniform vec3 light[LIGHT_COUNT];

void main() {
    // clip coordinate
    gl_Position =  mvp * vec4(in_position, 1.0);

    // out
    uv = in_uv;
    position_c   = (c_T_w * vec4(in_position, 1)).xyz;
    normal_c = (c_T_w * vec4(in_normal, 0)).xyz;

    for (int i = 0; i < light_cnt; ++i) {
        light_c[i] = (c_T_w * vec4(light[i], 1)).xyz;
    }
}
