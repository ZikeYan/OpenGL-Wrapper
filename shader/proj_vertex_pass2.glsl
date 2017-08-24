#version 330 core

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_position;

out vec4 original_position;
out vec4 position;
uniform mat4 mvp;

void main() {
    // clip coordinate
    gl_Position = vec4(2*in_uv-1, 0, 1);
    original_position = vec4(in_position, 1);
    position = mvp * vec4(in_position, 1);
//    gl_PointSize = 5.0f;
//
//    gl_Position =  mvp * vec4(in_position, 1.0);
//    position = gl_Position;
//    shadow_position = vec4(0.5 * gl_Position.xyz + vec3(0.5, 0.5, 0.5), 1);
}
