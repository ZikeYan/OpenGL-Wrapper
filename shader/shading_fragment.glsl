#version 330 core

in vec3 position;
in vec3 normal;

out vec3 shading;
uniform vec3 light;

void main() {
    float s = dot(normalize(normal), normalize(light - position));
    shading = vec3(s, s, s);
}