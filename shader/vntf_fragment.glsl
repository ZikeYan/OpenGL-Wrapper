#version 330 core

// Interpolated values from the vertex shaders
in vec2 uv;

in vec3 position_c;
in vec3 normal_c;
in vec3 light_c;

in float z;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texture_sampler;

void main(){
	float light_power = 0.5f;
	vec4 light_color = vec4(1, 1, 1, 1);
	vec4 diffuse_color = texture(texture_sampler, uv).rgba;
	vec4 ambient_color  = vec4(0.2, 0.2, 0.2, 1) * diffuse_color;
	vec4 specular_color = vec4(0.3, 0.3, 0.3, 1);
	float distance = length(light_c - position_c);

	// Material properties
//    int u = int(1023 * outUV.x);
//    int v = int(1023 * outUV.y);
    // vec3(outUV.x, outUV.y, 1);

//    int vrem = v % 255, urem = u % 255;
//    int vdiv = v / 255, udiv = u / 255;
//    vec4 diffuse_color;
//    if (0 <= vdiv && vdiv <= 4) {
//        diffuse_color  =  vec4(urem / 255.0, udiv * 0.2,
//                               vrem / 255.0, vdiv * 0.2);
//        // float(v) * 0.0009775171065493646,
//    //    vdiv;
//	} else {
//        diffuse_color = vec4(0, 0, 0, 0);
//	}
    vec3 n = normalize(normal_c);
    vec3 l = normalize(light_c - position_c);
	float cos_theta = clamp(dot(n, l), 0, 1);

	vec3 e = -normalize(position_c);
	vec3 r = reflect(-l, n);
	float cos_alpha = clamp(dot(e, r), 0, 1);

    vec4 factor = light_color * light_power / (distance * distance);
	color =
		// Ambient : simulates indirect lighting
		0 * ambient_color +
		// Diffuse : "color" of the object
		diffuse_color * cos_theta// * factor
		+
		// Specular : reflective highlight, like a mirror
		0 * specular_color * pow(cos_alpha, 5) * factor;
}