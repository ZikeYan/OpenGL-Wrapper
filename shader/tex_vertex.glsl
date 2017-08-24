#version 330 core

layout(location = 0) in vec3 in_position;
out vec2 uv;

uniform mat4 mvp;

void main() {
    // clip coordinate
    gl_Position = vec4(in_position, 1);
    uv = (in_position.xy + 1) * 0.5;
//    gl_PointSize = 5.0f;
//
//    gl_Position =  mvp * vec4(in_position, 1.0);
//    position = gl_Position;
//    shadow_position = vec4(0.5 * gl_Position.xyz + vec3(0.5, 0.5, 0.5), 1);
}
