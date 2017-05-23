//
// Created by Neo on 16/8/2.
//

#ifndef OPENGL_TEMPLATE_F16_H
#define OPENGL_TEMPLATE_F16_H

#include "gl_object.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Plane : public GLObject {
public:
  // Constructor and Deconstructor
  Plane(std::string vertex_shader_path,
        std::string fragment_shader_path,
        std::string model_path,
        std::string texture_path);
  ~Plane();

  // Init and Render
  void Init();
  void Render();

  // Data updating
  void set_mvp(GLfloat * mvp);
  void set_v(GLfloat * v);

private:
  // Path
  std::string vertex_shader_path_;
  std::string fragment_shader_path_;
  std::string model_path_;
  std::string texture_path_;

  // Data
  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec2> uvs_;
  std::vector<glm::vec3> normals_;
  std::vector<unsigned int> indices_;

  // GL context
  GLuint vbo_[4];

  // Shader specific
  GLint texture_sampler_id_;
  GLint mvp_id_;
  GLint v_id_;
  GLfloat * mvp_;
  GLfloat * v_;

  // Other data structures
};


#endif //OPENGL_TEMPLATE_F16_H
