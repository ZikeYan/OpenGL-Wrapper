#version 330 core

#define LIGHT_COUNT 1

in vec2 uv;
in vec3 position;
in vec3 normal;

out vec3 shading;
uniform vec3 lights[LIGHT_COUNT];
uniform float light_power;
uniform vec3 light_color;

uniform sampler2D tex;

void main() {
    vec3 s = vec3(0);
    for (int i = 0; i < LIGHT_COUNT; ++i) {
        float distance = length(lights[i] - position);

        vec3 n = normalize(normal);
        vec3 l = normalize(lights[i] - position);
        float cos_theta = clamp(dot(n, l), 0, 1);

        vec3 factor = light_color * light_power / (distance * distance);
        s += cos_theta * factor;
    }
    shading = s + 0 * (s.x * texture(tex, uv)).xyz;
}