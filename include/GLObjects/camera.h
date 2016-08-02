//
// Created by Neo on 16/8/2.
//

#ifndef OPENGL_TEMPLATE_CAMERA_H
#define OPENGL_TEMPLATE_CAMERA_H

#include "GLObject.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Camera : public GLObject {
public:
  // Constructor and Deconstructor
  Camera(std::string vertex_shader_path,
         std::string fragment_shader_path);
  ~Camera();

  // Init and Render
  void Init();
  void Render();

  // Data updating
  void UpdateData(glm::mat4 v);
  void set_mvp(GLfloat *mvp);

private:
  // Path
  std::string vertex_shader_path_;
  std::string fragment_shader_path_;

  // Data
  std::vector<glm::vec3> vertices_;
  std::vector<unsigned int> indices_;

  // GL context
  GLuint vbo_[2];

  // Shader specific
  GLint mvp_id_;
  GLfloat *mvp_;

  // Others
  unsigned int enabled_vertex_number_;
};


#endif //OPENGL_TEMPLATE_CAMERA_H
