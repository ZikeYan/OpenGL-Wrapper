#version 330 core

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_position;
layout(location = 2) in vec3 in_normal;

out vec2 uv;
out vec3 position;
out vec3 normal;

void main() {
    // clip coordinate
    gl_Position =  vec4(2 * in_uv - 1, 0.0, 1.0);
    position = in_position;
    normal = in_normal;
    uv = in_uv;
}
