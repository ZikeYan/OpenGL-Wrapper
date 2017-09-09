#version 330 core

layout(location = 0) in vec3 in_position;
out vec2 uv;

void main() {
    // clip coordinate [-1, 1] -> [0, 1]
    gl_Position = vec4(in_position, 1);
    uv = (in_position.xy + 1) * 0.5;
}
