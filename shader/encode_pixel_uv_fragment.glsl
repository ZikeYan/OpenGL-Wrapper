#version 330 core

layout(location = 0) out vec4 float_uv;

in vec2 uv;

void main() {
    float_uv = vec4(uv, 1, 1);
}
