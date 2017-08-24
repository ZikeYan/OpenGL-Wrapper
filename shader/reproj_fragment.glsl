#version 330 core

// Interpolated values from the vertex shaders
in vec2 uv;
in vec3 position_c;
in float z;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texture_sampler;

void main(){
	float light_power = 0.5f;
	vec4 light_color = vec4(1, 1, 1, 1);
	vec4 diffuse_color = texture(texture_sampler, uv).rgba;

    color = diffuse_color;
}