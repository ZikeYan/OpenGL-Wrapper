#version 330 core

in vec4 original_position;
in vec4 position;
out vec4 color;

uniform sampler2D z_buffer;
layout(location = 0) out float x;

float zbuf2z(float zbuf) {
  float zNear = 0.1f, zFar = 1000.0f;
  zbuf = zbuf * 2.0f - 1;
  return 2.0 * zNear * zFar / (zFar + zNear - zbuf * (zFar - zNear));
}

void main() {
  // Project 3D point (corresponded to atlas) onto 2D image
  vec3 ndc = 0.5f * (position.xyz / position.w + 1);
  if (ndc.x < 0 || ndc.x > 1 || ndc.y < 0 || ndc.y > 1)  {
    color = vec4(0);
    return;
  }

    float bias = 0.001f;
  float z_face = texture(z_buffer, ndc.xy).x;
  float visibility  = float(z_face != 1.0            // finite z
                     && zbuf2z(z_face) > zbuf2z(ndc.z) - bias); // not
  color = visibility * vec4(1);
  //+ visibility * vec4(z_face, z_face, z_face, 1);
}