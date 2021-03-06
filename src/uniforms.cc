//
// Created by Neo on 14/09/2017.
//

#include "uniforms.h"
#include <iostream>

namespace gl {

void Uniforms::GetLocation(GLuint program,
                           std::string name,
                           UniformType type) {
  GLint uniform_id = glGetUniformLocation(program, name.c_str());
  if (uniform_id < 0) {
    std::cerr << "Invalid uniform name!" << std::endl;
    exit(1);
  }
  uniform_ids_[name] = std::make_pair((GLuint)uniform_id, type);
}

/// Override specially designed sfor texture
void Uniforms::Bind(std::string name, GLuint idx) {
  switch (type(name)) {
    case kTexture2D:
      glUniform1i(id(name), idx);
      break;
    default:
      break;
  }
}
void Uniforms::Bind(std::string name, void *data, int n) {
  switch (type(name)) {
    case kMatrix4f:
      glUniformMatrix4fv(id(name), n, GL_FALSE, (float*)data);
      break;
    case kVector3f:
      glUniform3fv(id(name), n, (float*)data);
      break;
    case kFloat:
      glUniform1fv(id(name), n, (float*)data);
      break;
    case kInt:
      glUniform1iv(id(name), n, (int*)data);
      break;
    default:
      break;
  }
}
}