//
// Created by Neo on 27/05/2017.
//

#include "gl_objects/gl_object.h"

#include <iostream>
#include "utils/shader.h"

GLObject::GLObject() {}
GLObject::~GLObject() {
  if (is_shader_inited_) {
    glDeleteProgram(program_);
  }
}

void GLObject::InitShader(std::string vert_shader_path,
                          std::string frag_shader_path,
                          std::vector <std::pair<UniformType, std::string> >
                          &uniform_names) {
  LoadShaders(vert_shader_path, frag_shader_path, program_);

  uniform_count_ = uniform_names.size();
  if (uniform_count_ == 0)
    return;

  uniforms_.clear();
  for (int i = 0; i < uniform_count_; ++i) {
    GLint uniform_id = glGetUniformLocation(program_,
                                            uniform_names[i].second.c_str());
    if (uniform_id < 0) {
      std::cerr << "Invalid uniform name!" << std::endl;
      exit(1);
    }
    std::cout << uniform_names[i].second << " : " << uniform_id << std::endl;
    uniforms_.push_back(std::make_pair(uniform_names[i].first, uniform_id));
  }
}

void GLObject::SetUniforms(std::vector<void *> uniform_vals) {
  if (uniforms_.size() != uniform_vals.size()) {
    std::cerr << "Incorrect uniform size" << std::endl;
    exit(1);
  }

  uniform_values_.resize(uniform_vals.size());
  for (int i = 0; i < uniform_vals.size(); ++i) {
    if (uniforms_[i].first == kTexture) {
      // No need to buffer data
    } else if (uniforms_[i].first == kMatrix4f) {
      uniform_values_[i] = uniform_vals[i];
    }
  }
}

void GLObject::SetTexture(unsigned char *data, int width, int height) {
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
}
