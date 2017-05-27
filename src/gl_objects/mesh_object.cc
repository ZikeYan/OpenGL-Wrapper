//
// Created by Neo on 26/05/2017.
//

#include "gl_objects/mesh_object.h"

#include <iostream>

MeshObject::MeshObject(MeshType mesh_type) {
  mesh_type_ = mesh_type;
  switch (mesh_type_) {
    case kVertex:
      vbo_count_ = 1; break;
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

MeshObject::~MeshObject() {
  if (is_vao_inited_) {
    glDeleteBuffers(vbo_count_, vbos_);
    glDeleteVertexArrays(1, &vao_);
    delete [] vbos_;
  }
}

void MeshObject::InitVAO(std::vector<int>& count_of_objects) {
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
               sizeof(float) * 3 * count_of_objects[0],
               NULL,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  switch (mesh_type_) {
    case kVertex:
      break;
    case kVertexFace:
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * count_of_objects[1],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexNormalFace:
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 3 * count_of_objects[1],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[2]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * count_of_objects[2],
                   NULL,
                   GL_STATIC_DRAW);
      break;
    case kVertexNormalUVFace:
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[1]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 3 * count_of_objects[1],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[2]);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 2 * count_of_objects[2],
                   NULL,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_[3]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * 3 * count_of_objects[3],
                   NULL,
                   GL_STATIC_DRAW);
    default:
      break;
  }

  is_vao_inited_ = true;
}

void MeshObject::InitTexture(int width, int height) {
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

void MeshObject::LoadMesh(float *vertices,
                          float *normals,
                          float *uvs,
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

void MeshObject::Render() {
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
  /// only deal with kVertexNormalUVFace
  std::cout << count_of_objects_[3] << std::endl;
  glDrawElements(GL_TRIANGLES, count_of_objects_[3] * 3, GL_UNSIGNED_INT, 0);
}