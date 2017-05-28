#version 330 core

// Interpolated values from the vertex shaders
in vec2 outUV;
in vec3 vertex_position_w;
in vec3 vertex_normal_c;
in vec3 eye_dir_c;
in vec3 light_dir_c;
in float z;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureSampler;

void main(){
  vec3  light_position_w = vec3(8, 5, 4);
	vec3  light_color = vec3(1, 1, 1);
	float light_power = 50.0f;

	// Material properties
	vec3 diffuse_color  = texture(textureSampler, outUV).rgb;
	// diffuse_color = vec3(0, 1, 0);
	vec3 ambient_color  = vec3(0.1, 0.1, 0.1) * diffuse_color;
	vec3 specular_color = vec3(0.3, 0.3, 0.3);

	float distance = length(light_position_w - vertex_position_w);

	vec3 n = normalize(vertex_normal_c);
	vec3 l = normalize(light_dir_c);
	float cos_theta = clamp(dot(n, l), 0, 1);

	vec3 e = normalize(eye_dir_c);
	vec3 r = reflect(-l, n);
	float cos_alpha = clamp(dot(e, r), 0, 1);

	color =
		// Ambient : simulates indirect lighting
		ambient_color +
		// Diffuse : "color" of the object
		diffuse_color * light_color * light_power * cos_theta / (distance*distance) +
		// Specular : reflective highlight, like a mirror
		specular_color * light_color * light_power * pow(cos_alpha, 5) / (distance*distance);

}