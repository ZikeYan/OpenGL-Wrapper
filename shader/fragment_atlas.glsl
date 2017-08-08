// vntf stands for vertex, normal, texture and face
#version 330 core

in vec3 position;
in vec3 normal;

out vec3 shading;

void main() {
    vec3 light_position = vec3(2, 4, 2);
    float s = dot(normalize(normal), normalize(light_position - position));
    shading = vec3(s, s, s);
}