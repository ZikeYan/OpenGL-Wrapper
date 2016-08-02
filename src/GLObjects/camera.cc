//
// Created by Neo on 16/8/2.
//

#include "GLObjects/camera.h"

#include "utils/shader.h"

Camera::Camera(std::string vertex_shader_path,
               std::string fragment_shader_path) :
    vertex_shader_path_(vertex_shader_path),
    fragment_shader_path_(fragment_shader_path),
    enabled_vertex_number_(0) {}

void Camera::Init() {
  LoadShaders(vertex_shader_path_, fragment_shader_path_, program_id_);
  mvp_id_ = glGetUniformLocation(program_id_, "mvp");

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenBuffers(2, vbo_);
}

void Camera::Render() {
  glUseProgram(program_id_);

  glUniformMatrix4fv(mvp_id_, 1, GL_FALSE, mvp_);

  glBindVertexArray(vao_);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
  glBufferData(GL_ARRAY_BUFFER,
               vertices_.size() * sizeof(vertices_[0]),
               vertices_.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indices_.size() * sizeof(indices_[0]),
               indices_.data(),
               GL_STATIC_DRAW);

  glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_INT, 0);
}


void Camera::set_mvp(GLfloat *mvp) {
  mvp_ = mvp;
}

void Camera::UpdateData(glm::mat4 v) {
  glm::mat4 v_inv = glm::inverse(v);
  glm::vec4 o = v_inv * glm::vec4(0,   0,    -0.1, 1);
  glm::vec4 x = v_inv * glm::vec4(0.1, 0,    -0.1, 1);
  glm::vec4 y = v_inv * glm::vec4(0,   -0.1, -0.1, 1);
  glm::vec4 z = v_inv * glm::vec4(0,   0,    -0.2, 1);

  vertices_.push_back(glm::vec3(o.x, o.y, o.z));
  vertices_.push_back(glm::vec3(x.x, x.y, x.z));
  vertices_.push_back(glm::vec3(y.x, y.y, y.z));
  vertices_.push_back(glm::vec3(z.x, z.y, z.z));

  indices_.push_back(enabled_vertex_number_);
  indices_.push_back(enabled_vertex_number_ + 1);
  indices_.push_back(enabled_vertex_number_);
  indices_.push_back(enabled_vertex_number_ + 2);
  indices_.push_back(enabled_vertex_number_);
  indices_.push_back(enabled_vertex_number_ + 3);

  enabled_vertex_number_ += 4;
}

Camera::~Camera() {
  glDeleteProgram(program_id_);
  glDeleteBuffers(2, vbo_);
  glDeleteVertexArrays(1, &vao_);
}