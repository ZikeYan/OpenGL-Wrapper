//
// Created by Neo on 16/8/2.
//

#include "gl_objects/plane.h"

#include "utils/model.h"
#include "utils/shader.h"
#include "utils/texture.h"

Plane::Plane(std::string vertex_shader_path,
         std::string fragment_shader_path,
         std::string model_path,
         std::string texture_path) :
    vertex_shader_path_(vertex_shader_path),
    fragment_shader_path_(fragment_shader_path),
    model_path_(model_path),
    texture_path_(texture_path) {}

void Plane::Init() {
  LoadModel(model_path_, vertices_, uvs_, normals_, indices_);
  LoadTexture(texture_path_, texture_id_);
  LoadShaders(vertex_shader_path_, fragment_shader_path_, program_id_);

  v_id_               = glGetUniformLocation(program_id_, "c_T_w");
  mvp_id_             = glGetUniformLocation(program_id_, "mvp");
  texture_sampler_id_ = glGetUniformLocation(program_id_, "textureSampler");

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenBuffers(4, vbo_);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               vertices_.size() * sizeof(vertices_[0]),
               vertices_.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[1]);
  glBufferData(GL_ARRAY_BUFFER,
               uvs_.size() * sizeof(uvs_[0]),
               uvs_.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[2]);
  glBufferData(GL_ARRAY_BUFFER,
               normals_.size() * sizeof(normals_[0]),
               normals_.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indices_.size() * sizeof(indices_[0]),
               indices_.data(),
               GL_STATIC_DRAW);
}

void Plane::Render() {
  glUseProgram(program_id_);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE0, texture_id_);
  glUniform1i(texture_sampler_id_, 0);

  glUniformMatrix4fv(mvp_id_, 1, GL_FALSE, mvp_);
  glUniformMatrix4fv(v_id_,   1, GL_FALSE, v_);

  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
}

void Plane::set_mvp(GLfloat * mvp) {
  mvp_ = mvp;
}

void Plane::set_v(GLfloat * v) {
  v_ = v;
}

Plane::~Plane() {
  glDeleteProgram(program_id_);
  glDeleteBuffers(3, vbo_);
  glDeleteVertexArrays(1, &vao_);
  glDeleteTextures(1, &texture_id_);
}