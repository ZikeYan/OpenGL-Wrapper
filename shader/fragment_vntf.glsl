#version 330 core

// Interpolated values from the vertex shaders
in vec2 outUV;
in vec3 vertex_position_w;
in vec3 vertex_normal_w;
in vec3 vertex_normal_c;
in vec3 eye_dir_c;
in vec3 light_dir_c;
in float z;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureSampler;

void main(){
    vec3  light_position_w = vec3(2, 4, 2);
	vec4  light_color = vec4(1, 1, 1, 1);
	float light_power = 50.0f;

	// Material properties
    int u = int(1023 * outUV.x);
    int v = int(1023 * outUV.y);
    // vec3(outUV.x, outUV.y, 1);

    int vrem = v % 255, urem = u % 255;
    int vdiv = v / 255, udiv = u / 255;
    vec4 diffuse_color;
    if (0 <= vdiv && vdiv <= 4) {
        diffuse_color  =  vec4(urem / 255.0, udiv * 0.2,
                               vrem / 255.0, vdiv * 0.2);
        // float(v) * 0.0009775171065493646,
    //    vdiv;
	} else {
        diffuse_color = vec4(0, 0, 0, 0);
	}
	diffuse_color = 1 * texture(textureSampler,outUV).rgba;
	// diffuse_color = vec3(0, 1, 0);
	vec4 ambient_color  = vec4(0.1, 0.1, 0.1, 1) * diffuse_color;
	vec4 specular_color = vec4(0.3, 0.3, 0.3, 1);

	float distance = length(light_position_w - vertex_position_w);

	//vec3 n = normalize(vertex_normal_c);
    //vec3 l = normalize(light_dir_c);
    vec3 n = normalize(vertex_normal_w);
    vec3 l = normalize(light_position_w - vertex_position_w);

	float cos_theta = clamp(dot(n, l), 0, 1);

	vec3 e = normalize(eye_dir_c);
	vec3 r = reflect(-l, n);
	float cos_alpha = clamp(dot(e, r), 0, 1);

	color =
		// Ambient : simulates indirect lighting
		0 * ambient_color +
		// Diffuse : "color" of the object
		diffuse_color //light_color * light_power *
        // * cos_theta
		// / (distance*distance)
		+
		// Specular : reflective highlight, like a mirror
		0 * specular_color * light_color * light_power * pow(cos_alpha, 5) /
		(distance*distance);

}