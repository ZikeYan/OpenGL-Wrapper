// v stands for vertex only
#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 mvp; // K * c_T_w

void main() {
    // clip coordinate
    gl_Position =  mvp * vec4(position, 1.0);
}
