#version 330 core

in vec2 uv;
in vec3 position;
in vec3 normal;

out vec3 shading;
uniform vec3 light;
uniform sampler2D tex;
void main() {
    float s = dot(normalize(normal), normalize(light - position));
    shading = vec3(s, s, s) + 0 * (s * texture(tex, uv)).xyz;
}