// v stands for vertex only
#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 c_T_w;
uniform mat4 mvp; // K * c_T_w

void main() {
    // clip coordinate
    vec4 position_w = c_T_w * vec4(position, 1.0);
    gl_Position =  mvp * vec4(position, 1.0);
}
