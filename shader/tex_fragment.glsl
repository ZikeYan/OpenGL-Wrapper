#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D z_buffer;

void main() {
    color = texture(z_buffer, uv);
}