#version 330 core

layout(location = 0) in vec2 uv_position;
layout(location = 1) in vec3 in_position;
layout(location = 2) in vec3 in_normal;

out vec3 position;
out vec3 normal;

void main() {
    // clip coordinate
    vec2 uv = vec2(2 * uv_position.x - 1, 2 * uv_position.y - 1);
    gl_Position =  vec4(uv, 0.0, 1.0);
    position = in_position;
    normal = in_normal;
}
