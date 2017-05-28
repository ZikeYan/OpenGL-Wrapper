//
// Created by Neo on 27/05/2017.
//

#include "gl_objects/gl_object.h"

#include <iostream>
#include "utils/shader.h"

GLObject::GLObject(MeshType mesh_type) {
  mesh_type_ = mesh_type;
  switch (mesh_type_) {
    case kVertex:
      vbo_count_ = 2; break;
    case kVertexFace:
      vbo_count_ = 2; break;
    case kVertexNormalFace:
      vbo_count_ = 3; break;
    case kVertexNormalUVFace:
      vbo_count_ = 4; break;
    default:
      vbo_count_ = 0; break;
  }
}

GLObject::~GLObject() {
  if (is_shader_inited_) {
    glDeleteProgram(program_);
  }
  if (is_vao_inited_) {
    glDeleteBuffers(vbo_count_, vbos_);
    glDeleteVertexArrays(1, &vao_);
    delete [] vbos_;
  }
  if (is_texture_inited_) {
    glDeleteTextures(1, &texture_);
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

  is_shader_inited_ = true;
}

void GLObject::InitVAO(std::vector<int>& count_of_objects) {
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  if (count_of_objects.size() != vbo_count_) {
    std::cerr << "Invalid buffer sizes!" << std::endl;
    exit(1);
  }

  count_of_objects_ = std::vector<int>(count_of_objects.begin(),
                                       count_of_objects.end());

  vbos_ = new GLuint[vbo_count_];
  glGenBuffers(vbo_count_, vbos_);

  // Vertices: all shared
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * count_of_objects_[0],
               NULL,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  switch (mesh_type_) {
    case kVertex:
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   // vertex number <-> index
                   sizeof(unsigned int) * count_of_objects_[0],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexFace:
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * count_of_objects_[1],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexNormalFace:
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 3 * count_of_objects_[1],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[2]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * count_of_objects_[2],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexNormalUVFace:
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 3 * count_of_objects_[1],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[2]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 2 * count_of_objects_[2],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[3]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * count_of_objects_[3],
                   NULL,
                   GL_STATIC_DRAW);
    default:
      break;
  }

  is_vao_inited_ = true;
}

void GLObject::InitTexture(int width, int height) {
  // Texture manipulation
  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
               GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  is_texture_inited_ = true;
}

void GLObject::SetUniforms(std::vector<void *> uniform_values) {
  if (uniforms_.size() != uniform_values.size()) {
    std::cerr << "Incorrect uniform size" << std::endl;
    exit(1);
  }

  uniform_values_.resize(uniform_values.size());
  for (int i = 0; i < uniform_values.size(); ++i) {
    if (uniforms_[i].first == kTexture) {
      // No need to buffer data
    } else if (uniforms_[i].first == kMatrix4f) {
      uniform_values_[i] = uniform_values[i];
    }
  }
}

void GLObject::SetTexture(unsigned char *data, int width, int height) {
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void GLObject::SetMesh(float* vertices) {
  if (mesh_type_ != kVertex) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void GLObject::SetMesh(float* vertices,
                       unsigned int *faces) {
  if (mesh_type_ != kVertexFace) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * 3 * count_of_objects_[1],
               faces,
               GL_STATIC_DRAW);
}

void GLObject::SetMesh(float* vertices, float* normals,
                       unsigned int *faces) {
  if (mesh_type_ != kVertexNormalFace) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * count_of_objects_[1],
               normals,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * 3 * count_of_objects_[2],
               faces,
               GL_STATIC_DRAW);
}

void GLObject::SetMesh(float *vertices, float *normals, float *uvs,
                       unsigned int *faces) {
  if (mesh_type_ != kVertexNormalUVFace) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * count_of_objects_[1],
               normals,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[2]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 2 * count_of_objects_[2],
               uvs,
               GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * 3 * count_of_objects_[3],
               faces,
               GL_STATIC_DRAW);
}

void GLObject::Render() {
  glUseProgram(program_);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE0, texture_);

  for (int i = 0; i < uniforms_.size(); ++i) {
    if (uniforms_[i].first == kTexture) {
      glUniform1i(uniforms_[i].second, 0);
    } else if (uniforms_[i].first == kMatrix4f) {
      glUniformMatrix4fv(uniforms_[i].second, 1, GL_FALSE,
                         (float*)uniform_values_[i]);
    }
  }

  glBindVertexArray(vao_);

  /// only deal with kVertexNormalUVFace at current
  // TODO: Adapt to other situations
  glDrawElements(GL_TRIANGLES,
                 count_of_objects_[3] * 3, GL_UNSIGNED_INT, 0);
}
