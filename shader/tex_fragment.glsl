#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D z_buffer;

void main() {
    float z_b = texture(z_buffer, uv).x;
    color = vec4(z_b, z_b, z_b, 1);
}