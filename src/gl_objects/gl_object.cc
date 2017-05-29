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
                          std::unordered_map<std::string, UniformType> &uniform_types) {
  LoadShaders(vert_shader_path, frag_shader_path, program_);

  uniform_count_ = uniform_types.size();
  if (uniform_count_ == 0)
    return;

  uniforms_.clear();
  for (auto& uniform_type : uniform_types) {
    GLint uniform_id = glGetUniformLocation(program_,
                                            uniform_type.first.c_str());
    if (uniform_id < 0) {
      std::cerr << "Invalid uniform name!" << std::endl;
      exit(1);
    }

    std::cout << uniform_type.first << " : " << uniform_id << std::endl;
    uniforms_[uniform_type.first] = Uniform(uniform_type.second,
                                            uniform_id,
                                            NULL);
  }

  is_shader_inited_ = true;
}

void GLObject::InitVAO(std::vector<int>& max_count_of_objects) {
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  if (max_count_of_objects.size() != vbo_count_) {
    std::cerr << "Invalid buffer sizes!" << std::endl;
    exit(1);
  }

  max_count_of_objects_ = std::vector<int>(max_count_of_objects.begin(),
                                           max_count_of_objects.end());
  curr_count_of_objects_ = std::vector<int>(max_count_of_objects.begin(),
                                            max_count_of_objects.end());

  vbos_ = new GLuint[vbo_count_];
  glGenBuffers(vbo_count_, vbos_);

  // Vertices: all shared
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * max_count_of_objects_[0],
               NULL,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  switch (mesh_type_) {
    case kVertex:
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   // vertex number <-> index
                   sizeof(unsigned int) * max_count_of_objects_[0],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexFace:
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * max_count_of_objects_[1],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexNormalFace:
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 3 * max_count_of_objects_[1],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[2]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * max_count_of_objects_[2],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexNormalUVFace:
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 3 * max_count_of_objects_[1],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[2]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 2 * max_count_of_objects_[2],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[3]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * max_count_of_objects_[3],
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

void GLObject::SetUniforms(std::unordered_map<std::string, void*>& uniform_values) {
  if (uniforms_.size() != uniform_values.size()) {
    std::cerr << "Incorrect uniform size" << std::endl;
    exit(1);
  }

  for (auto& uniform_value : uniform_values) {
    if (uniforms_.find(uniform_value.first) == uniforms_.end()) {
      std::cerr << "Invalid uniform name!" << std::endl;
      exit(1);
    }
    uniforms_[uniform_value.first].data = uniform_value.second;
  }
}

void GLObject::SetTexture(unsigned char *data, int width, int height) {
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void GLObject::SetMesh(float* vertices, int vertex_count) {
  if (mesh_type_ != kVertex) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }

  if (vertex_count > max_count_of_objects_[0]) {
    std::cerr << "Insufficient memory!" << std::endl;
    exit(1);
  }

  curr_count_of_objects_[0] = vertex_count;
  curr_count_of_objects_[1] = vertex_count;

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * curr_count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  std::vector<unsigned int> indices;
  indices.resize(vertex_count);
  for (unsigned int i = 0; i < vertex_count; ++i) {
    indices[i] = i;
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      // vertex number <-> index
               sizeof(unsigned int) * curr_count_of_objects_[1],
               indices.data(),
               GL_STATIC_DRAW);
}

void GLObject::SetMesh(float* vertices, int vertex_count,
                       unsigned int *faces, int face_count) {
  if (mesh_type_ != kVertexFace) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }

  curr_count_of_objects_[0] = vertex_count;
  curr_count_of_objects_[1] = face_count;

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * curr_count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * 3 * curr_count_of_objects_[1],
               faces,
               GL_STATIC_DRAW);
}

void GLObject::SetMesh(float* vertices, int vertex_count,
                       float* normals, int normal_count,
                       unsigned int *faces, int face_count) {
  if (mesh_type_ != kVertexNormalFace) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }

  curr_count_of_objects_[0] = vertex_count;
  curr_count_of_objects_[1] = normal_count;
  curr_count_of_objects_[2] = face_count;

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * curr_count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * curr_count_of_objects_[1],
               normals,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * 3 * curr_count_of_objects_[2],
               faces,
               GL_STATIC_DRAW);
}

void GLObject::SetMesh(float *vertices, int vertex_count,
                       float *normals, int normal_count,
                       float *uvs,      int uv_count,
                       unsigned int *faces, int face_count) {
  if (mesh_type_ != kVertexNormalUVFace) {
    std::cerr << "Incompatible mesh type!" << std::endl;
    exit(1);
  }
  curr_count_of_objects_[0] = vertex_count;
  curr_count_of_objects_[1] = normal_count;
  curr_count_of_objects_[2] = uv_count;
  curr_count_of_objects_[3] = face_count;

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * curr_count_of_objects_[0],
               vertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * curr_count_of_objects_[1],
               normals,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, vbos_[2]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 2 * curr_count_of_objects_[2],
               uvs,
               GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * 3 * curr_count_of_objects_[3],
               faces,
               GL_STATIC_DRAW);
}

void GLObject::Render() {
  glUseProgram(program_);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE0, texture_);

  for (auto& uniform : uniforms_) {
    if (uniform.second.type == kTexture) {
      glUniform1i(uniform.second.id, 0);
    } else if (uniform.second.type == kMatrix4f) {
      glUniformMatrix4fv(uniform.second.id, 1, GL_FALSE,
                         (float*)uniform.second.data);
    }
  }
  glBindVertexArray(vao_);

  /// only deal with kVertexNormalUVFace at current
  // TODO: Adapt to other situations
  switch (mesh_type_) {
    case kVertex:
    case kVertexFace:
      glDrawElements(GL_TRIANGLES,
                     curr_count_of_objects_[1], GL_UNSIGNED_INT, 0);
      break;
    case kVertexNormalFace:
      glDrawElements(GL_TRIANGLES,
                     curr_count_of_objects_[2], GL_UNSIGNED_INT, 0);
      break;
    case kVertexNormalUVFace:
      glDrawElements(GL_TRIANGLES,
                     curr_count_of_objects_[3] * 3, GL_UNSIGNED_INT, 0);
      break;
    case kImage:
      break;
  }

}
